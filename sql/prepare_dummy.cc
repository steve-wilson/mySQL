#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"

#include <sstream>
#include <map>
#include <set>

static const string column_delimiter = "___";

#define DUMMY_STRING "DUMMY"
#define INT_STRING "INTEGER"
#define DECIMAL_STRING "DECIMAL"
#define TEXT_STRING "TEXT"

static string intTypesArray[] = { "TINYINT", "SMALLINT", "MEDIUMINT", "INT", "INTEGER"};
static string decimalTypesArray[] = { "REAL", "DOUBLE", "FLOAT", "DEC", "DECIMAL", "NUMERIC" };

static set<string> intTypes(intTypesArray, intTypesArray + sizeof(intTypesArray) / sizeof(intTypesArray[0]) );
static set<string> decimalTypes(decimalTypesArray, decimalTypesArray + sizeof(decimalTypesArray) / sizeof(decimalTypesArray[0]) );

struct dummyCol
{
	string colName;
	int m;
	int d;
	bool unsignedVal;	
};

// Used for initial create of table for adding dummy cols
string addDummyColsString(string newSchema, int numInts, int numDecimals, int numVarchars, float addColFraction, int numDummyColsMin)
{
	int totalCols = numInts + numDecimals + numVarchars;
	int addColsUsingFraction = addColFraction * totalCols;		// Rounds off fractional part
	float intFraction = (float) numInts / totalCols;
	float decimalFraction = (float) numDecimals / totalCols;
	float fractionPerColumn;
	int numColsToAdd = addColsUsingFraction;

	// Set numColsToAdd to max of numDummyColsMin and addColsUsingFraction
	if(numDummyColsMin > addColsUsingFraction)
		numColsToAdd = numDummyColsMin;
	
	fractionPerColumn = (float) 1 / numColsToAdd;

	// Find correct number of each column to add. Add remainder of new cols as varchars.
	int intColsToAdd = (int)(intFraction / fractionPerColumn);
	int decimalColsToAdd = (int)(decimalFraction / fractionPerColumn);

	// Get only fields
	newSchema = newSchema.substr(newSchema.find('(') + 1, string::npos);

	// Strip final ')'
	newSchema = newSchema.substr(0, newSchema.length() - 1);

	ostringstream os;
	for(int i = 0; i < numColsToAdd; ++i)
	{
		os << i;
		newSchema += ", " + column_delimiter + DUMMY_STRING + os.str() + column_delimiter + " "; 
		
		if(intColsToAdd > 0)
		{
			--intColsToAdd;
			newSchema += INT_STRING;
		}
		else if(decimalColsToAdd > 0)
		{
			--decimalColsToAdd;
			newSchema += DECIMAL_STRING;
		}
		else
		{
			newSchema += TEXT_STRING;			
		}
		os.str("");
	}

	newSchema += ")";

	return newSchema;
}

// Alter statement for adding dummy cols to exisiting table
string alterDummyColsStatement(map<string, vector<dummyCol> > &typeToDummyCol, set<int> &alreadyUsed, int numInts, int numDecimals, int numVarchars, float addColFraction, int numDummyColsMin)
{
	int totalCols = numInts + numDecimals + numVarchars;
	int addColsUsingFraction = addColFraction * totalCols;		// Rounds off fractional part
	float intFraction = (float) numInts / totalCols;
	float decimalFraction = (float) numDecimals / totalCols;
	float fractionPerColumn;
	int intDummyColsExisting = (int)typeToDummyCol[INT_STRING].size();
	int decimalDummyColsExisting = (int)typeToDummyCol[DECIMAL_STRING].size();
	int varcharDummyColsExisting = (int)typeToDummyCol[TEXT_STRING].size();
	int numColsToAdd = addColsUsingFraction;
	int totalDummyColsExisting = intDummyColsExisting + decimalDummyColsExisting + varcharDummyColsExisting;
	string alterDummyString = "";
	
	// Set numColsToAdd to max of numDummyColsMin and addColsUsingFraction
	if(numDummyColsMin > addColsUsingFraction)
		numColsToAdd = numDummyColsMin;
	
	fractionPerColumn = (float) 1 / numColsToAdd;

	// Find correct number of each column to add. Add remainder of new cols as varchars.
	int intColsToAdd = (int)(intFraction / fractionPerColumn);
	int decimalColsToAdd = (int)(decimalFraction / fractionPerColumn);

	intColsToAdd -= intDummyColsExisting;
	decimalColsToAdd -= decimalDummyColsExisting;
	numColsToAdd -= totalDummyColsExisting;

	int dummyIndex = 0;
	ostringstream os;
	for(int i = 0; i < numColsToAdd; ++i)
	{
		// If we are already using the index, try another one
		while(alreadyUsed.find(dummyIndex) != alreadyUsed.end())
			++dummyIndex;

		os << dummyIndex;
		alterDummyString += ", ADD " + column_delimiter + DUMMY_STRING + os.str() + column_delimiter + " "; 
		
		++dummyIndex;

		if(intColsToAdd > 0)
		{
			--intColsToAdd;
			alterDummyString += INT_STRING;
		}
		else if(decimalColsToAdd > 0)
		{
			--decimalColsToAdd;
			alterDummyString += DECIMAL_STRING;
		}
		else
		{
			alterDummyString += TEXT_STRING;			
		}
		os.str("");	
	}

	return alterDummyString;
}

