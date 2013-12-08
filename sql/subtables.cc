
#include "subtables.h"
#include "simplesql.h"
#include <cstdlib>
#include <algorithm>

static const string column_delimiter = "___";
static const string modified_delimiter = "xxx";

#define MODIFIED_STRING "MODIFY"

static void stringToUpper(string &s)
{
 	for(unsigned int i = 0; i < s.length(); ++i)
     	s[i] = toupper(s[i]);
}

static string getCastString(typeAndMD type)
{
	// Get m and d strings
	string d, m;
	ostringstream os;
	os << type.m;
	m = os.str();
	os.str("");
	os << type.d;
	d = os.str();

	if(type.type == "BIT" || type.type == "BINARY" || type.type == "VARBINARY")
		return "BINARY";
	else if(type.type == "TINYINT" || type.type == "SMALLINT" || type.type == "MEDIUMINT"
			|| type.type == "INT" || type.type == "INTEGER" || type.type == "BIGINT")
	{
		if(type.unsignedVal)
			return "UNSIGNED";
		else 
			return "SIGNED";
	}
	else if(type.type == "REAL" || type.type == "DOUBLE" || type.type == "DOUBLE_PRECISION"
			|| type.type == "FLOAT" || type.type == "DECIMAL" || type.type == "DEC" 
			|| type.type == "NUMERIC")
	{	
		if(type.m >= 0 && type.d >= 0)
			return "DECIMAL(" + m + "," + d + ")";
		else if(type.m >=0)
			return "DECIMAL(" + m + ")";
		return "DECIMAL";
	}
	else if(type.type == "DATE")
		return "DATE";
	else if(type.type == "TIME" || type.type == "TIMESTAMP")
		return "TIME";
	else if(type.type == "DATETIME" || type.type == "YEAR")
		return "DATETIME";
	else if(type.type == "CHAR" || type.type == "VARCHAR")
	{	
		if(type.m >= 0)
			return "CHAR(" + m + ")";
		else
			return "CHAR(255)";
	}
	return "";
}

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

void drop_all_subtables(THD * thd, std::string db, std::string table_name, bool keep_latest){
    Ed_connection c(thd);
    vector<string> to_drop;
    string sub_table_name = sub_table_delimiter + table_name + sub_table_delimiter;
    List<Ed_row> results = executeQuery(c, "SHOW TABLES FROM " + db + " LIKE \"" + sub_table_name + "\%\";");
    List_iterator<Ed_row> it(results);
    Ed_row* row;
    if (!results.is_empty()){
        while((row=it++)){
            string tbl = row->get_column(0)->str;
            if (!((keep_latest) and tbl ==  getSubTableName(table_name,getHighestTID(thd, db, table_name))))
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
    map<string,string> col_name_to_type;
    map<string, set<int> > colNameToModifiedIndexes;
    Ed_row* row;

    while((row=it++)!=NULL) {
          string colName = row->get_column(0)->str;
          if(((string)row->get_column(0)->str).find(modified_delimiter + MODIFIED_STRING) != string::npos){
              size_t startOfColName = colName.find('Y') + 1;
              size_t lengthOfColName = colName.find(modified_delimiter, 5) - startOfColName;

              string baseColName = colName.substr(startOfColName, lengthOfColName);
              
              int modifiedIndex = atoi(colName.substr(colName.find(modified_delimiter, 5) + 3, string::npos).c_str());
              if(colNameToModifiedIndexes.find(baseColName) == colNameToModifiedIndexes.end())
              {
                  set<int> modifiedIndexes;
                  colNameToModifiedIndexes.insert(make_pair(baseColName, modifiedIndexes));
              }

              colNameToModifiedIndexes[baseColName].insert(modifiedIndex);
          }
          my_cols.push_back(colName);
          col_name_to_type.insert(make_pair(colName,row->get_column(1)->str));
    }

    for (vector<column>::iterator it = match_cols->begin(); it!=match_cols->end(); it++){
        string col_name = "";

        col_name = (!it->newName.empty())? it->newName : it->existingName;
        // if it->newName or it->existingName is in mycols
        if (find(my_cols.begin(),my_cols.end(),col_name)!=my_cols.end()){
            string col_to_add = col_name;
            stringToUpper(col_name_to_type[col_name]);
            if(colNameToModifiedIndexes.find(col_name) != colNameToModifiedIndexes.end()){
                  string col_to_add = "CAST(COALESCE(" + col_name;
				  set<int>::reverse_iterator sit;
                  for(sit = colNameToModifiedIndexes[col_name].rbegin(); sit != colNameToModifiedIndexes[col_name].rend(); ++sit)
                  {
                      ostringstream os; 
                      os << *sit;

                      col_to_add += "," + modified_delimiter + MODIFIED_STRING + col_name + modified_delimiter + os.str();
                      os.str("");
                  }
                      
                  col_to_add += ") AS " + getCastString(it->typeMD) + ") AS " + col_name;
            }
            else if (col_name_to_type[col_name] != toString(it->typeMD)){
                col_to_add = "CAST(" + col_name + " AS " + getCastString(it->typeMD) + ") AS " + col_name;
            }
            cols.push_back(col_to_add);
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
