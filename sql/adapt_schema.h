
#ifndef ADAPT_SCHEMA
#define ADAPT_SCHEMA

enum schema_update_method {SCHEMA_UPDATE_NONE, SCHEMA_UPDATE_DEFAULT, SCHEMA_UPDATE_ALTER, SCHEMA_UPDATE_VIEW, SCHEMA_UPDATE_AUTO };

#include "sql_class.h"
#include "sql_list.h"

/*
    update schema to accomodate

    return false if successful,
             true if error occurs
*/
bool update_schema_to_accomodate_data(THD *thd, sql_exchange *ex, TABLE_LIST *table_list, schema_update_method method);

#endif
