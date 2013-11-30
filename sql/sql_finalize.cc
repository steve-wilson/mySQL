
#include "sql_finalize.h"
#include "simplesql.h"
#include "subtables.h"

//static const string sub_table_delimiter = "___";

using namespace std;

int finalize_schema(THD * thd){

    string table_name = thd->lex->select_lex.table_list.first->table_name;
    int i = getHighestTID(thd, table_name);
    string sub_table_name = getSubTableName(table_name,i);
    Ed_connection c(thd);
    string final_table_name = getSubTableName(table_name,i+1);
    executeQuery(c, "CREATE TABLE " + final_table_name + " LIKE " + sub_table_name);
    executeQuery(c, "INSERT INTO " + final_table_name + " SELECT * FROM " + table_name);
    executeQuery(c, "DROP VIEW " + table_name);
    executeQuery(c, "RENAME TABLE " + final_table_name + " TO " + table_name);
    drop_all_subtables(thd, table_name);

    return 0;
}
