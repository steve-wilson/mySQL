#ifndef READER_H
#define READER_H

#include "sql_class.h"
#include <my_dir.h>
#include "my_sys.h"
#include "my_net.h"
#include "sql_string.h"
#include "sql_list.h"
#include <algorithm>
#include <iostream>

extern "C" {
  #include <mysql.h>
  #include <hash.h>
}

using std::min;
using std::max;
using std::cout;

class XMLTAG {
public:
  int level;
  String field;
  String value;
  XMLTAG(int l, String f, String v) {
    level= l;
    field.append(f);
    value.append(v);
  }
};

class READER {
  File        file;
  uchar        *buffer,                        /* Buffer for read text */
        *end_of_buff;                        /* Data in bufferts ends here */
  uint        buff_length,                        /* Length of buffert */
        max_length;                        /* Max length of row */
  const char *field_term_ptr, *line_term_ptr, *line_start_ptr, *line_start_end;
  uint        field_term_length,line_term_length,enclosed_length;
  int        field_term_char,line_term_char,enclosed_char,escape_char;
  int        *stack,*stack_pos;
  bool        found_end_of_line,start_of_line,eof;
  bool  need_end_io_cache;
  IO_CACHE cache;
  NET *io_net;
  int level; /* for load xml */

public:
  bool error,line_cuted,found_null,enclosed;
  uchar        *row_start,                        /* Found row starts here */
        *row_end;                        /* Found row ends here */
  const CHARSET_INFO *read_charset;

  READER(File file,uint tot_length,const CHARSET_INFO *cs,
            const String &field_term,
            const String &line_start,
            const String &line_term,
            const String &enclosed,
            int escape,bool get_it_from_net, bool is_fifo);
  ~READER();
  int read_field();
  int read_fixed_length(void);
  int next_line(void);
  char unescape(char chr);
  int terminator(const char *ptr,uint length);
  bool find_start_of_fields();
  /* load xml */
  List<XMLTAG> taglist;
  int read_value(int delim, String *val);
  int read_xml();
  int clear_level(int level);

  /*
    We need to force cache close before destructor is invoked to log
    the last read block
  */
  void end_io_cache()
  {
    ::end_io_cache(&cache);
    need_end_io_cache = 0;
  }

  /*
    Either this method, or we need to make cache public
    Arg must be set from mysql_load() since constructor does not see
    either the table or THD value
  */
  void set_io_cache_arg(void* arg) { cache.arg = arg; }
};

#endif
