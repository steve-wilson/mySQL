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

bool nameIsInOldAndNewSchemas(string name)
{
	return (name[0] != ',' && name[name.length() - 1] != ',');
}

bool nameIsNotInOldSchema(string name)
{
	return name[0] == ',';
}

string findOutputColName(string outputColNames)
{
    string outputColName;

    if(outputColNames[0] == ',')
        outputColName = outputColNames.substr(1, string::npos);
    else
        outputColName = outputColNames.substr(0, outputColNames.find(','));

    return outputColName;
}

// Adds nameAndType to map and removes current name and type from newSchema.
void addToMap(map<string, string> &nameToNameAndType, string &newSchema, size_t newSchemaPos, size_t delimiterLength)
{
	string name = newSchema.substr(0, newSchema.find(" "));
    string nameAndType = newSchema.substr(0, newSchemaPos);

    nameToNameAndType.insert(pair<string, string>(name, nameAndType));
    newSchema.erase(0, newSchemaPos + delimiterLength);
} 

string makeAlterStatement(string newSchema, vector<column> matches)
{
	string alterString = "";
    string delimiter = ", ";
    size_t newSchemaPos = 0;
	map<string, string> nameToNameAndType;
    bool firstAlter = true;
    string tableName = findTableName(newSchema);

	while((newSchemaPos = newSchema.find(delimiter)) != string::npos)
		addToMap(nameToNameAndType, newSchema, newSchemaPos, delimiter.length());

    //Add last col (no comma following)                                               
    if(newSchema.length() > 0)
		addToMap(nameToNameAndType, newSchema, string::npos, 0);	

	map<string, string>::iterator it;
	for(int i = 0; i < matches.size(); ++i)
    {               
        column curCol = matches[i];
		string outputColName = findOutputColName(curCol.name);
                    
        // Found name in old and new schemas              
        if(nameIsInOldAndNewSchemas(outputColName))
        {           
            if(curCol.typeMD.changedFromExisting)
            {       
                if(!firstAlter)
                    alterString += ", ";
                else
                    alterString = "ALTER TABLE " + tableName + ' ';
               
                alterString += "MODIFY " + it->second;
                firstAlter = false;
            }       
        }           
        // Can't find name in oldSchema need to add column.                                 
        else if(nameIsNotInOldSchema(outputColName))       
        {           
            if(!firstAlter)
                alterString += ", ";
            else 
                alterString = "ALTER TABLE " + tableName + ' ';

            alterString += "ADD " + it->second;
            firstAlter = false;
        }           
    }               
                    
    return alterString;
}
                                                                                      
void prepareNaive(THD* thd, string oldSchema, string newSchema, vector<column> matches) {
    Ed_connection c(thd);
	string alterStatement = makeAlterStatement(newSchema, matches);
	
	// Don't execute empty statements.
	if(alterStatement.length() > 0)
		executeQuery(c, alterStatement);
}
