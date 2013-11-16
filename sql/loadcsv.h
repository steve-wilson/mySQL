#ifndef LOADCSV_H
#define LOADCSV_H

#include <string>
#include <vector>
#include "computeNextSchema.h"
#include "reader.h"

using namespace std;

class LoadCSV {
  string db;
  string table;

  typeManager tm;

  READER * reader;

  vector<string> header;

  public:

  LoadCSV(string db, string table, READER * reader);

  vector<string> getHeader() {
    return header;
  }

  vector<typeAndMD> calculateColumnTypes();
  string calculateSchema();
  vector<column> match(string schema1, string schema2);
};

#endif
