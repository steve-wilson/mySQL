#ifndef LOADCSV_H
#define LOADCSV_H

#include <string>
#include <vector>
#include "types.h"
#include "reader.h"

using namespace std;

class LoadCSV {
  READER * reader;

  TypeManager tm;

  vector<string> header;

  public:

  LoadCSV(READER * reader);

  vector<string> getHeader() {
    return header;
  }

  vector<TypeInstance> calculateColumnTypes();
};

#endif
