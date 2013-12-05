#ifndef ADAPT_SCHEMA
#define ADAPT_SCHEMA

#include "sql_class.h"
#include "sql_list.h"
#include "adapt_schema_methods.h"
#include "computeNextSchema.h"
#include "reader.h"

using std::vector;

static const int ROW_LIMIT = 1000;

/*
    update schema to accomodate

    return false if successful,
             true if error occurs
*/
bool update_schema_to_accomodate_data(READER& reader, THD* thd, sql_exchange *ex, TABLE_LIST **table_list_ptr, List<Item>& field_list, vector<string>& header, schema_update_method method, string& newSchema, bool relaxed_schema_inference, unsigned int infer_sample_size);

string schema_from_row(string& db, string& table_name, vector<string>& header, vector<string>& row);

bool finalize_schema_update(THD * thd, TABLE_LIST * table_list, schema_update_method method);

void prepareNaive(THD* thd, string oldSchema, string newSchema, vector<column> matches);
void prepareViews(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr);
void prepareDummy(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr);		
void swapTableWithView(THD * thd, string db, string table_name);

string findTableName(string &schema);
bool oldSchemaDoesntExist(string oldSchema); 
#endif
