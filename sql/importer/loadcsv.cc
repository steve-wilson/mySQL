#include <iostream>
#include <ctime>
#include "loadcsv.h"
#include "csv_parser.hpp"

void print(string tableName, vector<string> header, vector<TypeInstance*> data){
  cout << tableName << "(";
  for(unsigned int i=0; i<data.size(); i++) {
    cout << header[i] << " " << *data.at(i) << ((i<data.size()-1)? ", " : "");
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

int main(int argc, const char* argV[]) {
  if(argc!=3) {
    cout << "USAGE: loadcsv TableName /path/path/file.csv\n";
    exit(1);
  }

  LoadCSV csv(argV[2]);
  print(argV[1], csv.getHeader(), csv.calculateColumnTypes());
}

LoadCSV::LoadCSV(string fn){
  fileName = fn;

  const char field_terminator = ',';
  const char line_terminator  = '\n';

  file_parser.init(fileName.c_str());
  file_parser.set_field_term_char(field_terminator);
  file_parser.set_line_term_char(line_terminator);

  // Get header
  csv_row row = file_parser.get_row();
  for(unsigned int i=0; i<row.size(); i++) {
    header.push_back(row[i]);
  }
}

vector<vector<string>* >* LoadCSV::load(int numberOfRows){
  vector<vector<string>* >* ret = new vector<vector<string>* >();
  while(file_parser.has_more_rows() && numberOfRows-->0) {
    unsigned int i = 0;

    csv_row row = file_parser.get_row();
    vector<string>* rowVector = new vector<string>();
    for (i = 0; i < row.size(); i++) {
      rowVector->push_back(row[i]);
    }

    ret->push_back(rowVector);
  }

  return ret;
}

vector<TypeInstance*> LoadCSV::calculateColumnTypes() {
  vector<TypeInstance*> typeForRow;
  int numColumns = header.size();
			
  for(int i=0; i<numColumns; i++) {
    typeForRow.push_back(new TypeInstance(TNULL, 0));
  }

  long start = time(0);			
  int count=0;
  while(file_parser.has_more_rows()) {
    unsigned int i = 0;

    csv_row row = file_parser.get_row();
    for (i = 0; i < row.size(); i++) {
      for(unsigned int a=0; a<row.size(); a++) {
        typeForRow[a] = tm.leastCommonSuperType(typeForRow.at(a), tm.inferType(row[a]));
      }
    }

    count++;
  }

  long duration = time(0) - start;
  cout << "loaded " << count << " rows in " << duration << " seconds\n";
			
  return typeForRow;
}