void createTableAndView(string db, string dummy_table_name, string table_name, string newSchema, vector<column> &matches, Ed_connection &c)
{
	string createStatement = "CREATE TABLE " + db + "." + dummy_table_name + "(";
	string viewStatement = "CREATE VIEW " + db + "." + table_name + " AS SELECT ";
	string viewCols = "";
	int numInts = 0;
	int numDecimals = 0;
	int numVarchars = 0;

	// Create view with actual cols
	vector<column>::iterator it;
	for(it = matches.begin(); it != matches.end(); ++it)
	{
		if(it != matches.begin())
			viewCols += ", ";
		
		if(intTypes.find(it->typeMD.type) != intTypes.end())
			++numInts;
		else if(decimalTypes.find(it->typeMD.type) != decimalTypes.end())
			++numDecimals;
		else
			++numVarchars; 
		
		viewCols += it->newName;			
	}		

	// Create table with dummy cols
	createStatement += addDummyColsString(newSchema, numInts, numDecimals, numVarchars, .25, 5);
	executeQuery(c, createStatement);

	viewStatement += viewCols + " FROM " + db + "." + dummy_table_name;
	executeQuery(c, viewStatement);
 }

string getCastString(typeAndMD type)
{
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
		return "DECIMAL";
	else if(type.type == "DATE")
		return "DATE";
	else if(type.type == "TIME" || type.type == "TIMESTAMP")
		return "TIME";
	else if(type.type == "DATETIME" || type.type == "YEAR")
		return "DATETIME";
	else if(type.type == "CHAR" || type.type == "VARCHAR")
		return "CHAR";

	return "";
}
        
string findDummyColType(string type)
{
	if(intTypes.find(type) != intTypes.end())
		return INT_STRING;
	else if(decimalTypes.find(type) != decimalTypes.end()) 
		return DECIMAL_STRING;
	
	return TEXT_STRING;
}

/*
string updateDummyCol(dummyCol &dc)
{
	string typeOfDummyCol;
	if(dc.type.find("(") != string::npos)
	{
		typeOfDummyCol = typeString.substr(0, typeString.find("("));
		
		// m and d exist
		if(typeString.find(",") != string::npos)
		{
			dc.m = atoi(typeString.substr(typeString.find("(") + 1, typeString.find(",") - typeString.find("(") - 1).c_str());
			dc.d = atoi(typeString.substr(typeString.find(",") + 1, typeString.find(")") - typeString.find(",") - 1).c_str());
		}
		else
		{
			dc.m = atoi(typeString.substr(typeString.find("(") + 1, typeString.find(")") - typeString.find("(") - 1).c_str());
			dc.d = -1;
		}
	}	
	else
	{
		typeOfDummyCol = typeString(0, typeString.find(" "));
		dc.m = -1;
		dc.d = -1;
	}

	if(typeString.find(" ") != string::npos)
		dc.unsignedVal = true;
	else
		dc.unsignedVal = false;
	
	return findDummyColType(typeOfDummyCol);
}
*/

string getType(string typeString)
{
	if(typeString.find("(") != string::npos)
		return typeString.substr(0, typeString.find("("));

	return typeString.substr(0, typeString.find(" ")); 
}

