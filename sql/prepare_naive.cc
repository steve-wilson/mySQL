#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"

void AdaptSchema::prepareNaive(THD* thd, string oldSchema, string newSchema, vector<column> matches) {
    Ed_connection c(thd);
	string sqlStatement = "";
	
	if(oldSchemaDoesntExist(oldSchema))
		sqlStatement = "CREATE TABLE " + newSchema;
	else
	{
		string tableName = findTableName(newSchema); 
		sqlStatement = makeAlterStatement(tableName, matches);
	}
	
	// Don't execute empty statements.
	if(sqlStatement.length() > 0)
		executeQuery(c, sqlStatement);
}
