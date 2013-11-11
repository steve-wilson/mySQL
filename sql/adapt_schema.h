#include "sql_class.h"
#include "sql_list.h"

enum schema_update_method {SCHEMA_UPDATE_NIAVE, SCHEMA_UPDATE_ALTER, SCHEMA_UPDATE_VIEW, SCHEMA_UPDATE_MATERIALIZED};

/*
    update schema to accomodate

    return false if successful,
             true if error occurs
*/
bool update_schema_to_accomodate_data(File file, uint tot_length, const CHARSET_INFO *cs, const String &field_term, const String &line_start, const String &line_term, const String &enclosed, int escape, bool get_it_from_net, bool is_fifo, THD* thd, sql_exchange *ex, TABLE_LIST *table_list, schema_update_method method);
