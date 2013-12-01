#ifndef SUBTABLES_H
#define SUBTABLES_H

#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>
#include "adapt_schema.h"

using namespace std;

/*
 
   Shared functions that help deal with subtables.
   Subtables store portions of a whole table during
   the schema merge process.
 
 */

const string sub_table_delimiter = "___";

// find highest table number based on current name
// e.g. if ___table___5 is the newest table, return 5
int getHighestTID(THD* thd, string db, string table_name);

// generates a delimiter-enclosed string followed by i
string getSubTableName(string table_name, int i);

// do a name swap between the most recent table and the view
void swapTableWithView(THD* thd, string db, string table_name);

// do a "drop table" on all subtables
void drop_all_subtables(THD * thd, string db, string table_name);

class SubTable{
    
    string name;
    vector<string> cols;

    public:

    SubTable(string name_in);
    void update_matches(THD* thd, string db, vector<column> * match_cols);
    string make_string(string sep, string db);
};

class SubTableList{

    string db;
    vector<SubTable> tables;
    
    public:
    SubTableList(THD* thd, string table_name, string db_in);
    void update_all(THD* thd, vector<column> * cols);

    string make_string(string sep);
};

#endif
