#include <sstream>
#include <ctime>
#include "loadcsv.h"

string sanitize_fieldname(string& fn) {
  const char* str = fn.c_str();
  for(unsigned int i=0; i<fn.length(); i++)
    if(!((str[i]>='a'&&str[i]<='z') ||
       (str[i]>='A'&&str[i]<='Z') ||
       (str[i]>='0'&&str[i]<='9') ||
       str[i]=='$'|| str[i]=='_'
      ))
      fn.replace(i,1,"_");

  return fn;
}

LoadCSV::LoadCSV(string dbs, string tables, READER * r)
{
  db = dbs;
  table = tables;
  reader = r;
}

vector<string> LoadCSV::getHeader() {
  if(header.empty()) {
    // Get header
    while(!reader->read_field()) {
      uint length = reader->row_end-reader->row_start;
      reader->row_start[length] = '\0';
      string s = string((char*)reader->row_start);
      header.push_back(sanitize_fieldname(s));
    }  

    reader->next_line();
  }

  return header;
}

vector<column> LoadCSV::match(string schema1, string schema2) {
  int pos1 = schema1.find("("); 
  int pos2 = schema2.find("("); 
  string s1 = schema1.substr(pos1+1, schema1.size()-pos1-2);
  string s2 = schema2.substr(pos2+1, schema2.size()-pos2-2);
  return tm.generateNewSchema(s1, s2);
}

string LoadCSV::calculateSchema(bool relaxed, unsigned int sample_size) {
  vector<string> header = getHeader();
  vector<typeAndMD> data = calculateColumnTypes(rows);

  std::stringstream ss;
  ss << db << "." << table << "(";

  for(uint i=0; i<header.size(); i++) {
    ss << header[i] << " " << 
        toString(data.at(i)) << ((i<data.size()-1)? ", " : "");
  }

  ss << ")";

  return ss.str();
}

vector<typeAndMD> LoadCSV::calculateColumnTypes(int rows) {
  vector<typeAndMD> typeForRow;
  int numColumns = header.size();
			
  for(int i=0; i<numColumns; i++) {
    typeAndMD t = {(TypeWrapper::Type)-1, "NULL",-1,-1,false};
    typeForRow.push_back(t);
  }

  long start = time(0);			
  int count=0;

  do {
    int a = 0;
    while(!reader->read_field()) {
      uint length = reader->row_end-reader->row_start;
      reader->row_start[length] = '\0';

      typeAndMD inferredType = tm.inferType((char*)reader->row_start, length);
      typeForRow[a] = tm.leastCommonTypeAndMD(typeForRow.at(a), inferredType);
      a++;
    }

    count++;
  } while(!reader->next_line() && rows > count);

  long duration = time(0) - start;
  cout << "loaded " << count << " rows in " << duration << " seconds\n";
			
  return typeForRow;
}
