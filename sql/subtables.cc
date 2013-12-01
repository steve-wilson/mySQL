
#include "subtables.h"
#include "simplesql.h"
#include <cstdlib>
#include <algorithm>

// find highest table number based on current name
// e.g. if ___table___5 is the newest table, return 5
int getHighestTID(THD* thd, string db, string table_name){
    Ed_connection c(thd);
    int max_i = 0;
    string sub_table_name = sub_table_delimiter + table_name + sub_table_delimiter;
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + sub_table_name + "\%\";");
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
string getSubTableName(string table_name,int i){
        stringstream ss;
        ss << sub_table_delimiter << table_name 
           << sub_table_delimiter << i;
        return ss.str();
}

// do a name swap between the most recent table and the view
void swapTableWithView(THD* thd, string db, string table_name){
        int i = getHighestTID(thd, db, table_name);
        // i.e., what the view will be named at the end of the swap
        string view_name = getSubTableName(table_name, i);
        Ed_connection c(thd);
        string tmp_name = "TEMP_SWAP_TABLE";
        executeQuery(c, "RENAME TABLE " + db + "." + table_name + " TO " + db + "." + tmp_name);
        executeQuery(c, "RENAME TABLE " + db + "." + view_name + " TO " + db + "." + table_name);
        executeQuery(c, "RENAME TABLE " + db + "." + tmp_name + " TO " + db + "." + view_name);
}

void drop_all_subtables(THD * thd, std::string db, std::string table_name){
    Ed_connection c(thd);
    vector<string> to_drop;
    string sub_table_name = sub_table_delimiter + table_name + sub_table_delimiter;
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + sub_table_name + "\%\";");
    List_iterator<Ed_row> it(results);
    Ed_row* row;
    if (!results.is_empty()){
        while((row=it++)){
            string tbl = row->get_column(0)->str;
            to_drop.push_back(tbl);
        }
    }
    for(vector<string>::iterator it=to_drop.begin(); it!=to_drop.end(); ++it){
        executeQuery(c, "DROP TABLE " + db + "." + *it);
    }
}

SubTable::SubTable(string name_in) :
    name(name_in) {
}

void SubTable::update_matches(THD* thd, string db, vector<column> * match_cols){

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

string SubTable::make_string(string sep, string db){
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

SubTableList::SubTableList(THD* thd, string table_name, string db_in) :
    db(db_in) {
    Ed_connection c(thd);
    string sub_table_name = sub_table_delimiter + table_name + sub_table_delimiter;
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + sub_table_name + "\%\";");
    List_iterator<Ed_row> it(results);
    Ed_row* row;
    if (!results.is_empty()){
        while((row=it++)){
            SubTable tbl(string(row->get_column(0)->str));
            tables.push_back(tbl);
        }
    }
}

void SubTableList::update_all(THD* thd, vector<column> * cols){
    for (vector<SubTable>::iterator it = tables.begin(); it!=tables.end(); it++){
        it->update_matches(thd,db,cols);
    }
}

string SubTableList::make_string(string sep){
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
