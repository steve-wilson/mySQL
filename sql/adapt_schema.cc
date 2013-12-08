#include "adapt_schema.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "loadcsv.h"
#include "sql_parse.h"
#include "transaction.h"
#include "sql_prepare.h"
#include "simplesql.h"
#include "computeNextSchema.h"
#include "subtables.h"

set<string> indexedColumns;

// Gets table name from schema and removes this from schema.
string findTableName(string &schema)
{
    string tableName = schema.substr(0, schema.find('('));
    schema = schema.substr(schema.find('(') + 1, string::npos);

    return tableName;
}

bool oldSchemaDoesntExist(string oldSchema)
{
    if((oldSchema.find(')') - oldSchema.find('(')) <= 1)
        return true;
    
    return false;
}

string toUpper(string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);

  return str;
}

string resultToSchema(string db, string table, List<Ed_row> list) {
  List_iterator<Ed_row> it(list);

  std::stringstream ss;

  ss << db << "." << table << "(";
  int first = 0;
  Ed_row* row;
  while((row=it++)!=NULL) {
    ss << ((first++)? ", " : "") << 
        row->get_column(0)->str << " " << 
        toUpper(row->get_column(1)->str);
  }
  ss << ")";

  return ss.str();
}

string AdaptSchema::getColName(const column &curCol)
{
	if(curCol.existingName == "")
		return curCol.newName;

	return curCol.existingName;
}

string AdaptSchema::makeAlterStatement(string tableName, const vector<column> &matches)
{
	string alterStatement = "";
	bool firstAlter = true;
	string priorColName = "";

    // unsigned to avoid compiler warnings, size() returns unsigned
	for(unsigned int i = 0; i < matches.size(); ++i)
    {               
        column curCol = matches[i];
		                    
        // Found name in old and new schemas              
        if(curCol.existingName.length() > 0 && curCol.newName.length() > 0)
        {           
            if(curCol.changedFromExisting)
            {       
                if(!firstAlter)
                    alterStatement += ", ";
                else
                    alterStatement = "ALTER TABLE " + tableName + ' ';
               
                alterStatement += "MODIFY " + curCol.newName + " " + toString(curCol.typeMD);
                firstAlter = false;
            }       
        }           
        // Can't find name in oldSchema need to add column.                                 
        else if(curCol.addedFromExisting)       
        {           
            if(!firstAlter)
                alterStatement += ", ";
            else 
                alterStatement = "ALTER TABLE " + tableName + ' ';

            alterStatement += "ADD " + curCol.newName + " " + toString(curCol.typeMD);

			// Add column in correct location
			if(priorColName == "")
				alterStatement += " FIRST";
			else 
				alterStatement += " AFTER " + priorColName;

            firstAlter = false;
        }

		priorColName = getColName(curCol);           
    }               
                    
    return alterStatement;
}
                                                                                      
bool columnHasIndex(string colName)
{
	if(indexedColumns.find(colName) != indexedColumns.end())
    	return true;

	return false;
}

void populateIndexedColumns(List<Ed_row> list)
{
  List_iterator<Ed_row> it(list);

  Ed_row* row;
  while((row=it++)!=NULL) {
      if(!((string)row->get_column(3)->str).empty())
      	indexedColumns.insert((string)row->get_column(0)->str);
  }
}

string AdaptSchema::schema_from_row(string& db, string& table, vector<string>& row) {
  vector<typeAndMD> types;
  for(unsigned int i=0; i<row.size(); i++) {
    types.push_back(tm.inferType((char*)row[i].c_str(), row[i].size()));
  }

  std::stringstream ss;
  ss << db << "." << table << "(";

  for(uint i=0; i<header.size(); i++) {
    ss << header[i] << " " << 
        toString(types[i]) << ((i<types.size()-1)? ", " : "");
  }

  ss << ")";

  return ss.str();
}

string AdaptSchema::makeViewStatement(string& db, const string& table_name, THD* thd){

        SubTableList subTables(thd, table_name, db);
        subTables.update_all(thd, &matches);

        stringstream queryStream;
        queryStream << "CREATE OR REPLACE VIEW " << db << "." << table_name << " AS SELECT ";
        queryStream << subTables.make_string("UNION ALL SELECT");

        return queryStream.str();

}

