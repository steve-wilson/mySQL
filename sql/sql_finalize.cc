
#include "sql_finalize.h"
#include "simplesql.h"
#include "subtables.h"
#include <iostream>

using namespace std;

static string make_field_list(vector<string> * vec){
    stringstream ss;
    for(vector<string>::iterator it=vec->begin(); it!=vec->end(); ++it){
        if(it!=vec->begin()){
            ss << ", ";
        }
        ss << *it;
    }
    return ss.str();
}

int finalize_schema(THD * thd){   
    string db = thd->lex->select_lex.table_list.first->db;
    string table_name = thd->lex->select_lex.table_list.first->table_name;
    int i = getHighestTID(thd, db, table_name);
    // If we can't find the table, just stop
    if(i==0)
      return 0;

    string sub_table_name = getSubTableName(table_name,i);
    Ed_connection c(thd);
    string final_table_name = getSubTableName(table_name,i+1);
    executeQuery(c, "CREATE TABLE " + db + "." + final_table_name + " LIKE " + db + "." + sub_table_name);

    // get all fields from view, these are the ones to use
    List<Ed_row> res = executeQuery(c, "describe " + db  + "." + table_name); 
    List_iterator<Ed_row> it(res);
    vector<string> final_cols;
    Ed_row* row;

    while((row=it++)!=NULL) {
        final_cols.push_back(row->get_column(0)->str);
    }

    // search all fields actually in the table (may include dummy columns)
    string alter_statement = "ALTER TABLE " + db + "." + final_table_name;
    List<Ed_row> final_res = executeQuery(c, "describe " + db  + "." + final_table_name); 
    List_iterator<Ed_row> final_it(final_res);
    Ed_row* final_row;

    bool dropping = false;
    while((final_row=final_it++)!=NULL) {
        string field = final_row->get_column(0)->str;
        // if field is not in final column list
        if (find(final_cols.begin(), final_cols.end(), field) == final_cols.end()){
            if (dropping) 
                alter_statement+= ",";
            else
                dropping = true;
            alter_statement += " DROP COLUMN " + field;
        }
    }

    // drop any dummy/extra columns from the table into which data will be inserted
    executeQuery(c, alter_statement);

    bool is_error = false;
    const char* error = NULL;

    executeQuery(c, "INSERT INTO " + db + "." + final_table_name + " SELECT " + make_field_list(&final_cols) + " FROM " + db + "." + table_name, is_error, error);

    if (is_error){
        executeQuery(c, "ALTER TABLE " + db + "." + final_table_name + " DROP PRIMARY KEY");
        cout << "error was:" << error << "\n";
        executeQuery(c, "INSERT INTO " + db + "." + final_table_name + " SELECT * FROM " + table_name, is_error, error);

        if (is_error)
        {
            // finalize not working, undo create table
            executeQuery(c, "DROP TABLE " + db + "." + final_table_name);
            // don't drop all tables if this didn't work
            return 1;
        }
    }

    executeQuery(c, "DROP VIEW " + db + "." + table_name);
    executeQuery(c, "RENAME TABLE " + db + "." + final_table_name + " TO " + db + "." + table_name);
    drop_all_subtables(thd, db, table_name);

    return 0;
}
