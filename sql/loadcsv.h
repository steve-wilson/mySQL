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

  LoadCSV(string db, string table, READER * reader, typeManager& type_manager);

  vector<string> getHeader();

  string calculateSchema(bool relaxed, unsigned int sample_size);
  vector<typeAndMD> calculateColumnTypes(int rows);
  vector<column> match(string schema1, string schema2);
};

#endif
