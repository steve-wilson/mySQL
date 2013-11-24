#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"

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

string getColName(const column &curCol)
{
	if(curCol.existingName == "")
		return curCol.newName;

	return curCol.existingName;
}

string makeAlterStatement(string tableName, const vector<column> &matches)
{
	string alterStatement = "";
	bool firstAlter = true;
	string priorColName = "";

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
                                                                                      
void prepareNaive(THD* thd, string oldSchema, string newSchema, vector<column> matches) {
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
