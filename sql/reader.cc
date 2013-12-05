#include "reader.h"

#include <vector>
#include <iostream>

using std::min;
using std::max;
using std::vector;

#define my_b_get2(info, cache, read_pos) \
       ((info)->read_pos != (info)->read_end ?\
       ((info)->read_pos++, (int) (uchar) (info)->read_pos[-1]) :\
       do_read(info, cache, read_pos, freeBuffers))

/*
#define my_b_get2(info, cache) \
      ((info)->read_pos != (info)->read_end ?\
          ((info)->read_pos++, (int) (uchar) (info)->read_pos[-1]) :\
          do_read(info, cache, read_pos))


inline int do_read(IO_CACHE* cache, map<int,BufStruct>* buffers, int& read_pos) {
  read_pos++;
  int r = _my_b_get(cache);

  uchar* b = new uchar[cache->buffer_length+sizeof(int)];
  if(cache->request_pos==NULL)
      return my_b_EOF;

  strncpy((char*)b, (char*)cache->request_pos, cache->buffer_length);
  *((int*)(b+cache->buffer_length)) = (int)(cache->read_end-cache->request_pos);
  (*buffers)[read_pos] = b;

  return r;
}
*/

inline int do_read(IO_CACHE* cache, map<int,BufStruct>& buffers, uint& read_pos, vector<uchar*>& freeBuffers) {
  read_pos++;
  if(buffers.find(read_pos)!=buffers.end()) {
    BufStruct s = buffers[read_pos];
    cache->request_pos = s.request_pos;
    cache->read_pos = cache->request_pos+1;
    // We need these two pieces of info so we just tag them to the end of the buffer
    int length = s.length;  
        
    // cache->pos_in_file = pos_in_file;
    cache->read_end = s.request_pos + length;
    cache->buffer = s.request_pos;
 
    return (int)*cache->request_pos;
  }
  /*
  else if(cache->request_pos==NULL) {
    // If the request_pos is null then there is not data, so we return eof
    // This ensures we terminate and the client returns correct error
    return my_b_EOF;
  }
   */
  // If the free buffer last has a free buffer, use it, otherwise allocate a new one

  uchar* buf;
  if(freeBuffers.empty()) {
    buf = new uchar[cache->buffer_length+sizeof(int)];
  } else {
    buf = freeBuffers.back();
    freeBuffers.pop_back();
  }
/*
    int readend = cache->read_end-cache->request_pos;
    cache->request_pos = buf;
    cache->buffer = buf;
    cache->read_end = buf + readend;
    cache->read_pos = cache->read_end;
*/
  int r = _my_b_get(cache);

  strncpy((char*)buf, (char*)cache->request_pos, cache->buffer_length);
  int length  = cache->read_end-cache->request_pos;

  buffers[read_pos] = (BufStruct){buf, length};

  return r;
}
 
#define GET (stack_pos != stack ? *--stack_pos : my_b_get2(&cache, buffers, read_pos))
#define PUSH(A) *(stack_pos++)=(A)

/* Unescape all escape characters, mark \N as null */

char
READER::unescape(char chr)
{
  /* keep this switch synchornous with the ESCAPE_CHARS macro */
  switch(chr) {
  case 'n': return '\n';
  case 't': return '\t';
  case 'r': return '\r';
  case 'b': return '\b';
  case '0': return 0;				// Ascii null
  case 'Z': return '\032';			// Win32 end of file
  case 'N': found_null=1;

    /* fall through */
  default:  return chr;
  }
}


/*
  Read a line using buffering
  If last line is empty (in line mode) then it isn't outputed
*/