AdaptSchema::AdaptSchema(READER* reader_in, THD* thd_in, sql_exchange* ex_in, List<Item>* fields_vars_in, schema_update_method method_in, bool relaxed_schema_inference_in, unsigned int infer_sample_size_in)
 : tm(relaxed_schema_inference_in? typeManager::RELAXED_POW2 : typeManager::STRICT), reader(reader_in),
 thd(thd_in), ex(ex_in), field_list(fields_vars_in), method(method_in),
 relaxed_schema_inference(relaxed_schema_inference_in), sample_size(infer_sample_size_in)
{
  insert_table = thd->lex->select_lex.table_list.first;
 
  Ed_connection c(thd);


  // Find the indexed columns for a table   
  List<Ed_row> actualTableList = executeQuery(c, "describe " + (string)insert_table->db  + "." +(string)insert_table->table_name);
  populateIndexedColumns(actualTableList);
  
  // Find the indexed columns for dummy table  
  List<Ed_row> dummyList = executeQuery(c, "describe " + (string)insert_table->db  + "." + getSubTableName((string)insert_table->table_name, 1));
  populateIndexedColumns(dummyList);
}

bool AdaptSchema::update_schema_to_accomodate_data(TABLE_LIST** table_list_ptr, string& newSchema) {
    /*
        In this function we can make whatever calls are necessary
        in order to update the schema to accomodate the incoming data
        from the file. Should be able to get file handle and all other
        required information from the thd struct, but may require digging.

        Update: ex->file_name is the name of the csv file
    */
    thd->lex->select_lex.table_list.save_and_clear(&thd->lex->auxiliary_table_list);
    add_table_to_select_lex(thd, table_list_ptr, insert_table->db, insert_table->table_name);

    bool is_partial_read = sample_size>0;

    Ed_connection c(thd);
    TABLE_LIST * table_list = *table_list_ptr;

    string db(table_list->db);
    string table_name(table_list->table_name);

    // Get existing schema
    List<Ed_row> it = executeQuery(c, "describe " + db  + "." + table_name); 
    string oldSchema = resultToSchema(table_list->db, table_list->table_name, it);

    // If we cannot read the file, return an error
    if(reader->error)
      return 1;

    LoadCSV csv(table_list->db, table_list->table_name, reader, tm);

    if(header.empty())
      header = csv.getHeader();

    if(is_partial_read)
      reader->set_checkpoint();
    if(newSchema.length()==0) {
      newSchema = csv.calculateSchema(relaxed_schema_inference, sample_size);

      // Printf generated schema to outfile
      std::ofstream outfile;
      outfile.open("/tmp/update_schema_artifact", ios::out);
      outfile << oldSchema << std::endl;
      outfile << newSchema << std::endl;
      outfile.close();
    }
 
    if(is_partial_read) 
      reader->reset_line();

    cout << "existing schema: " << oldSchema << "\n";
    cout << "schema derived from file: " << newSchema << "\n";
    vector<column> matches = csv.match(oldSchema, newSchema);

    // If there were no changes, nothing should be done
    if (oldSchema != newSchema){
      switch(method) {
          //case SCHEMA_UPDATE_NAIVE:
          case SCHEMA_UPDATE_ALTER:
          // TODO: move default to AUTO once completed
          case SCHEMA_UPDATE_DEFAULT:
            prepareNaive(thd, oldSchema, newSchema, matches);
            break;
          case SCHEMA_UPDATE_VIEW:
            prepareViews(thd, oldSchema, newSchema, matches, table_list_ptr);
            break;
		  case SCHEMA_UPDATE_DUMMY:
			prepareDummy(thd, oldSchema, newSchema, matches, table_list_ptr);
			break;
		  case SCHEMA_UPDATE_AUTO:
            // call to decision engine here
            break;
          case SCHEMA_UPDATE_NONE:
            // shouldn't get here, i.e. there is a check that method isn't 'none' earlier
            break;
      }
    }

    if(field_list->elements==0) {
      for(unsigned int i=0; i<header.size(); i++) {
        Item_field* f = new (thd->mem_root) Item_field(&thd->lex->select_lex.context, NullS, NullS, header[i].c_str());
        field_list->push_back(f);
      }
    }

  return 0;
}

void add_table_to_select_lex(THD* thd, TABLE_LIST **table_list_ptr, string db, string table_name) {
  char * table_name_cstr = new char[table_name.length()];
  strcpy(table_name_cstr, table_name.c_str());
  
  char * db_cstr = new char[db.length()];
  strcpy(db_cstr, db.c_str());

  // create lex string in order to create a new table identifier
  LEX_STRING table_ls = { C_STRING_WITH_LEN(table_name_cstr) };
  table_ls.length = table_name.length();

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

bool AdaptSchema::finalize_schema_update(THD * thd, TABLE_LIST * table_list, schema_update_method method){
    if (method==SCHEMA_UPDATE_VIEW){
        string table_name = table_list->table_name;
        string db = table_list->db;
        swapTableWithView(thd, db, table_name);
    }

    return 0;
}
