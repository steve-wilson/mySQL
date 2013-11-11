#include <ctime>
#include "loadcsv.h"

std::ostream & operator<<(std::ostream & Str, TypeInstance& v) { 
  Str << v.toString();
  if(v.hasPrecision()) {
    if(v.hasScale()) {
      Str << "(" << v.precision << "," << v.scale << ")";
    } else {
      Str << "(" << v.precision << ")";
    }
  }                        
                        
  Str << (v.isUnsigned()? " UNSIGNED" : "");
  return Str;
}

void print(string tableName, vector<string> header, vector<TypeInstance> data){
  cout << tableName << "(";
  for(unsigned int i=0; i<data.size(); i++) {
    cout << header[i] << " " << data.at(i) << ((i<data.size()-1)? ", " : "");
  }
  cout << ")";

  cout << endl;
}

void print(vector<vector<string>* >* data){
  for(unsigned int i=0; i<data->size(); i++){
    for(unsigned int j=0; j<data->at(i)->size(); j++){
      cout << data->at(i)->at(j) << "\t";
    }

    cout << endl;
  }
}
/*
int main(int argc, const char* argV[]) {
  if(argc!=3) {
    cout << "USAGE: loadcsv TableName /path/path/file.csv\n";
    exit(1);
  }

  FILE * file = my_fopen(argV[2], O_RDONLY, MYF(MY_WME));
  File f = my_fileno(file);

  uint tot_length = 0;
  const CHARSET_INFO *cs = &my_charset_bin;
  const String field_term(",", cs);
  const String line_start("", cs);

  String line_term("\n", cs);
  const String enclosed("\"", cs);
  int escape = 0;
  bool get_it_from_net = false;
  bool is_fifo = false;

  READER * reader = new READER(f, tot_length, cs, field_term, line_start, line_term,
                        enclosed, escape, get_it_from_net, is_fifo);

  LoadCSV csv(reader);
  print(argV[1], csv.getHeader(), csv.calculateColumnTypes());
}
*/

LoadCSV::LoadCSV(READER * r)
{
  reader = r;
  // Get header
  while(!reader->read_field()) {
    uint length = reader->row_end-reader->row_start;
    reader->row_start[length] = '\0';
    header.push_back((char*)reader->row_start);
  }

  reader->next_line();
}

vector<TypeInstance> LoadCSV::calculateColumnTypes() {
  vector<TypeInstance> typeForRow;
  int numColumns = header.size();
			
  for(int i=0; i<numColumns; i++) {
    typeForRow.push_back(TypeInstance(TNULL, 0));
  }

  long start = time(0);			
  int count=0;

  do {
    int a = 0;
    while(!reader->read_field()) {
      uint length = reader->row_end-reader->row_start;
      reader->row_start[length] = '\0';

      typeForRow[a] = tm.leastCommonSuperType(typeForRow.at(a), tm.inferType((char*)reader->row_start, length));
      a++;
    }

    count++;
  } while(!reader->next_line());

  long duration = time(0) - start;
  cout << "loaded " << count << " rows in " << duration << " seconds\n";
			
  return typeForRow;
}
