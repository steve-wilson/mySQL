
#include "adapt_schema.h"
#include <fstream>
#include <iostream>

bool update_schema_to_accomodate_data(THD *thd, sql_exchange *ex, TABLE_LIST *table_list, schema_update_method method){
    /*
        In this function we can make whatever calls are necessary
        in order to update the schema to accomodate the incoming data
        from the file. Should be able to get file handle and all other
        required information from the thd struct, but may require digging.

        Update: ex->file_name is the name of the csv file
    */

    //for now, just writing something to a file to prove that this function is getting called
    std::ofstream outfile;
    outfile.open("/tmp/update_schema_artifact");
    if (!outfile.is_open())
        return 1;
    outfile << "The function was actually called!\n";
    outfile.close();
    return 0;
}
