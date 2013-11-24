#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"
#include <cstdlib>

using namespace std;

static const string sub_table_delimiter = "___";

// find highest table number based on current name
// e.g. if ___table___5 is the newest table, return 5
static int getHighestTID(THD* thd, string table_name){
    Ed_connection c(thd);
    int max_i = 0;
    string sub_table_name = sub_table_delimiter + table_name + sub_table_delimiter;
    List<Ed_row> results = executeQuery(c, "SHOW TABLES LIKE \"" + sub_table_name + "\%\";");
    List_iterator<Ed_row> it(results);
    Ed_row* row;
    if (!results.is_empty()){
        while((row=it++)){
            string tbl = row->get_column(0)->str;
            unsigned int pos = tbl.find(sub_table_name);
            // atoi returns 0 if cannot convert
            int i = atoi(tbl.substr(pos).c_str());
            max_i = (max_i>i) ? max_i:i;
        }
    }
    return max_i;
}

// generates a delimiter-enclosed string followed by i
static string getSubTableName(string table_name,int i){
        stringstream ss;
        ss << sub_table_delimiter << table_name 
           << sub_table_delimiter << i;
        return ss.str();
}

// do a name swap between the most recent table and the view
void swapTableWithView(THD* thd, string table_name){
        int i = getHighestTID(thd, table_name);
        string view_name = getSubTableName(table_name, i);
        Ed_connection c(thd);
        string tmp_name = "TEMP_SWAP_TABLE";
        executeQuery(c, "RENAME TABLE " + table_name + " TO " + tmp_name);
        executeQuery(c, "RENAME TABLE " + view_name + " TO " + table_name);
        executeQuery(c, "RENAME TABLE " + tmp_name + " TO " + view_name);
}

// TODO: add commands/pseudo-triggers to clean up all of the sub tables used to make views work
void prepareViews(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr) {
    TABLE_LIST* table_list = *table_list_ptr;

    string table_name = table_list->table_name;
    string db = table_list->db;

    int i = getHighestTID(thd,table_name);
    Ed_connection c(thd);
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + table_name + "\";");
    if (!results.is_empty()){

        string sub_table_name1 = getSubTableName(table_name,i+1);
        string sub_table_name2 = getSubTableName(table_name,i+2);
        executeQuery(c, "RENAME TABLE " + db+"."+table_name + " TO " + db+"."+sub_table_name1);
        executeQuery(c, "CREATE TABLE " + newSchema);
        executeQuery(c, "RENAME TABLE " + db+"."+table_name + " TO " + db+"."+sub_table_name2);

        // get fields to select from old table with NULL wherever field is missing
        // use to create the view
        stringstream uf1;
        stringstream uf2;
        for (vector<column>::iterator it = matches.begin(); it!=matches.end(); it++){
            if(it->addedFromExisting){
                uf1 << "NULL";
            }
            else{
                uf1 << it->existingName;
            }
            uf2 << it->newName;
            if( it+1 != matches.end()){
                uf1 << ", ";
                uf2 << ", ";
            }
        }
        string union_fields1 = uf1.str();
        string union_fields2 = uf2.str();
        executeQuery(c, "CREATE VIEW " + db+"."+table_name + " AS SELECT " + union_fields2 + " FROM " + db+"."+sub_table_name2 
                + " UNION ALL SELECT " + union_fields1 + " FROM " + db+"."+sub_table_name1);

        // save the original table in aux list, just in case it is needed later
        // then clear the table list
        thd->lex->select_lex.table_list.save_and_clear(&thd->lex->auxiliary_table_list);

        // important: create dynamic string or it will get truncated later and cause errors
        //TODO: make sure this is deleted later
        char * sub_table_name2_cstr = new char[sub_table_name2.length()];
        strcpy(sub_table_name2_cstr, sub_table_name2.c_str());

        char * db_cstr = new char[db.length()];
        strcpy(db_cstr, db.c_str());

        // create lex string in order to create a new table identifier
        LEX_STRING table_ls = { C_STRING_WITH_LEN(sub_table_name2_cstr) };
        table_ls.length = sub_table_name2.length();

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

        // delete this block of code after it makes it to the repo once
        /*
        TABLE_LIST *tl_ptr;
        if (!(tl_ptr = (TABLE_LIST *) thd->calloc(sizeof(TABLE_LIST))))
          DBUG_RETURN(0);
        // make a shallow copy
        *tl_ptr = *table_list;
        tl_ptr->table_name = sub_table_name2;
        tl_ptr->alias = sub_table_name2;
        tl_ptr->table_name_length= sub_table_name2.length();*/
            /*
        tl_ptr->db = table_list.db;
        tl_ptr->is_fqtn = table_list.is_fqtn;
        tl_ptr->db_length = table_list.db_length;
        tl_ptr->alias = table_list.alias*/

        // see 2nd comment
        // clear current table list, save in aux table list in case needed (necessary?)
 //       table_list.save_and_clear(&lex->auxiliary_table_list);
        //no, link into select lex i list then point table list appropriately
 //       table_list.link_in_list(newest_sub_table, &newest_sub_table->next);

       // table_list =     
            //if (!Select->add_table_to_list(YYTHD, $12, NULL, TL_OPTION_UPDATING,
              //                             $4, MDL_SHARED_WRITE, NULL, $13))
              //MYSQL_YYABORT;

        // changeLoadTarget(table_list,sub_table_name2);
        // setToMaterialize(table_name);

        // now, after data is loaded, need to:
        // 1) give table's name to view
        // 2) make table's name sub_table_name with new highest TID (Tn)
        // 3) set custom "trigger" on view that: 
        //      a) forces materialization upon update/delete
        //      b) redirects insert into Tn (see #2)
        // OR
        // 1) modify current thd's table list to be Tn instead of T
        // 2) set view and table names correctly NOW <-- doing this already
        // 3) still set custom trigger
    }
    else{
        executeQuery(c, "CREATE TABLE " + newSchema);
        return;
    }
}
