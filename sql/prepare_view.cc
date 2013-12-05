#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"
#include "subtables.h"
#include <cstdlib>
#include <algorithm>

using namespace std;

// TODO: add commands/pseudo-triggers to clean up all of the sub tables used to make views work
void AdaptSchema::prepareViews(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr) {
    TABLE_LIST* table_list = *table_list_ptr;

    string table_name = table_list->table_name;
    string db = table_list->db;

    int i = getHighestTID(thd, db, table_name);
    Ed_connection c(thd);

    // Look for an exact match on table_name
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + table_name + "\";");
    // If table/view already exists
    if (!results.is_empty()){

        // in this case, the original table exists (no view created yet)
        // so need to rename original table to subtable before continuing
        if (i==0){
            string sub_table_name = getSubTableName(table_name, i+1);
            executeQuery(c,"RENAME TABLE " + db + "." + table_name + " TO " + db + "." + sub_table_name);
            ++i;
        }
        executeQuery(c,"DROP VIEW " + db + "." + table_name);
        executeQuery(c,"CREATE TABLE " + newSchema);
        string sub_table_name = getSubTableName(table_name, i+1);
        executeQuery(c,"RENAME TABLE " + db + "." + table_name + " TO " + db + "." + sub_table_name);

        SubTableList subTables(thd, table_name, db);
        subTables.update_all(thd, &matches);

        stringstream queryStream;
        queryStream << "CREATE OR REPLACE VIEW " << db << "." << table_name << " AS SELECT ";
        queryStream << subTables.make_string("UNION ALL SELECT");

        string create_view_sql = queryStream.str();
        executeQuery(c,create_view_sql);

        // save the original table in aux list, just in case it is needed later
        // then clear the table list
        thd->lex->select_lex.table_list.save_and_clear(&thd->lex->auxiliary_table_list);

        // important: create dynamic string or it will get truncated later and cause errors
        //TODO: make sure this is deleted later
//        char * sub_table_name_cstr = new char[sub_table_name2.length()];
        char * sub_table_name_cstr = new char[sub_table_name.length()];
        strcpy(sub_table_name_cstr, sub_table_name.c_str());

        char * db_cstr = new char[db.length()];
        strcpy(db_cstr, db.c_str());

        // create lex string in order to create a new table identifier
        LEX_STRING table_ls = { C_STRING_WITH_LEN(sub_table_name_cstr) };
        table_ls.length = sub_table_name.length();

        LEX_STRING db_ls = { C_STRING_WITH_LEN(db_cstr) };
        db_ls.length = db.length();

        Table_ident* tid = new Table_ident(thd, db_ls, table_ls, true);

        // add the new table (the one data should be loaded into) to the table list
        thd->lex->select_lex.add_table_to_list(thd, tid, NULL, TL_OPTION_UPDATING,
                                                TL_WRITE_DEFAULT, MDL_SHARED_WRITE, NULL, 0);

        // changing table_list itself to point to the new table
        // load data can now proceed
        *table_list_ptr = thd->lex->select_lex.table_list.first;
        // BL:  I had to add this line to get autocalculating the field list to work
        thd->lex->select_lex.context.first_name_resolution_table = thd->lex->select_lex.table_list.first;
    }
    else{
        executeQuery(c, "CREATE TABLE " + newSchema);
        return;
    }
}
