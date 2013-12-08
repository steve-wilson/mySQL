
#include "sql_finalize.h"
#include "simplesql.h"
#include "subtables.h"
#include <iostream>

//static const string sub_table_delimiter = "___";

using namespace std;

int finalize_schema(THD * thd){   
    string db = thd->lex->select_lex.table_list.first->db;
    string table_name = thd->lex->select_lex.table_list.first->table_name;
    int i = getHighestTID(thd, db, table_name);
    string sub_table_name = getSubTableName(table_name,i);
    Ed_connection c(thd);
    string final_table_name = getSubTableName(table_name,i+1);
    executeQuery(c, "CREATE TABLE " + db + "." + final_table_name + " LIKE " + sub_table_name);
    bool is_error = false;
    const char* error = NULL;
    executeQuery(c, "INSERT INTO " + db + "." + final_table_name + " SELECT * FROM " + table_name, is_error, error);
    if (is_error){
        executeQuery(c, "ALTER TABLE " + db + "." + final_table_name + " DROP PRIMARY KEY");
        cout << "error was:" << error << "\n";
        executeQuery(c, "INSERT INTO " + db + "." + final_table_name + " SELECT * FROM " + table_name, is_error, error);
        if (is_error)
            // don't drop all tables if this didn't work
            return 1;
    }
    executeQuery(c, "DROP VIEW " + db + "." + table_name);
    executeQuery(c, "RENAME TABLE " + db + "." + final_table_name + " TO " + db + "." + table_name);
    drop_all_subtables(thd, db, table_name);

    return 0;
}