READER::READER(File file_par, uint tot_length, const CHARSET_INFO *cs,
                     const String &field_term,
                     const String &line_start,
                     const String &line_term,
                     const String &enclosed_par,
                     int escape, bool get_it_from_net, bool is_fifo)
  :file(file_par), buff_length(tot_length), escape_char(escape),
   found_end_of_line(false), eof(false), need_end_io_cache(false),
   error(false), line_cuted(false), found_null(false), read_charset(cs), read_pos(0)
{
  field_term_ptr= field_term.ptr();
  field_term_length= field_term.length();
  line_term_ptr= line_term.ptr();
  line_term_length= line_term.length();
  level= 0; /* for load xml */
  if (line_start.length() == 0)
  {
    line_start_ptr=0;
    start_of_line= 0;
  }
  else
  {
    line_start_ptr=(char*) line_start.ptr();
    line_start_end=line_start_ptr+line_start.length();
    start_of_line= 1;
  }
  /* If field_terminator == line_terminator, don't use line_terminator */
  if (field_term_length == line_term_length &&
      !memcmp(field_term_ptr,line_term_ptr,field_term_length))
  {
    line_term_length=0;
    line_term_ptr=(char*) "";
  }
  enclosed_char= (enclosed_length=enclosed_par.length()) ?
    (uchar) enclosed_par[0] : INT_MAX;
  field_term_char= field_term_length ? (uchar) field_term_ptr[0] : INT_MAX;
  line_term_char= line_term_length ? (uchar) line_term_ptr[0] : INT_MAX;


  /* Set of a stack for unget if long terminators */
  uint length= max(cs->mbmaxlen, max(field_term_length, line_term_length)) + 1;
  set_if_bigger(length,line_start.length());
  stack=stack_pos=(int*) sql_alloc(sizeof(int)*length);

  if (!(buffer=(uchar*) my_malloc(buff_length+1,MYF(0))))
    error=1; /* purecov: inspected */
  else
  {
    end_of_buff=buffer+buff_length;
    if (init_io_cache(&cache,(get_it_from_net) ? -1 : file, 0,
		      (get_it_from_net) ? READ_NET :
		      (is_fifo ? READ_FIFO : READ_CACHE),0L,1,
		      MYF(MY_WME)))
    {
      my_free(buffer); /* purecov: inspected */
      buffer= NULL;
      error=1;
    }
    else
    {
      /*
	init_io_cache() will not initialize read_function member
	if the cache is READ_NET. So we work around the problem with a
	manual assignment
      */
      need_end_io_cache = 1;

#ifndef EMBEDDED_LIBRARY
      if (get_it_from_net)
	cache.read_function = _my_b_net_read;

      if (mysql_bin_log.is_open())
	cache.pre_read = cache.pre_close =
	  (IO_CACHE_CALLBACK) log_loaded_block;
#endif
    }
  }
}

void READER::init_io(bool get_it_from_net, bool is_fifo){
  init_io_cache(&cache,(get_it_from_net) ? -1 : file, 0,
		      (get_it_from_net) ? READ_NET :
		      (is_fifo ? READ_FIFO : READ_CACHE),0L,1,
		      MYF(MY_WME));
  need_end_io_cache = 1;
  eof = false;
}

READER::~READER()
{
  map<int,BufStruct>::iterator it;
  for(it=buffers.begin(); it!=buffers.end(); it++) {
    uchar* add = it->second.request_pos;
    // Don't want to free this buffer twice!!
    if(add!=cache.buffer)
      delete[] add;
  }

  while(!freeBuffers.empty()) {
    uchar* add = freeBuffers.back();
    delete[] add;
    freeBuffers.pop_back();
  }
  
  if (need_end_io_cache)
    ::end_io_cache(&cache);

  if (buffer != NULL)
    my_free(buffer);
  List_iterator<XMLTAG> xmlit(taglist);
  XMLTAG *t;
  while ((t= xmlit++))
    delete(t);
}

inline int READER::terminator(const char *ptr,uint length)
{
  int chr=0;					// Keep gcc happy
  uint i;
  for (i=1 ; i < length ; i++)
  {
    if ((chr=GET) != *++ptr)
    {
      break;
    }
  }
  if (i == length)
    return 1;
  PUSH(chr);
  while (i-- > 1)
    PUSH((uchar) *--ptr);
  return 0;
}


