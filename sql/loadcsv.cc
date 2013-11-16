#include <sstream>
#include <ctime>
#include "loadcsv.h"

/*
std::ostream & operator<<(std::ostream & Str, typeAndMD& v) { 
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

void print(string tableName, vector<string> header, vector<typeAndMD> data){
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
*/

LoadCSV::LoadCSV(string dbs, string tables, READER * r)
{
  db = dbs;
  table = tables;
  reader = r;

  // Get header
  while(!reader->read_field()) {
    uint length = reader->row_end-reader->row_start;
    reader->row_start[length] = '\0';
    header.push_back((char*)reader->row_start);
  }

  reader->next_line();
}

vector<column> LoadCSV::match(string schema1, string schema2) {
  int pos1 = schema1.find("("); 
  int pos2 = schema2.find("("); 
  string s1 = schema1.substr(pos1+1, schema1.size()-pos1-2);
  string s2 = schema2.substr(pos2+1, schema2.size()-pos2-2);
  return tm.generateNewSchema(s1, s2);
}

string LoadCSV::calculateSchema() {
  vector<string> header = getHeader();
  vector<typeAndMD> data = calculateColumnTypes();

  std::stringstream ss;
  ss << db << "." << table << "(";

  for(uint i=0; i<header.size(); i++) {
    ss << header[i] << " " << 
        toString(data.at(i)) << ((i<data.size()-1)? ", " : "");
  }

  ss << ")";

  return ss.str();
}

vector<typeAndMD> LoadCSV::calculateColumnTypes() {
  vector<typeAndMD> typeForRow;
  int numColumns = header.size();
			
  for(int i=0; i<numColumns; i++) {
    typeAndMD t = {(Type)-1, "NULL",-1,-1,false};
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
  } while(!reader->next_line());

  long duration = time(0) - start;
  cout << "loaded " << count << " rows in " << duration << " seconds\n";
			
  return typeForRow;
}
