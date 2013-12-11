#include "adapt_schema.h"
#include "sql_prepare.h"
#include "simplesql.h"
#include "subtables.h"

#include <sstream>
#include <map>
#include <set>

static const string column_delimiter = "___";
static const string modified_delimiter = "xxx";

static const int MIN_DUMMY_COLS = 7;
static const float DUMMY_COL_FRACTION = .25;

#define DUMMY_STRING "DUMMY"
#define MODIFIED_STRING "MODIFY"
#define INTEGER_STRING "INTEGER"
#define DECIMAL_STRING "DECIMAL(65,30)"
#define VARCHAR_STRING "VARCHAR(255)"

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
			newSchema += INTEGER_STRING;
		}
		else if(decimalColsToAdd > 0)
		{
			--decimalColsToAdd;
			newSchema += DECIMAL_STRING;
		}
		else
		{
			newSchema += VARCHAR_STRING;
		}
		os.str("");
	}

	newSchema += ")";

	return newSchema;
}

void correctlyProportionDummyCols(string &alterDummyString, map<string, vector<dummyCol> > &typeToDummyCol, int &numColsToAdd, string colToAdd, 
								  int &numColsToModify1, string colToModify1, int &numColsToModify2, string colToModify2)
{
	while(numColsToAdd > 0 && (numColsToModify1 < 0 || numColsToModify2 < 0))
	{
		alterDummyString += ", MODIFY ";
		
		if(numColsToModify1 < 0)
		{
			dummyCol dc = typeToDummyCol[colToModify1].back();
			typeToDummyCol[colToModify1].pop_back();
			++numColsToModify1;

			alterDummyString += dc.colName; 	
		}
		else 
		{
			dummyCol dc = typeToDummyCol[colToModify2].back();
            typeToDummyCol[colToModify2].pop_back();
			++numColsToModify2;			

			alterDummyString += dc.colName;
		}
		
		alterDummyString += " " + colToAdd;
		--numColsToAdd;	
	}
}

// Alter statement for adding dummy cols to exisiting table
string alterDummyColsStatement(map<string, vector<dummyCol> > &typeToDummyCol, set<int> &alreadyUsed, int numInts, int numDecimals, int numVarchars, float addColFraction, int numDummyColsMin)
{
	int totalCols = numInts + numDecimals + numVarchars;
	int addColsUsingFraction = addColFraction * totalCols;		// Rounds off fractional part
	float intFraction = (float) numInts / totalCols;
	float decimalFraction = (float) numDecimals / totalCols;
	float fractionPerColumn;
	int intDummyColsExisting = (int)typeToDummyCol[INTEGER_STRING].size();
	int decimalDummyColsExisting = (int)typeToDummyCol[DECIMAL_STRING].size();
	int varcharDummyColsExisting = (int)typeToDummyCol[VARCHAR_STRING].size();
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
	int varcharColsToAdd = numColsToAdd - intColsToAdd - decimalColsToAdd;

	intColsToAdd -= intDummyColsExisting;
	decimalColsToAdd -= decimalDummyColsExisting;
	varcharColsToAdd -= varcharDummyColsExisting;

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
			alterDummyString += INTEGER_STRING;
		}
		else if(decimalColsToAdd > 0)
		{
			--decimalColsToAdd;
			alterDummyString += DECIMAL_STRING;
		}
		else
		{
			--varcharColsToAdd;
			alterDummyString += VARCHAR_STRING;			
		}

		os.str("");	
	}
	
	// Need to make dummy col proportions correct that takes into account newly added cols
	// Adds more int cols from a type that has too many if necessary
	correctlyProportionDummyCols(alterDummyString, typeToDummyCol, intColsToAdd, INTEGER_STRING, 
								 decimalColsToAdd, DECIMAL_STRING, varcharColsToAdd, VARCHAR_STRING);	

	// Adds more decimal cols from a type that has too many if necessary	
	correctlyProportionDummyCols(alterDummyString, typeToDummyCol, varcharColsToAdd, VARCHAR_STRING, 
                                 decimalColsToAdd, DECIMAL_STRING, intColsToAdd, INTEGER_STRING);

	// Adds more varchar cols from a type that has too many if necessary
	correctlyProportionDummyCols(alterDummyString, typeToDummyCol, decimalColsToAdd, DECIMAL_STRING, 
                                 intColsToAdd, INTEGER_STRING, varcharColsToAdd, VARCHAR_STRING);

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
	createStatement += addDummyColsString(newSchema, numInts, numDecimals, numVarchars, DUMMY_COL_FRACTION, MIN_DUMMY_COLS);
	executeQuery(c, createStatement);

	viewStatement += viewCols + " FROM " + db + "." + dummy_table_name;
	executeQuery(c, viewStatement);
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
       