int READER::read_field()
{
  int chr,found_enclosed_char;
  uchar *to,*new_buffer;

  found_null=0;
  if (found_end_of_line)
    return 1;					// One have to call next_line

  /* Skip until we find 'line_start' */

  if (start_of_line)
  {						// Skip until line_start
    start_of_line=0;
    if (find_start_of_fields())
      return 1;
  }
  if ((chr=GET) == my_b_EOF)
  {
    found_end_of_line=eof=1;
    return 1;
  }
  to=buffer;
  if (chr == enclosed_char)
  {
    found_enclosed_char=enclosed_char;
    *to++=(uchar) chr;				// If error
  }
  else
  {
    found_enclosed_char= INT_MAX;
    PUSH(chr);
  }

  for (;;)
  {
    while ( to < end_of_buff)
    {
      chr = GET;
      if (chr == my_b_EOF)
	goto found_eof;
      if (chr == escape_char)
      {
	if ((chr=GET) == my_b_EOF)
	{
	  *to++= (uchar) escape_char;
	  goto found_eof;
	}
        /*
          When escape_char == enclosed_char, we treat it like we do for
          handling quotes in SQL parsing -- you can double-up the
          escape_char to include it literally, but it doesn't do escapes
          like \n. This allows: LOAD DATA ... ENCLOSED BY '"' ESCAPED BY '"'
          with data like: "fie""ld1", "field2"
         */
        if (escape_char != enclosed_char || chr == escape_char)
        {
          *to++ = (uchar) unescape((char) chr);
          continue;
        }
        PUSH(chr);
        chr= escape_char;
      }
#ifdef ALLOW_LINESEPARATOR_IN_STRINGS
      if (chr == line_term_char)
#else
      if (chr == line_term_char && found_enclosed_char == INT_MAX)
#endif
      {
	if (terminator(line_term_ptr,line_term_length))
	{					// Maybe unexpected linefeed
	  enclosed=0;
	  found_end_of_line=1;
	  row_start=buffer;
	  row_end=  to;
	  return 0;
	}
      }
      if (chr == found_enclosed_char)
      {
	if ((chr=GET) == found_enclosed_char)
	{					// Remove dupplicated
	  *to++ = (uchar) chr;
	  continue;
	}
	// End of enclosed field if followed by field_term or line_term
	if (chr == my_b_EOF ||
	    (chr == line_term_char && terminator(line_term_ptr,
						line_term_length)))
	{					// Maybe unexpected linefeed
	  enclosed=1;
	  found_end_of_line=1;
	  row_start=buffer+1;
	  row_end=  to;
	  return 0;
	}
	if (chr == field_term_char &&
	    terminator(field_term_ptr,field_term_length))
	{
	  enclosed=1;
	  row_start=buffer+1;
	  row_end=  to;
	  return 0;
	}
	/*
	  The string didn't terminate yet.
	  Store back next character for the loop
	*/
	PUSH(chr);
	/* copy the found term character to 'to' */
	chr= found_enclosed_char;
      }
      else if (chr == field_term_char && found_enclosed_char == INT_MAX)
      {
	if (terminator(field_term_ptr,field_term_length))
	{
	  enclosed=0;
	  row_start=buffer;
	  row_end=  to;
	  return 0;
	}
      }
#ifdef USE_MB
      if (my_mbcharlen(read_charset, chr) > 1 &&
          to + my_mbcharlen(read_charset, chr) <= end_of_buff)
      {
        uchar* p= (uchar*) to;
        int ml, i;
        *to++ = chr;

        ml= my_mbcharlen(read_charset, chr);

        for (i= 1; i < ml; i++) 
        {
          chr= GET;
          if (chr == my_b_EOF)
          {
            /*
             Need to back up the bytes already ready from illformed
             multi-byte char 
            */
            to-= i;
            goto found_eof;
          }
          *to++ = chr;
        }
        if (my_ismbchar(read_charset,
                        (const char *)p,
                        (const char *)to))
          continue;
        for (i= 0; i < ml; i++)
          PUSH((uchar) *--to);
        chr= GET;
      }
#endif
      *to++ = (uchar) chr;
    }
    /*
    ** We come here if buffer is too small. Enlarge it and continue
    */
    if (!(new_buffer=(uchar*) my_realloc((char*) buffer,buff_length+1+IO_SIZE,
					MYF(MY_WME))))
      return (error=1);
    to=new_buffer + (to-buffer);
    buffer=new_buffer;
    buff_length+=IO_SIZE;
    end_of_buff=buffer+buff_length;
  }

found_eof:
  enclosed=0;
  found_end_of_line=eof=1;
  row_start=buffer;
  row_end=to;
  return 0;
}

