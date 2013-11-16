#ifndef SIMPLESQL
#define SIMPLESQL

#include "sql_prepare.h"

inline List<Ed_row> executeQuery(Ed_connection &c, string queryStr) {
    LEX_STRING query = {(char*)queryStr.c_str(), queryStr.length()};

    Execute_sql_statement st(query);
    c.execute_direct(&st);
    c.getTHD()->reset_for_next_command();

    List<Ed_row> empty;
    
    return (c.get_field_count()>0)? *c.use_result_set() : empty;
}

#endif