void prepareDummy(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr) 
{
    Ed_connection c(thd);
    string sqlStatement = "";
        
	TABLE_LIST* table_list = *table_list_ptr;

    string table_name = table_list->table_name;
    string db = table_list->db;

	string dummy_table_name = column_delimiter + table_name + column_delimiter;
    
	// Initial creation of table and view
	if(oldSchemaDoesntExist(oldSchema))
	{
		createTableAndView(db, dummy_table_name, table_name, newSchema, matches, c);
   	}
	// Add columns to dummy table and cast them to the view 
	else
    {
		// Maps type to dummyCol. Right now types of dummyCols can only be INTEGER, DECIMAL or VARCHAR.
		map<string, dummyCol> dummyFieldStrings;
		List<Ed_row> dummyFields = executeQuery(c, "describe " + db  + "." + dummy_table_name);
		List_iterator<Ed_row> dummyFieldsIT(dummyFields);
		Ed_row *row;
		string viewCols = "";
		map<string, vector<dummyCol> > typeToUnusedDummyCols;
		set<int> alreadyUsedDummyIndexes;

		// Store all available dummy cols in dummyFieldString vector
		while((row = dummyFieldsIT++) != NULL)
		{
			if(((string)row->get_column(0)->str).find(column_delimiter + DUMMY_STRING) != string::npos)
			{	
				dummyCol dc;
				dc.colName = (string)row->get_column(0)->str;
								
				string typeToAdd;
				string type = getType((string)row->get_column(1)->str);

				typeToAdd = findDummyColType(type);

				// These dummyCol indexes cannot be used again this time
				size_t startOfDummyIndex = dc.colName.find('Y') + 1;
             	size_t lengthOfDummyIndex = dc.colName.find('_', 5) - startOfDummyIndex;
             	int currentlyUsedDummyColIndex = atoi(dc.colName.substr(startOfDummyIndex, lengthOfDummyIndex).c_str());
             	alreadyUsedDummyIndexes.insert(currentlyUsedDummyColIndex);
	
				// If the vector doesn't already exists for that type create one and add it to the map.
				if(typeToUnusedDummyCols.find(typeToAdd) == typeToUnusedDummyCols.end())
				{
					vector<dummyCol> dcv;
					typeToUnusedDummyCols.insert(make_pair<string, vector<dummyCol> >(typeToAdd, dcv));
				}

				typeToUnusedDummyCols[typeToAdd].push_back(dc);
			}
			else
			{
				if(viewCols.length() > 0)
					viewCols += ",";
				
				viewCols += (string)row->get_column(0)->str; 
			}
		}

		// Find which fields are added from original schema and which are modified from original.
		vector<column> fieldsAdded;
		vector<column> fieldsModified;
		vector<column>::iterator it;
		bool alreadyGoingToAddCols = false;
		int numInts = 0;
		int numDecimals = 0;
		int numVarchars = 0;
	
		// Find added and modifed cols and store in vectors. Also compute counts of existing cols for computing dummyCols to add next
		for(it = matches.begin(); it != matches.end(); ++it)
		{
			if(intTypes.find(it->typeMD.type) != intTypes.end())
           		++numInts;
         	else if(decimalTypes.find(it->typeMD.type) != decimalTypes.end())
             	++numDecimals;
         	else
             	++numVarchars;
	
			if(it->addedFromExisting)
				fieldsAdded.push_back(*it);
			else if(it->changedFromExisting)
				fieldsModified.push_back(*it);
		}
	
		string alterAddColString = "";
		string alterModifyColString = "";
		string alterDummyColString = "";
		string castString = "";

		// Handle added columns
		if(fieldsAdded.size() > 0)
		{
			// For each added column see if you can add it using existing dummy cols or need to extend the table
			for(it = fieldsAdded.begin(); it != fieldsAdded.end(); ++it)
			{
				string type = findDummyColType(it->typeMD.type);
			
				if(alterAddColString.length() > 0)
					alterAddColString += ", ";

				if(type == INT_STRING)
					++numInts;
				else if(type == DECIMAL_STRING)
					++numDecimals;
				else
					++numVarchars;

				// Have an available matching type dummy col
				if(typeToUnusedDummyCols.find(type) != typeToUnusedDummyCols.end() && typeToUnusedDummyCols[type].size() > 0)
			 	{
					dummyCol dc = typeToUnusedDummyCols[type].back();
					typeToUnusedDummyCols[type].pop_back();
					
					alterAddColString += "CHANGE " + dc.colName + " " + it->newName + " " + type;
					
					// Need to cast if type is varchar and type of what we are storing is not varchar				
					if(type == TEXT_STRING && it->typeMD.type != TEXT_STRING)
					{
						string castString = getCastString(it->typeMD);
						
						if(viewCols.length() > 0)
							viewCols += ",";

						viewCols += "CAST(" + it->newName + " AS " + castString + ") AS " + it->newName;
					}
				}
				// Need to extend table. Don't remove from fieldsAdded
				else 
				{	
					alreadyGoingToAddCols = true;
					
					alterAddColString += "ADD " + it->newName + " " + toString(it->typeMD);
				}
			}

			// Add dummy cols if we are going to add cols
			if(alreadyGoingToAddCols)
			{
				alterDummyColString = alterDummyColsStatement(typeToUnusedDummyCols, alreadyUsedDummyIndexes, numInts, numDecimals, numVarchars, .25, 5);
			}

			if(alterAddColString.length() > 0)
			{
				// Alter the table adding columns and renaming old columns
				string executeString = "ALTER TABLE " + db + "." + dummy_table_name + " " + alterAddColString + alterDummyColString;
				executeQuery(c, "ALTER TABLE " + db + "." + dummy_table_name + " " + alterAddColString + alterDummyColString);
 
                // Now drop old view and make a new view that casts the datatypes when appropriate  
                executeQuery(c, "DROP VIEW " + db + "." + table_name);
                executeQuery(c, "CREATE VIEW " + db + "." + table_name + " AS SELECT " + viewCols + " FROM " + db + "." + dummy_table_name);
			}
	
		}
		
		// Handle modifying types of cols
		if(fieldsModified.size() > 0)
		{

		}
	}
}