/*
  Read a row with fixed length.

  NOTES
    The row may not be fixed size on disk if there are escape
    characters in the file.

  IMPLEMENTATION NOTE
    One can't use fixed length with multi-byte charset **

  RETURN
    0  ok
    1  error
*/

int READER::read_fixed_length()
{
  int chr;
  uchar *to;
  if (found_end_of_line)
    return 1;					// One have to call next_line

  if (start_of_line)
  {						// Skip until line_start
    start_of_line=0;
    if (find_start_of_fields())
      return 1;
  }

  to=row_start=buffer;
  while (to < end_of_buff)
  {
    if ((chr=GET) == my_b_EOF)
      goto found_eof;
    if (chr == escape_char)
    {
      if ((chr=GET) == my_b_EOF)
      {
	*to++= (uchar) escape_char;
	goto found_eof;
      }
      *to++ =(uchar) unescape((char) chr);
      continue;
    }
    if (chr == line_term_char)
    {
      if (terminator(line_term_ptr,line_term_length))
      {						// Maybe unexpected linefeed
	found_end_of_line=1;
	row_end=  to;
	return 0;
      }
    }
    *to++ = (uchar) chr;
  }
  row_end=to;					// Found full line
  return 0;

found_eof:
  found_end_of_line=eof=1;
  row_start=buffer;
  row_end=to;
  return to == buffer ? 1 : 0;
}

int READER::set_checkpoint() {
  // We can delete all the unused buffers when next_line is called

  line_start = cache.read_pos-cache.request_pos;
  line_end = cache.read_end-cache.request_pos;

  if(last_start_pos<read_pos) {
    last_start_pos = read_pos;
 
    int d = read_pos - 1;
    while(true) {
      if(buffers.find(d)==buffers.end())
        break;
      uchar* dbuff = buffers[d].request_pos;
      freeBuffers.push_back(dbuff);
      buffers.erase(d);
      d--;
    }
  }

  return 0;
}

int READER::reset_line() {
  cache.request_pos = buffers[last_start_pos].request_pos;
  cache.buffer = buffers[last_start_pos].request_pos;
  cache.read_pos = cache.request_pos+line_start;
  cache.read_end = cache.request_pos+line_end;
  read_pos = last_start_pos;

  found_end_of_line = false;
  
  return 0;
}

int READER::next_line_set() {
  int l = next_line();
  set_checkpoint();

  return l;
}

int READER::next_line()
{
  line_cuted=0;
  start_of_line= line_start_ptr != 0;
  if (found_end_of_line || eof)
  {
    found_end_of_line=0;
    return eof;
  }
  found_end_of_line=0;
  if (!line_term_length)
    return 0;					// No lines
  for (;;)
  {
    int chr = GET;
#ifdef USE_MB
    if (chr == my_b_EOF)
    {
      eof= 1;
      return 1;
    }
   if (my_mbcharlen(read_charset, chr) > 1)
   {
       for (uint i=1;
            chr != my_b_EOF && i<my_mbcharlen(read_charset, chr);
            i++)
	   chr = GET;
       if (chr == escape_char)
	   continue;
   }
#endif
   if (chr == my_b_EOF)
   {
      eof=1;
      return 1;
    }
    if (chr == escape_char)
    {
      line_cuted=1;
      if (GET == my_b_EOF)
	return 1;
      continue;
    }
    if (chr == line_term_char && terminator(line_term_ptr,line_term_length))
      return 0;
    line_cuted=1;
  }
}


bool READER::find_start_of_fields()
{
  int chr;
 try_again:
  do
  {
    if ((chr=GET) == my_b_EOF)
    {
      found_end_of_line=eof=1;
      return 1;
    }
  } while ((char) chr != line_start_ptr[0]);
  for (const char *ptr=line_start_ptr+1 ; ptr != line_start_end ; ptr++)
  {
    chr=GET;					// Eof will be checked later
    if ((char) chr != *ptr)
    {						// Can't be line_start
      PUSH(chr);
      while (--ptr != line_start_ptr)
      {						// Restart with next char
	PUSH((uchar) *ptr);
      }
      goto try_again;
    }
  }
  return 0;
}


