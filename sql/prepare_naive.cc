#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"

void AdaptSchema::prepareNaive(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr) {
    Ed_connection c(thd);
	string sqlStatement = "";
	
	if(oldSchemaDoesntExist(oldSchema))
		sqlStatement = "CREATE TABLE " + newSchema;
	else
	{
		string tableName = findTableName(newSchema);
        string dbName = (*table_list_ptr)->db;
        if(getHighestTID(thd, db, tableName)){
            executeQuery(c, "FINALIZE_SCHEMA " + db + "." tableName);
        }
		sqlStatement = makeAlterStatement(tableName, matches);
	}
	
	// Don't execute empty statements.
	if(sqlStatement.length() > 0)
		executeQuery(c, sqlStatement);
}
