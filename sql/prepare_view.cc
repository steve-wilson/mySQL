#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"
#include <cstdlib>
#include <algorithm>

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
            unsigned int start_pos = sub_table_name.length();
            unsigned int end_pos = tbl.length();
            // atoi returns 0 if cannot convert
            int i = atoi(tbl.substr(start_pos,end_pos).c_str());
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
        // i.e., what the view will be named at the end of the swap
        string view_name = getSubTableName(table_name, i);
        Ed_connection c(thd);
        string tmp_name = "TEMP_SWAP_TABLE";
        executeQuery(c, "RENAME TABLE " + table_name + " TO " + tmp_name);
        executeQuery(c, "RENAME TABLE " + view_name + " TO " + table_name);
        executeQuery(c, "RENAME TABLE " + tmp_name + " TO " + view_name);
}

/* Delete after one commit to repo
static vector<string> separate_string(string s, string sep, string beg=""){
        vector<string> vec;
        int start_pos = beg.length();
        int pos = s.find(sep,start_pos);
        while (pos!=string::npos) {
            vec.push_back(s.substr(start_pos,pos));
            start_pos = pos + sep.length();
            pos = s.find(sep,start_pos);
        }
        return vec;
}
*/
class SubTable{
    
    string name;
    vector<string> cols;

    public:
    SubTable(string name_in) :
        name(name_in) {
    }

    void update_matches(THD* thd, string db, vector<column> * match_cols){

        Ed_connection c(thd);

        List<Ed_row> res = executeQuery(c, "describe " + db  + "." + name); 
        List_iterator<Ed_row> it(res);
        vector<string> my_cols;
        Ed_row* row;

        while((row=it++)!=NULL) {
              my_cols.push_back(row->get_column(0)->str);
        }

        for (vector<column>::iterator it = match_cols->begin(); it!=match_cols->end(); it++){
            if (find(my_cols.begin(),my_cols.end(),it->newName)!=my_cols.end()) {
                cols.push_back(it->newName);
            }
            else{
                cols.push_back("NULL");
            }
        }
        //sanity check
        assert(cols.size()==match_cols->size());
    }

    string make_string(string sep, string db){
        stringstream ss;
        for (vector<string>::iterator it = cols.begin();
                it!=cols.end(); it++){
            ss << *it;
            if( it+1 != cols.end()){
                ss << " " << sep << " ";
            }
        }
        ss << " FROM " << db << "." << name;
        string ret = ss.str();
        return ret;
    }
};

class SubTables{

    string db;
    vector<SubTable> tables;
    
    public:

    SubTables(THD* thd, string table_name, string db_in) :
        db(db_in) {
        Ed_connection c(thd);
        string sub_table_name = sub_table_delimiter + table_name + sub_table_delimiter;
        List<Ed_row> results = executeQuery(c, "SHOW TABLES LIKE \"" + sub_table_name + "\%\";");
        List_iterator<Ed_row> it(results);
        Ed_row* row;
        if (!results.is_empty()){
            while((row=it++)){
                SubTable tbl(string(row->get_column(0)->str));
                tables.push_back(tbl);
            }
        }
    }

    void update_all(THD* thd, vector<column> * cols){
        for (vector<SubTable>::iterator it = tables.begin(); it!=tables.end(); it++){
            it->update_matches(thd,db,cols);
        }
    }

    string make_string(string sep){
        stringstream ss;
        for (vector<SubTable>::reverse_iterator rit = tables.rbegin();
                rit!=tables.rend(); ++rit){
            ss << rit->make_string(",", db);
            if( rit+1 != tables.rend()){
                ss << " " << sep << " ";
            }
        }
        string ret = ss.str();
        return ret;
    }

};

// TODO: add commands/pseudo-triggers to clean up all of the sub tables used to make views work
void prepareViews(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr) {
    TABLE_LIST* table_list = *table_list_ptr;

    string table_name = table_list->table_name;
    string db = table_list->db;

    int i = getHighestTID(thd,table_name);
    Ed_connection c(thd);

    // Look for an exact match on table_name
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + table_name + "\";");
    // If table/view already exists
    if (!results.is_empty()){

        // in this case, the original table exists (no view created yet)
        // so need to rename original table to subtable before continuing
        if (i==0){
            string sub_table_name = getSubTableName(table_name, i+1);
            executeQuery(c,"RENAME TABLE " + table_name + " TO " + sub_table_name);
            ++i;
        }
        executeQuery(c,"DROP VIEW " + table_name);
        executeQuery(c,"CREATE TABLE " + newSchema);
        string sub_table_name = getSubTableName(table_name, i+1);
        executeQuery(c,"RENAME TABLE " + table_name + " TO " + sub_table_name);

        SubTables subTables(thd, table_name, db);
        subTables.update_all(thd, &matches);

        stringstream queryStream;
        queryStream << "CREATE OR REPLACE VIEW " << table_name << " AS SELECT ";
        queryStream << subTables.make_string("UNION ALL SELECT");

        string create_view_sql = queryStream.str();
        executeQuery(c,create_view_sql);
        /*

        ////new broken method: delete after one commit to repo

        List<Ed_row> view_definition = executeQuery(c, "SELECT VIEW_DEFINITION FROM " +
                "INFORMATION_SCHEMA.VIEWS WHERE TABLE_SCHEMA = \"" + db + "\" AND " +
                "TABLE_NAME = \"" + table_name + "\"");

        List_iterator<Ed_row> it(view_definition);
        Ed_row* row;
        string previous_view = "";

        // TODO: just get first elt from List if not empty
        while((row=it++)!=NULL) {
            previous_view = row->get_column(0)->str;
        }

        vector<string> subtables = separate_string(previous_view,"union all select","select");
        vector<vector<string>> subtable_fields;

        // note that last field also includes "FROM <db>.<table>"
        // this is okay because of the way this vector is used
        for (vector<string>::iterator it = subtables.begin(); it!=subtables.end(); it++){
            subtable_fields.push_back(separate_string(*it,","));
        }

        stringstream uf;
        stringstream * sub_table_strings = new stringstream[subtable_fields.length()];

        for (vector<column>::iterator it = matches.begin(); it!=matches.end(); it++){
            if(it->addedFromExisting){
                for (int i = 0; i < sub_table_fields.length(); ++i){
                    sub_table_strings[i] << "NULL"
                }
            else{
                for (int i = 0; i < sub_table_fields.length(); ++i){
                    sub_table_strings[i] << sub_table_fields.at(i).//srw
                }
            }
            uf << it->newName;
        }
        delete sub_table_strings;

        ////end new broken method

        ///OLD METHOD (nested)
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
        /// END OLD METHOD
        */

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