/*
  Clear taglist from tags with a specified level
*/
int READER::clear_level(int level_arg)
{
  DBUG_ENTER("READER::read_xml clear_level");
  List_iterator<XMLTAG> xmlit(taglist);
  xmlit.rewind();
  XMLTAG *tag;
  
  while ((tag= xmlit++))
  {
     if(tag->level >= level_arg)
     {
       xmlit.remove();
       delete tag;
     }
  }
  DBUG_RETURN(0);
}


/*
  Convert an XML entity to Unicode value.
  Return -1 on error;
*/
static int
my_xml_entity_to_char(const char *name, uint length)
{
  if (length == 2)
  {
    if (!memcmp(name, "gt", length))
      return '>';
    if (!memcmp(name, "lt", length))
      return '<';
  }
  else if (length == 3)
  {
    if (!memcmp(name, "amp", length))
      return '&';
  }
  else if (length == 4)
  {
    if (!memcmp(name, "quot", length))
      return '"';
    if (!memcmp(name, "apos", length))
      return '\'';
  }
  return -1;
}


/**
  @brief Convert newline, linefeed, tab to space
  
  @param chr    character
  
  @details According to the "XML 1.0" standard,
           only space (#x20) characters, carriage returns,
           line feeds or tabs are considered as spaces.
           Convert all of them to space (#x20) for parsing simplicity.
*/
static int
my_tospace(int chr)
{
  return (chr == '\t' || chr == '\r' || chr == '\n') ? ' ' : chr;
}


/*
  Read an xml value: handle multibyte and xml escape
*/
int READER::read_value(int delim, String *val)
{
  int chr;
  String tmp;

  for (chr= GET; my_tospace(chr) != delim && chr != my_b_EOF;)
  {
#ifdef USE_MB
    if (my_mbcharlen(read_charset, chr) > 1)
    {
      DBUG_PRINT("read_xml",("multi byte"));
      int i, ml= my_mbcharlen(read_charset, chr);
      for (i= 1; i < ml; i++) 
      {
        val->append(chr);
        /*
          Don't use my_tospace() in the middle of a multi-byte character
          TODO: check that the multi-byte sequence is valid.
        */
        chr= GET; 
        if (chr == my_b_EOF)
          return chr;
      }
    }
#endif
    if(chr == '&')
    {
      tmp.length(0);
      for (chr= my_tospace(GET) ; chr != ';' ; chr= my_tospace(GET))
      {
        if (chr == my_b_EOF)
          return chr;
        tmp.append(chr);
      }
      if ((chr= my_xml_entity_to_char(tmp.ptr(), tmp.length())) >= 0)
        val->append(chr);
      else
      {
        val->append('&');
        val->append(tmp);
        val->append(';'); 
      }
    }
    else
      val->append(chr);
    chr= GET;
  }            
  return my_tospace(chr);
}


