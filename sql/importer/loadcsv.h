#include <string>
#include <vector>
#include "csv_parser.hpp"
#include "types.h"

using namespace std;

class LoadCSV {
  TypeManager tm;

  string fileName;
  vector<string> header;
  csv_parser file_parser;

  public:

  LoadCSV(string fileName);

  vector<string> getHeader() {
    return header;
  }

  vector<vector<string>* >* load(int numberOfRows); 
  vector<TypeInstance*> calculateColumnTypes();
};

