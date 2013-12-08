#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"
#include "subtables.h"
#include <cstdlib>
#include <algorithm>

using namespace std;

void AdaptSchema::prepareViews(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr) {
    TABLE_LIST* table_list = *table_list_ptr;

    string table_name = table_list->table_name;
    string db = table_list->db;

    Ed_connection c(thd);

    // Look for an exact match on table_name
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + table_name + "\";");
    // If table/view already exists
    if (!results.is_empty()){

        int i = getHighestTID(thd, db, table_name);

        // in this case, the original table exists (no view created yet)
        // so need to rename original table to subtable before continuing
        if (i==0){
            ++i;
            string orig_sub_table_name = getSubTableName(table_name, i);
            executeQuery(c,"RENAME TABLE " + db + "." + table_name + " TO " + db + "." + orig_sub_table_name);
        }

        string sub_table_name = getSubTableName(table_name, i);
        string new_sub_table_name = getSubTableName(table_name, i+1);
        // create new table with structure of current table
        executeQuery(c,"CREATE TABLE " + db + "." + new_sub_table_name + " LIKE " + db + "." + sub_table_name);

        string alter_statement = makeAlterStatement(db + "." + new_sub_table_name, matches);
        executeQuery(c, alter_statement);

        SubTableList subTables(thd, table_name, db);
        subTables.update_all(thd, &matches);

        stringstream queryStream;
        queryStream << "CREATE OR REPLACE VIEW " << db << "." << table_name << " AS SELECT ";
        queryStream << subTables.make_string("UNION ALL SELECT");

        string create_view_sql = queryStream.str();
        executeQuery(c, create_view_sql);

        // save the original table in aux list, just in case it is needed later
        // then clear the table list
        thd->lex->select_lex.table_list.save_and_clear(&thd->lex->auxiliary_table_list);

        // important: create dynamic string or it will get truncated later and cause errors
        //TODO: make sure this is deleted later
//        char * sub_table_name_cstr = new char[sub_table_name2.length()];
        add_table_to_select_lex(thd, table_list_ptr, db, new_sub_table_name);
    }
    else{

        stringstream schemaStream;
        vector<column>::iterator it;
        schemaStream << db << "." << table_name << "(";
        it=matches.begin();
        if(it!=matches.end()) {
          schemaStream << it->newName << " " << toString(it->typeMD);
          for(it++; it!=matches.end(); it++) {
            if(!it->newName.empty())
              schemaStream << ", " << it->newName << " " << toString(it->typeMD);
          }
        }
        schemaStream << ")";

        string mergedSchema = schemaStream.str();    
        executeQuery(c, "CREATE TABLE " + mergedSchema);
        return;
    }
}