/*
  Read a record in xml format
  tags and attributes are stored in taglist
  when tag set in ROWS IDENTIFIED BY is closed, we are ready and return
*/
int READER::read_xml()
{
  DBUG_ENTER("READER::read_xml");
  int chr, chr2, chr3;
  int delim= 0;
  String tag, attribute, value;
  bool in_tag= false;
  
  tag.length(0);
  attribute.length(0);
  value.length(0);
  
  for (chr= my_tospace(GET); chr != my_b_EOF ; )
  {
    switch(chr){
    case '<':  /* read tag */
        /* TODO: check if this is a comment <!-- comment -->  */
      chr= my_tospace(GET);
      if(chr == '!')
      {
        chr2= GET;
        chr3= GET;
        
        if(chr2 == '-' && chr3 == '-')
        {
          chr2= 0;
          chr3= 0;
          chr= my_tospace(GET);
          
          while(chr != '>' || chr2 != '-' || chr3 != '-')
          {
            if(chr == '-')
            {
              chr3= chr2;
              chr2= chr;
            }
            else if (chr2 == '-')
            {
              chr2= 0;
              chr3= 0;
            }
            chr= my_tospace(GET);
            if (chr == my_b_EOF)
              goto found_eof;
          }
          break;
        }
      }
      
      tag.length(0);
      while(chr != '>' && chr != ' ' && chr != '/' && chr != my_b_EOF)
      {
        if(chr != delim) /* fix for the '<field name =' format */
          tag.append(chr);
        chr= my_tospace(GET);
      }
      
      // row tag should be in ROWS IDENTIFIED BY '<row>' - stored in line_term 
      if((tag.length() == line_term_length -2) &&
         (strncmp(tag.c_ptr_safe(), line_term_ptr + 1, tag.length()) == 0))
      {
        DBUG_PRINT("read_xml", ("start-of-row: %i %s %s", 
                                level,tag.c_ptr_safe(), line_term_ptr));
      }
      
      if(chr == ' ' || chr == '>')
      {
        level++;
        clear_level(level + 1);
      }
      
      if (chr == ' ')
        in_tag= true;
      else 
        in_tag= false;
      break;
      
    case ' ': /* read attribute */
      while(chr == ' ')  /* skip blanks */
        chr= my_tospace(GET);
      
      if(!in_tag)
        break;
      
      while(chr != '=' && chr != '/' && chr != '>' && chr != my_b_EOF)
      {
        attribute.append(chr);
        chr= my_tospace(GET);
      }
      break;
      
    case '>': /* end tag - read tag value */
      in_tag= false;
      chr= read_value('<', &value);
      if(chr == my_b_EOF)
        goto found_eof;
      
      /* save value to list */
      if(tag.length() > 0 && value.length() > 0)
      {
        DBUG_PRINT("read_xml", ("lev:%i tag:%s val:%s",
                                level,tag.c_ptr_safe(), value.c_ptr_safe()));
        taglist.push_front( new XMLTAG(level, tag, value));
      }
      tag.length(0);
      value.length(0);
      attribute.length(0);
      break;
      
    case '/': /* close tag */
      level--;
      chr= my_tospace(GET);
      if(chr != '>')   /* if this is an empty tag <tag   /> */
        tag.length(0); /* we should keep tag value          */
      while(chr != '>' && chr != my_b_EOF)
      {
        tag.append(chr);
        chr= my_tospace(GET);
      }
      
      if((tag.length() == line_term_length -2) &&
         (strncmp(tag.c_ptr_safe(), line_term_ptr + 1, tag.length()) == 0))
      {
         DBUG_PRINT("read_xml", ("found end-of-row %i %s", 
                                 level, tag.c_ptr_safe()));
         DBUG_RETURN(0); //normal return
      }
      chr= my_tospace(GET);
      break;   
      
    case '=': /* attribute name end - read the value */
      //check for tag field and attribute name
      if(!memcmp(tag.c_ptr_safe(), STRING_WITH_LEN("field")) &&
         !memcmp(attribute.c_ptr_safe(), STRING_WITH_LEN("name")))
      {
        /*
          this is format <field name="xx">xx</field>
          where actual fieldname is in attribute
        */
        delim= my_tospace(GET);
        tag.length(0);
        attribute.length(0);
        chr= '<'; /* we pretend that it is a tag */
        level--;
        break;
      }
      
      //check for " or '
      chr= GET;
      if (chr == my_b_EOF)
        goto found_eof;
      if(chr == '"' || chr == '\'')
      {
        delim= chr;
      }
      else
      {
        delim= ' '; /* no delimiter, use space */
        PUSH(chr);
      }
      
      chr= read_value(delim, &value);
      if(attribute.length() > 0 && value.length() > 0)
      {
        DBUG_PRINT("read_xml", ("lev:%i att:%s val:%s\n",
                                level + 1,
                                attribute.c_ptr_safe(),
                                value.c_ptr_safe()));
        taglist.push_front(new XMLTAG(level + 1, attribute, value));
      }
      attribute.length(0);
      value.length(0);
      if (chr != ' ')
        chr= my_tospace(GET);
      break;
    
    default:
      chr= my_tospace(GET);
    } /* end switch */
  } /* end while */
  
found_eof:
  DBUG_PRINT("read_xml",("Found eof"));
  eof= 1;
  DBUG_RETURN(1);
}