static void stringToUpper(string &s)
{
 	for(unsigned int i = 0; i < s.length(); ++i)
     	s[i] = toupper(s[i]);
}

string findDummyColType(string type)
{
	stringToUpper(type);

	if(intTypes.find(type) != intTypes.end())
		return INTEGER_STRING;
	else if(decimalTypes.find(type) != decimalTypes.end()) 
		return DECIMAL_STRING;
	
	return VARCHAR_STRING;
}

string getType(string typeString)
{
	if(typeString.find("(") != string::npos)
		return typeString.substr(0, typeString.find("("));

	return typeString.substr(0, typeString.find(" ")); 
}

void AdaptSchema::prepareDummy(THD* thd, string oldSchema, string newSchema, vector<column> matches, TABLE_LIST** table_list_ptr) 
{
    Ed_connection c(thd);
    string sqlStatement = "";
        
	TABLE_LIST* table_list = *table_list_ptr;

    string table_name = table_list->table_name;
    string db = table_list->db;

	int highestTID = getHighestTID(thd, db, table_name);
	
	if(highestTID == 0)
		++highestTID;

	string dummy_table_name = getSubTableName(table_name, highestTID);
    
	// Initial creation of table and view
	if(oldSchemaDoesntExist(oldSchema))
	{
		createTableAndView(db, dummy_table_name, table_name, newSchema, matches, c);
   	}
	// Add columns to dummy table and cast them to the view 
	else
    {
		// Maps type to dummyCol. Right now types of dummyCols can only be INTEGER, DECIMAL or TEXT.
		map<string, dummyCol> dummyFieldStrings;
		List<Ed_row> dummyFields = executeQuery(c, "describe " + db  + "." + dummy_table_name);
		List_iterator<Ed_row> dummyFieldsIT(dummyFields);
		Ed_row *row;
		string viewCols = "";
		map<string, vector<dummyCol> > typeToUnusedDummyCols;
		map<string, set<int> > colNameToModifiedIndexes;
		map<string, string> currentType;
		set<int> alreadyUsedDummyIndexes;
		vector<string> viewColumns;
		bool modifyingIndexedColumn = false;
		bool dummyTableExists = false;

		// Store all available dummy cols in dummyFieldString vector also store modified indexes 
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
					typeToUnusedDummyCols.insert(make_pair(typeToAdd, dcv));
				}

				typeToUnusedDummyCols[typeToAdd].push_back(dc);
			}
			else if(((string)row->get_column(0)->str).find(modified_delimiter + MODIFIED_STRING) != string::npos)
			{
				string modifiedName = (string)row->get_column(0)->str;
				size_t startOfColName = modifiedName.find('Y') + 1;
				size_t lengthOfColName = modifiedName.find(modified_delimiter, 5) - startOfColName;

				string colName = modifiedName.substr(startOfColName, lengthOfColName);
				
				int modifiedIndex = atoi(modifiedName.substr(modifiedName.find(modified_delimiter, 5) + 3, string::npos).c_str());

				if(colNameToModifiedIndexes.find(colName) == colNameToModifiedIndexes.end())
				{
					set<int> modifiedIndexes;
					colNameToModifiedIndexes.insert(make_pair(colName, modifiedIndexes));
				}

				colNameToModifiedIndexes[colName].insert(modifiedIndex);
			}
			else
			{
				currentType[(string)row->get_column(0)->str] = (string) row->get_column(1)->str;
				viewColumns.push_back((string)row->get_column(0)->str);
			}
		}

		List<Ed_row> availableTables = executeQuery(c, "show tables from " + db + "  like '" + dummy_table_name + "'");
		List_iterator<Ed_row> availableTablesIT(availableTables);
	
		while((row = availableTablesIT++) != NULL)
		{
			if(((string)row->get_column(0)->str) == dummy_table_name)
				dummyTableExists = true;
		}

		// Find which fields are added from original schema and which are modified from original.
		vector<column> fieldsAdded;
		vector<column> fieldsModified;
		vector<column>::iterator it;
		int numInts = 0;
		int numDecimals = 0;
		int numVarchars = 0;
		int numNewInts = 0;
		int numNewDecimals = 0;
		int numNewVarchars = 0;
		string finalViewCols = "";
		map<string, typeAndMD> colNameToTypeMD;
		bool needToExpand = false;

		// Find added and modifed cols and store in vectors. Also compute counts of existing cols for computing dummyCols to add next
		// Also maps column name to its type string
		for(it = matches.begin(); it != matches.end(); ++it)
		{
			if(finalViewCols.length() > 0)
				finalViewCols += ",";
			
			string colName = it->existingName;

			if(it->addedFromExisting)
				colName = it->newName;

			finalViewCols += colName;

			colNameToTypeMD[colName] = it->typeMD;

			if(it->typeMD.m > 255)
				needToExpand = true;

			if(intTypes.find(it->typeMD.type) != intTypes.end())
			{
				++numInts;
				
				if(it->addedFromExisting || it->changedFromExisting)
					++numNewInts;
			}
			else if(decimalTypes.find(it->typeMD.type) != decimalTypes.end())
			{
				++numDecimals;
				
				if(it->addedFromExisting || it->changedFromExisting)
					++numNewDecimals;
			}
			else
			{
             	++numVarchars;
				
				if(it->addedFromExisting || it->changedFromExisting)
					++numNewVarchars;
			}

			if(it->addedFromExisting)
			{
				viewColumns.push_back(it->newName);
				fieldsAdded.push_back(*it);
			}
			else if(it->changedFromExisting)
			{
				viewColumns.push_back(it->newName);
				fieldsModified.push_back(*it);
				
				// Want to extend table if modifying an indexed column
				modifyingIndexedColumn = columnHasIndex(it->newName);
			}
			else 
				viewColumns.push_back(it->existingName);

		}

		string alterAddColString = "";
		string alterDummyColString = "";

		// See if we can fit all the types of data into the existing dummy cols available
		int dummyColsThatFitInts = typeToUnusedDummyCols[INTEGER_STRING].size() + typeToUnusedDummyCols[DECIMAL_STRING].size() + typeToUnusedDummyCols[VARCHAR_STRING].size();
		int removeRequiredIntCols = typeToUnusedDummyCols[INTEGER_STRING].size();
		if(removeRequiredIntCols < numNewInts)
			removeRequiredIntCols = numNewInts;

		int dummyColsThatFitDecimals = dummyColsThatFitInts - removeRequiredIntCols;
		int removeRequiredDecimalCols = typeToUnusedDummyCols[DECIMAL_STRING].size();
		if(removeRequiredDecimalCols < numNewDecimals)
			removeRequiredDecimalCols = numNewDecimals;

		int dummyColsThatFitVarchars = dummyColsThatFitDecimals - removeRequiredDecimalCols;

		bool cantFitInDummyCols = numNewVarchars > dummyColsThatFitVarchars;
		
		// Need to add columns so add the exact type instead of storing in dummy cols, do this for when we cannot fit in 
		// existing dummy cols or we are modifying an existing index
		if(cantFitInDummyCols || modifyingIndexedColumn || needToExpand)
		{
			// Handle added columns
			if(fieldsAdded.size() > 0 || fieldsModified.size() > 0)
			{
				// For each added column add it to the alter column string
				for(it = fieldsAdded.begin(); it != fieldsAdded.end(); ++it)
				{
					if(alterAddColString.length() > 0)
						alterAddColString += ", ";
	
					alterAddColString += "ADD " + it->newName + " " + toString(it->typeMD);
				}

				// For each modified column modify it to the correct type
				for(it = fieldsModified.begin(); it != fieldsModified.end(); ++it)
				{
					if(alterAddColString.length() > 0)
						alterAddColString += ", ";

					alterAddColString += "MODIFY " + it->newName + " " + toString(it->typeMD);
				}
				
				// We are creating a new table so remove modified dummy cols
				set<int>::iterator sit;
				map<string, set<int> >::iterator cit;
				for(cit = colNameToModifiedIndexes.begin(); cit != colNameToModifiedIndexes.end(); ++cit)
				{
					for(sit = cit->second.begin(); sit != cit->second.end(); ++sit)
					{
						if(alterAddColString.length() > 0)
							alterAddColString += ", ";
	
						ostringstream os; 
						os << *sit;

						alterAddColString += "DROP " + modified_delimiter + MODIFIED_STRING + cit->first + modified_delimiter + os.str();
						os.str("");
					}
				}

				// Change prior renamed dummy types to actual types
				for(it = matches.begin(); it != matches.end(); ++it)
				{
					if(!it->addedFromExisting && !it->changedFromExisting)
					{	
						string typeExistingInDummyTable = currentType[it->existingName];
						stringToUpper(typeExistingInDummyTable);

						if(typeExistingInDummyTable != toString(it->typeMD))
							alterAddColString += ", MODIFY " + it->existingName + " " + toString(it->typeMD);
					}
				}

				List<Ed_row> viewFields = executeQuery(c, "describe " + db  +	"." + table_name);
		        List_iterator<Ed_row> viewFieldsIT(viewFields);

				// Find all columns in the view
				while((row = viewFieldsIT++) != NULL)
 	        	{
					if(viewCols.length() != 0)
						viewCols += ",";

					viewCols += (string)row->get_column(0)->str;
				}

				alterDummyColString = alterDummyColsStatement(typeToUnusedDummyCols, alreadyUsedDummyIndexes, numInts, numDecimals, numVarchars, DUMMY_COL_FRACTION, MIN_DUMMY_COLS); 

				// Create table like old underyling table
				string dummy_table_copy_name = dummy_table_name + "copy";
				string createTableString;
				
				if(dummyTableExists)
					createTableString = "CREATE TABLE " + db + "." + dummy_table_copy_name + " LIKE " + db + "." + dummy_table_name;
				else
					createTableString = "CREATE TABLE " + db + "." + dummy_table_copy_name + " LIKE " + db + "." + table_name;	

				executeQuery(c, createTableString);
				
				// Alter the new table adding columns and renaming old columns
				string alterTableString = "ALTER TABLE " + db + "." + dummy_table_copy_name + " " + alterAddColString + alterDummyColString;
				executeQuery(c, alterTableString);
						
				// Insert into new table from what is in the view
				string insertString = "INSERT INTO " + db + "." + dummy_table_copy_name + " (" +  viewCols + ") SELECT * FROM " + db + "." + table_name;
				executeQuery(c, insertString);

				// Drop old table
				string dropTableString;
				
				if(dummyTableExists)
					dropTableString = "DROP TABLE " + db + "." + dummy_table_name;
				else
					dropTableString = "DROP TABLE " + db + "." + table_name;

				executeQuery(c, dropTableString);

				// Rename new table to old table name
				string renameTableString = "RENAME TABLE " + db + "." + dummy_table_copy_name + " TO " + db + "." + dummy_table_name;
				executeQuery(c, renameTableString);
				
				// Now drop old view and make a new view that casts the datatypes when appropriate  
				if(dummyTableExists)
					executeQuery(c, "DROP VIEW " + db + "." + table_name);
				
				executeQuery(c, "CREATE VIEW " + db + "." + table_name + " AS SELECT " + finalViewCols + " FROM " + db + "." + dummy_table_name);

                drop_all_subtables(thd, db, table_name, true);
			}
		}
		// We can just rename the exisitng dummy cols
		else
		{
			if(fieldsAdded.size() > 0 || fieldsModified.size() > 0)
			{
				map<string, string> colNameToViewColString;

				// For each added column add it using existing dummy cols 
				for(it = fieldsAdded.begin(); it != fieldsAdded.end(); ++it) 
				{
					string type = findDummyColType(it->typeMD.type);
				
					// Update number of new types we still need
					if(type == INTEGER_STRING)
						--numNewInts;
					else if(type == DECIMAL_STRING)
						--numNewDecimals;
					else
						--numNewVarchars;

					if(alterAddColString.length() > 0)
						alterAddColString += ", ";

					// Have an available matching type dummy col 
					if(typeToUnusedDummyCols.find(type) != typeToUnusedDummyCols.end() && typeToUnusedDummyCols[type].size() > 0)
					{
						dummyCol dc = typeToUnusedDummyCols[type].back();
						typeToUnusedDummyCols[type].pop_back();
						
						alterAddColString += "CHANGE " + dc.colName + " " + it->newName + " " + type;
					}
					// Can store int in decimal or varchar
					else if(type == INTEGER_STRING)
					{
						string dummyColToStoreIn = VARCHAR_STRING;
	
						if(typeToUnusedDummyCols[DECIMAL_STRING].size() > (unsigned int)numNewDecimals)
							dummyColToStoreIn = DECIMAL_STRING;
						
						dummyCol dc = typeToUnusedDummyCols[dummyColToStoreIn].back();
						typeToUnusedDummyCols[dummyColToStoreIn].pop_back();
						
						alterAddColString += "CHANGE " + dc.colName + " " + it->newName + " " + dummyColToStoreIn;
					}
					// Can store decimal in varchar
					else if(type == DECIMAL_STRING)
					{
						dummyCol dc = typeToUnusedDummyCols[VARCHAR_STRING].back();
						typeToUnusedDummyCols[VARCHAR_STRING].pop_back();
						
						alterAddColString += "CHANGE " + dc.colName + " " + it->newName + " " + VARCHAR_STRING;
					}
					
					// Need to cast to correct type in view				
					string castString = getCastString(it->typeMD);
							
					string viewCol = "CAST(" + it->newName + " AS " + castString + ") AS " + it->newName;
					colNameToViewColString.insert(make_pair(it->newName, viewCol));	
				}

				// For each modfied column add a modified type to a dummy col
				for(it = fieldsModified.begin(); it != fieldsModified.end(); ++it)
				{
					string type = findDummyColType(it->typeMD.type);
				
					if(alterAddColString.length() > 0)
						alterAddColString += ", ";

					int currentMaxIndex = 0;

					if(colNameToModifiedIndexes.find(it->newName) != colNameToModifiedIndexes.end())
							currentMaxIndex = *colNameToModifiedIndexes[it->newName].rbegin();

					ostringstream os;
					os << currentMaxIndex + 1;
					colNameToModifiedIndexes[it->newName].insert(currentMaxIndex + 1);
					alterAddColString += "CHANGE " + it->newName + " " + modified_delimiter + MODIFIED_STRING + it->newName + modified_delimiter + os.str() + " " + currentType[it->newName] + ", ";
					os.str("");

					// Have an available matching type dummy col
					if(typeToUnusedDummyCols.find(type) != typeToUnusedDummyCols.end() && typeToUnusedDummyCols[type].size() > 0)
					{
						dummyCol dc = typeToUnusedDummyCols[type].back();
						typeToUnusedDummyCols[type].pop_back();
							
						alterAddColString += "CHANGE " + dc.colName + " " + it->newName + " " + type;
					}
					// Can store int in decimal or varchar
					else if(type == INTEGER_STRING)
					{
						string dummyColToStoreIn = VARCHAR_STRING;
	
						if(typeToUnusedDummyCols[DECIMAL_STRING].size() > (unsigned int)numNewDecimals)
							dummyColToStoreIn = DECIMAL_STRING;
						
						dummyCol dc = typeToUnusedDummyCols[dummyColToStoreIn].back();
						typeToUnusedDummyCols[dummyColToStoreIn].pop_back();
						
						alterAddColString += "CHANGE " + dc.colName + " " + it->newName + " " + dummyColToStoreIn;
					}
					// Can store decimal in varchar
					else if(type == DECIMAL_STRING)
					{
						dummyCol dc = typeToUnusedDummyCols[VARCHAR_STRING].back();
						typeToUnusedDummyCols[VARCHAR_STRING].pop_back();
						
						alterAddColString += "CHANGE " + dc.colName + " " + it->newName + " " + VARCHAR_STRING;
					}
				}
/*
				// Coalesce columns that have modified columns
				set<int>::reverse_iterator sit;
				map<string, set<int> >::iterator cit;
				for(cit = colNameToModifiedIndexes.begin(); cit != colNameToModifiedIndexes.end(); ++cit)
				{
					string viewCol = "CAST(COALESCE(" + cit->first;
					for(sit = cit->second.rbegin(); sit != cit->second.rend(); ++sit)
					{
						ostringstream os; 
						os << *sit;

						viewCol += "," + modified_delimiter + MODIFIED_STRING + cit->first + modified_delimiter + os.str();
						os.str("");
					}
						
					viewCol += ") AS " + getCastString(colNameToTypeMD[cit->first]) + ") AS " + cit->first;
					colNameToViewColString.insert(make_pair<string, string>(cit->first, viewCol)); 
				}

				for(it = matches.begin(); it != matches.end(); ++it)
				{
					string colName = it->existingName;

					if(it->addedFromExisting)
						colName = it->newName;

					string typeExistingInDummyTable = currentType[it->newName];
					stringToUpper(typeExistingInDummyTable);

					if(colNameToViewColString.find(colName) != colNameToViewColString.end())
						viewCols += colNameToViewColString[colName];
					// Cast columns stored in dummy cols in prior iterations to their respective values
					else if(typeExistingInDummyTable != toString(it->typeMD))
					{
						// Need to cast to correct type in view				
						string castString = getCastString(it->typeMD);
							
						viewCols += "CAST(" + it->existingName + " AS " + castString + ") AS " + it->existingName;
					}
					else
						viewCols += colName;

					if(it + 1 != matches.end())
						viewCols += ",";
				}
*/
				if(alterAddColString.length() > 0)
				{
					// Alter the table adding columns and renaming old columns
					string executeString = "ALTER TABLE " + db + "." + dummy_table_name + " " + alterAddColString + alterDummyColString;
					executeQuery(c, executeString);
	 
					// Now drop old view and make a new view that casts the datatypes when appropriate  
					executeQuery(c, "DROP VIEW " + db + "." + table_name);
					//executeQuery(c, "CREATE VIEW " + db + "." + table_name + " AS SELECT " + viewCols + " FROM " + db + "." + dummy_table_name);
                    executeQuery(c, makeViewStatement(db, table_name, thd, &matches));
				}
			}
		}	
	}

	// save the original table in aux list, just in case it is needed later
    // then clear the table list
    thd->lex->select_lex.table_list.save_and_clear(&thd->lex->auxiliary_table_list);

	// important: create dynamic string or it will get truncated later and cause errors
	//TODO: make sure this is deleted later
	char *dummy_table_name_cstr = new char[dummy_table_name.length()];
	strcpy(dummy_table_name_cstr, dummy_table_name.c_str());

	char * db_cstr = new char[db.length()];
	strcpy(db_cstr, db.c_str());

	// create lex string in order to create a new table identifier
	LEX_STRING table_ls = { C_STRING_WITH_LEN(dummy_table_name_cstr) };
	table_ls.length = dummy_table_name.length();

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
