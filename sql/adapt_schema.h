#ifndef ADAPT_SCHEMA
#define ADAPT_SCHEMA

#include "sql_class.h"
#include "sql_list.h"
#include "adapt_schema_methods.h"
#include "computeNextSchema.h"
#include "reader.h"

using std::vector;

/*
    update schema to accomodate

    return false if successful,
             true if error occurs
*/

class AdaptSchema {
  typeManager tm;
  READER* reader;
  THD* thd;
  sql_exchange *ex;
  List<Item>* field_list;
  vector<string> header;
  schema_update_method method;
  bool relaxed_schema_inference;
  unsigned int sample_size;
  TABLE_LIST* insert_table;

  public:
  AdaptSchema(READER* reader, THD* thd, sql_exchange* ex, List<Item>* field_list, schema_update_method method, bool relaxed_schema_inference, unsigned int infer_sample_size);

  vector<string> getHeader() { return header; }

  bool update_schema_to_accomodate_data(TABLE_LIST** table_list, string& newSchema);

  string getColName(const column &curCol);

  string makeAlterStatement(string tableName, const vector<column> &matches);

  string schema_from_row(string& db, string& table_name, vector<string>& row);

  bool finalize_schema_update(THD * thd, TABLE_LIST * table_list, schema_update_method method);

  void prepareNaive(THD* thd, string oldSchema, string newSchema, vector<column> matches);
  void prepareViews(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr);
  void prepareDummy(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr);		
};

void add_table_to_select_lex(THD* thd, TABLE_LIST **table_list_ptr, string db, string table_name);

void swapTableWithView(THD * thd, string db, string table_name);

string findTableName(string &schema);
bool oldSchemaDoesntExist(string oldSchema); 
bool columnHasIndex(string colName);
#endif
