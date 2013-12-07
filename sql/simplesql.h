#ifndef SIMPLESQL
#define SIMPLESQL

#include "sql_prepare.h"
#include <string>

inline List<Ed_row> executeQuery(Ed_connection &c, std::string queryStr, bool& is_error, const char* error) {
    LEX_STRING query = {(char*)queryStr.c_str(), queryStr.length()};

    Execute_sql_statement st(query);
    is_error = c.execute_direct(&st);
    if(is_error)
      error = c.get_last_error();
    c.getTHD()->reset_for_next_command();

    List<Ed_row> empty;
    
    return (c.get_field_count()>0)? *c.use_result_set() : empty;
}

inline List<Ed_row> executeQuery(Ed_connection &c, std::string queryStr) {
    bool is_error = false;
    const char* error = NULL;
    return executeQuery(c, queryStr, is_error, error); 
}

#endif
