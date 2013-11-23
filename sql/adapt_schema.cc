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

bool update_schema_to_accomodate_data(File file, uint tot_length, const CHARSET_INFO *cs, const String &field_term, const String &line_start, const String &line_term, const String &enclosed, int escape, bool get_it_from_net, bool is_fifo, THD* thd, sql_exchange *ex, TABLE_LIST **table_list_ptr, List<Item>& fields_vars, vector<string>& header, schema_update_method method){
    /*
        In this function we can make whatever calls are necessary
        in order to update the schema to accomodate the incoming data
        from the file. Should be able to get file handle and all other
        required information from the thd struct, but may require digging.

        Update: ex->file_name is the name of the csv file
    */

    Ed_connection c(thd);
    TABLE_LIST * table_list = *table_list_ptr;

    // Get existing schema
    List<Ed_row> it = executeQuery(c, "describe " + string(table_list->db)  + "." + string(table_list->table_name)); 
    string oldSchema = resultToSchema(table_list->db, table_list->table_name, it);

    // Get new schema
    READER reader(file, tot_length,
                        cs, field_term, line_start, line_term, enclosed,
                        escape, get_it_from_net, is_fifo);
    LoadCSV csv(table_list->db, table_list->table_name, &reader);

    header = csv.getHeader();

    if(fields_vars.elements==0) {
      for(unsigned int i=0; i<header.size(); i++) {
        //string* s = new string(header[i]);
        Item_field* f = new (thd->mem_root) Item_field(thd->lex->current_context(),NullS, NullS, header[i].c_str());
        fields_vars.push_back(f);
      }
    }

    string newSchema = csv.calculateSchema();

    // Printf generated schema to outfile
    std::ofstream outfile;
    outfile.open("/tmp/update_schema_artifact", ios::out);
    outfile << oldSchema << std::endl;
    outfile << newSchema << std::endl;
    outfile.close();

    vector<column> matches = csv.match(oldSchema, newSchema);

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
        case SCHEMA_UPDATE_AUTO:
          // call to decision engine here
          break;
        case SCHEMA_UPDATE_NONE:
          // shouldn't get here, i.e. there is a check that method isn't none earlier
          break;
    }

    return 0;
}

bool finalize_schema_update(THD * thd, TABLE_LIST * table_list, schema_update_method method){
    
    if (method==SCHEMA_UPDATE_VIEW){
        string table_name = table_list->table_name;
        swapTableWithView(thd, table_name);
    }
    return 0;

}
