#include "adapt_schema.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "loadcsv.h"
#include "sql_parse.h"
#include "transaction.h"
#include "sql_prepare.h"

bool update_schema_to_accomodate_data(File file, uint tot_length, const CHARSET_INFO *cs, const String &field_term, const String &line_start, const String &line_term, const String &enclosed, int escape, bool get_it_from_net, bool is_fifo, THD* thd, sql_exchange *ex, TABLE_LIST *table_list, schema_update_method method){
    /*
        In this function we can make whatever calls are necessary
        in order to update the schema to accomodate the incoming data
        from the file. Should be able to get file handle and all other
        required information from the thd struct, but may require digging.

        Update: ex->file_name is the name of the csv file
    */

    READER reader(file, tot_length,
                        cs, field_term, line_start, line_term, enclosed,
                        escape, get_it_from_net, is_fifo);

    LoadCSV csv(&reader);
 
    vector<string> header = csv.getHeader();
    vector<TypeInstance> data = csv.calculateColumnTypes();

    std::stringstream ss;
    ss << "CREATE TABLE " << table_list->db << "." << table_list->table_name << "(";
    for(uint i=0; i<header.size(); i++) {
      ss << header[i] << " " << data.at(i) << ((i<data.size()-1)? ", " : "");
    }
    ss << ")";

    // Printf generated schema to outfile
    std::ofstream outfile;
    outfile.open("/tmp/update_schema_artifact", ios::out);
    outfile << ss.str() << std::endl;
    outfile.close();

    LEX_STRING query = {(char*)ss.str().c_str(), ss.str().length()};

    Execute_sql_statement st(query);
    st.execute_server_code(thd);

    thd->reset_for_next_command();

    return 0;
}
