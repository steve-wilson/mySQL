#include <iostream>
#include <string>
#include <cstring>
#include <set>
#include <map>
#include <deque>
#include <sstream>
#include <vector>
#include <locale>
#include "computeNextSchema.h"

using namespace std;

#define UNSIGNED_STRING "UNSIGNED"

string mTypesArray[] = { "BIT", "TINYINT", "SMALLINT", "MEDIUMINT", "INT", "INTEGER", "BIGINT", "CHAR", "VARCHAR",
	"BINARY", "VARBINARY", "REAL", "DOUBLE", "FLOAT", "DEC", "DECIMAL", "NUMERIC" };
string dTypesArray[] = { "REAL", "DOUBLE", "FLOAT", "DEC", "DECIMAL", "NUMERIC" };
string intTypesArray[] = { "TINYINT", "SMALLINT", "MEDIUMINT", "INT", "INTEGER", "BIGINT" };

set<string> mTypes(mTypesArray, mTypesArray + sizeof(mTypesArray) / sizeof(mTypesArray[0]) );
set<string> dTypes(dTypesArray, dTypesArray + sizeof(dTypesArray) / sizeof(dTypesArray[0]) );
set<string> intTypes(intTypesArray, intTypesArray + sizeof(intTypesArray) / sizeof(intTypesArray[0]) );
		
map<string, TypeWrapper::Type> stringToEnum;
map<TypeWrapper::Type, string> enumToString;

void lcs::printlctMatrix()
{
	for(TypeWrapper::Type t1 = TypeWrapper::BIT; t1 < TypeWrapper::TYPECOUNT; t1 = TypeWrapper::Type(t1 + 1))
		for(TypeWrapper::Type t2 = TypeWrapper::BIT; t2 < TypeWrapper::TYPECOUNT; t2 = TypeWrapper::Type(t2 + 1))
		{
			TypeWrapper::Type type = getLeastCommonSupertype(t1, t2);
			if(type < TypeWrapper::TYPECOUNT)
				cout << "lct of " << enumToString[t1] << " and " << enumToString[t2] << " = " << enumToString[static_cast<TypeWrapper::Type>(type)] << endl;
		}
}

void lcs::printOrderStack()
{
	for(unsigned int i = 0; i < orderStack.size(); ++i)
	{
		cout << orderStack[i] << " ";
	}

	cout << endl;
}

void lcs::printTypesAndParents()
{
	map<TypeWrapper::Type, set<TypeWrapper::Type> >::iterator it;
    for(it = typeToParents.begin(); it != typeToParents.end(); ++it)
    {
        cout << it->first;
        set<TypeWrapper::Type>::iterator it2;
        for(it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            cout << " " << *it2;
        }
        cout << endl;
    }
}

void lcs::initializeGraph(Graph &g)
{
	g.addEdge("YEAR", "INTEGER");
    g.addEdge("YEAR", "DATE");
    g.addEdge("DATE", "DATETIME");
    g.addEdge("TIME", "DATETIME");
    g.addEdge("TIMESTAMP", "VARCHAR");
    g.addEdge("DATETIME", "VARCHAR");
	g.addEdge("TINYINT", "SMALLINT");
	g.addEdge("SMALLINT", "MEDIUMINT");
	g.addEdge("MEDIUMINT", "INT");
	g.addEdge("INT", "INTEGER");
	g.addEdge("INTEGER", "BIGINT");
	g.addEdge("BIGINT", "NUMERIC");
    g.addEdge("NUMERIC", "DECIMAL");
	g.addEdge("DECIMAL", "FLOAT");
	g.addEdge("FLOAT", "REAL");
	g.addEdge("REAL", "DOUBLE_PRECISION");
	g.addEdge("DOUBLE_PRECISION", "DOUBLE");
    g.addEdge("DOUBLE", "VARCHAR");
    g.addEdge("VARCHAR", "TINYTEXT");
	g.addEdge("TINYTEXT", "TEXT");
	g.addEdge("TEXT", "MEDIUMTEXT");
	g.addEdge("MEDIUMTEXT", "LONGTEXT");
	g.addEdge("BIT", "VARBINARY");
	g.addEdge("BIT", "VARCHAR");
    g.addEdge("VARBINARY", "TINYBLOB");
	g.addEdge("TINYBLOB", "TINYTEXT");
	g.addEdge("TINYBLOB", "BLOB");
	g.addEdge("BLOB", "TEXT");
	g.addEdge("BLOB", "MEDIUMBLOB");
	g.addEdge("MEDIUMBLOB", "MEDIUMTEXT");
	g.addEdge("MEDIUMBLOB", "LONGBLOB");
	g.addEdge("LONGBLOB", "LONGTEXT");
	g.addEdge("CHAR", "VARCAHR");
	g.addEdge("BINARY", "VARBINARY");
}

void initializeStringsAndEnumsMaps()
{
	stringToEnum.insert(pair<string, TypeWrapper::Type>("BIT", TypeWrapper::BIT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::BIT, "BIT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("TINYINT", TypeWrapper::TINYINT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::TINYINT, "TINYINT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("SMALLINT", TypeWrapper::SMALLINT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::SMALLINT, "SMALLINT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("MEDIUMINT", TypeWrapper::MEDIUMINT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::MEDIUMINT, "MEDIUMINT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("INT", TypeWrapper::INT));
    enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::INT, "INT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("INTEGER", TypeWrapper::INTEGER));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::INTEGER, "INTEGER"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("BIGINT", TypeWrapper::BIGINT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::BIGINT, "BIGINT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("REAL", TypeWrapper::REAL));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::REAL, "REAL"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("DOUBLE", TypeWrapper::DOUBLE));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::DOUBLE, "DOUBLE"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("DOUBLE_PRECISION", TypeWrapper::DOUBLE_PRECISION));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::DOUBLE_PRECISION, "DOUBLE_PRECISION"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("FLOAT", TypeWrapper::FLOAT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::FLOAT, "FLOAT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("DECIMAL", TypeWrapper::DECIMAL));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::DECIMAL, "DECIMAL"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("DEC", TypeWrapper::DEC));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::DEC, "DEC"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("NUMERIC", TypeWrapper::NUMERIC));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::NUMERIC, "NUMERIC"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("DATE", TypeWrapper::DATE));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::DATE, "DATE"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("TIME", TypeWrapper::TIME));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::TIME, "TIME"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("TIMESTAMP", TypeWrapper::TIMESTAMP));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::TIMESTAMP, "TIMESTAMP"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("DATETIME", TypeWrapper::DATETIME));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::DATETIME, "DATETIME"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("YEAR", TypeWrapper::YEAR));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::YEAR, "YEAR"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("CHAR", TypeWrapper::CHAR));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::CHAR, "CHAR"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("VARCHAR", TypeWrapper::VARCHAR));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::VARCHAR, "VARCHAR"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("BINARY", TypeWrapper::BINARY));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::BINARY, "BINARY"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("VARBINARY", TypeWrapper::VARBINARY));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::VARBINARY, "VARBINARY"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("TINYBLOB", TypeWrapper::TINYBLOB));
    enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::TINYBLOB, "TINYBLOB"));
    stringToEnum.insert(pair<string, TypeWrapper::Type>("BLOB", TypeWrapper::BLOB));
    enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::BLOB, "BLOB"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("MEDIUMBLOB", TypeWrapper::MEDIUMBLOB));
    enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::MEDIUMBLOB, "MEDIUMBLOB"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("LONGBLOB", TypeWrapper::LONGBLOB));
    enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::LONGBLOB, "LONGBLOB"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("TINYTEXT", TypeWrapper::TINYTEXT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::TINYTEXT, "TINYTEXT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("TEXT", TypeWrapper::TEXT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::TEXT, "TEXT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("MEDIUMTEXT", TypeWrapper::MEDIUMTEXT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::MEDIUMTEXT, "MEDIUMTEXT"));
	stringToEnum.insert(pair<string, TypeWrapper::Type>("LONGTEXT", TypeWrapper::LONGTEXT));
	enumToString.insert(pair<TypeWrapper::Type, string>(TypeWrapper::LONGTEXT, "LONGTEXT"));
}   

lcs::lcs()
{
    initializeStringsAndEnumsMaps();
	Graph g;
	initializeGraph(g);

	set<TypeWrapper::Type> visitedTypes;
	g.topologicalSort(visitedTypes, orderStack);
	
	for(TypeWrapper::Type t = TypeWrapper::BIT; t < TypeWrapper::TYPECOUNT; t = TypeWrapper::Type(t + 1))
    {
        set<TypeWrapper::Type> visited;
        g.DFS(t, visited);
        typeToParents.insert(pair<TypeWrapper::Type, set<TypeWrapper::Type> >(t, visited));
    }

	// For each pair of types compute least common subtype
    for(TypeWrapper::Type t1 = TypeWrapper::BIT; t1 < TypeWrapper::TYPECOUNT; t1 = TypeWrapper::Type(t1 + 1))
        for(TypeWrapper::Type t2 = TypeWrapper::BIT; t2 < TypeWrapper::TYPECOUNT; t2 = TypeWrapper::Type(t2 + 1))
            lct[t1][t2] = generateLeastCommonSupertypes(t1, t2);
}

TypeWrapper::Type lcs::getLeastCommonSupertype(TypeWrapper::Type type1, TypeWrapper::Type type2)
{
	return lct[type1][type2];
}

TypeWrapper::Type lcs::generateLeastCommonSupertypes(TypeWrapper::Type type1, TypeWrapper::Type type2)
{
    if(type1 == type2)
		return type1;
    else 
    {
		for(unsigned int i = 0; i < orderStack.size(); ++i)
		{
			TypeWrapper::Type current = orderStack[i];
			
			if(typeToParents[type1].find(current) != typeToParents[type1].end() &&
			   typeToParents[type2].find(current) != typeToParents[type2].end())
				return current;
		}
	}
	return TypeWrapper::TYPECOUNT;
}

void Graph::addEdge(string from, string to)
{
    set<TypeWrapper::Type> adjList;
 
    // Check if we need to insert v into map adj
    adj.insert(pair<TypeWrapper::Type, set<TypeWrapper::Type> >(stringToEnum[from], adjList));

    adj[stringToEnum[from]].insert(stringToEnum[to]); // Add wint to vint’s list.
}

// DFS traversal of the vertices reachable from v
void Graph::DFS(TypeWrapper::Type v, set<TypeWrapper::Type> &visited)
{
    // Mark the current node as visited.
    visited.insert(v);
 
    // Recur for all the vertices adjacent to this vertex
    set<TypeWrapper::Type>::iterator i;
    for(i = adj[v].begin(); i != adj[v].end(); ++i)
        // Check if node is not visited
		if(visited.find(*i) == visited.end())
          	DFS(*i, visited);
}

// A recursive function used by topologicalSort
void Graph::topologicalSortUtil(TypeWrapper::Type v, set<TypeWrapper::Type> &visited, deque<TypeWrapper::Type> &orderStack)
{
    // Mark the current node as visited.
    visited.insert(v);
 
    // Recur for all the vertices adjacent to this vertex
    set<TypeWrapper::Type>::iterator i;
    for(i = adj[v].begin(); i != adj[v].end(); ++i)
        // Check if node is not visited
        if(visited.find(*i) == visited.end())
            topologicalSortUtil(*i, visited, orderStack);
 
    // Push current vertex to stack which stores result
    orderStack.push_front(v);
}
 
// The function to do Topological Sort. It uses recursive topologicalSortUtil()
void Graph::topologicalSort(set<TypeWrapper::Type> &visited, deque<TypeWrapper::Type> &orderStack)
{
    // Call the recursive helper function to store Topological Sort
    // starting from all vertices one by one
    for (TypeWrapper::Type t = TypeWrapper::BIT; t < TypeWrapper::TYPECOUNT; t = TypeWrapper::Type(t + 1))
	 	// Check if node is not visited
        if(visited.find(t) == visited.end())        
	    	topologicalSortUtil(t, visited, orderStack);
}

string getField(string str, int startIndex, string delimeter)
{
	return str.substr(startIndex, str.find(delimeter));
}

string getType(string str, string startDelimiter, size_t endingPos )
{
	return str.substr(str.find(startDelimiter) + 1, endingPos - str.find(startDelimiter) -1);
}

// Deletes m, d and UNSIGNED from the type string if they exist and sets m, d and unsigned to their corresponding values.
typeAndMD getTypeMDObjFromString(string typeString)
{
	size_t mPos, dPos, spacePos;
	int mInt, dInt;
	string m = "";
	string d = "";
	string type = "";
	bool typeUnsigned = false;
	
	// Contains multiple words so assume this is unsigned
	if((spacePos = typeString.find(" ")) != string::npos)
	{   
		typeUnsigned = true;
		typeString = typeString.substr(0, spacePos);
	}
	
	if((mPos = typeString.find("(")) != string::npos)
    {
        if((dPos = typeString.find(",")) != string::npos)
        {
            m = typeString.substr(mPos + 1, dPos - mPos);
            d = typeString.substr(dPos + 1, typeString.find(")") - dPos - 1);
        }
        else
        {
            m = typeString.substr(mPos + 1, typeString.find(")") - mPos - 1);
        }
    	
	  	type = typeString.substr(0, mPos);
    }
	else
	{
		type = typeString;	
	}

	// if m or d is not present set it to -1
	if(m.length() == 0)
		mInt = -1;
	else
		mInt = atoi(m.c_str());
	
	if(d.length() == 0)
		dInt = -1;
	else
		dInt = atoi(d.c_str());

	typeAndMD typeMD;
	typeMD.type = type;
	typeMD.m = mInt;
	typeMD.d = dInt;
	typeMD.unsignedVal = typeUnsigned;
	typeMD.typeEnum = stringToEnum[type];
	return typeMD;
}

void stringToUpper(string &s)
{
	for(unsigned int i = 0; i < s.length(); ++i)
		s[i] = toupper(s[i]);
}

void getNameAndTypeMD(string &schema, size_t endingPos, size_t delimiterLength, string &name, typeAndMD &typeMD)
{
	string typeString;

	name = getField(schema, 0, " ");
	//stringToUpper(name);
	
	typeString = getType(schema, " ", endingPos);	
	typeMD = getTypeMDObjFromString(typeString);
}

void addColToVector(vector<column> &existingSchemaCols, string &existingSchema, size_t existingPos, size_t delimiterLength)
{
	column col;
	string name;
	typeAndMD typeMD;

	getNameAndTypeMD(existingSchema, existingPos, delimiterLength, name, typeMD);
	col.existingName = name;
	col.typeMD = typeMD;

	existingSchemaCols.push_back(col);   	
	existingSchema.erase(0, existingPos + delimiterLength);
}
  
string toString(typeAndMD& type)
{
    stringstream ss;
    bool showD = false;
    switch(type.typeEnum) {
      case TypeWrapper::REAL: showD = true; break;
      case TypeWrapper::DOUBLE: showD = true; break;
      case TypeWrapper::DOUBLE_PRECISION: showD = true; break;
      case TypeWrapper::FLOAT: showD = true; break;
      case TypeWrapper::DECIMAL: showD = true; break;
      case TypeWrapper::DEC: showD = true; break;
      case TypeWrapper::NUMERIC: showD = true; break;
      default: showD = false; break;
    }
    ss << type.type;
    if(type.m >= 0 && type.d >= 0 && showD)
        ss << "(" << type.m << "," << type.d << ")";
    else if(type.m >= 0)
        ss << "(" << type.m << ")";
    
    if(type.unsignedVal)
        ss << " " << UNSIGNED_STRING;
    return ss.str();
}

void outputCol(string name, string type, int m, int d, bool typeUnsigned, bool &firstOutput)
{
	if(!firstOutput)
		cout << ", ";
	else 
		firstOutput = false;
	
	cout << name << " " << type;
	
	if(m >= 0 && d >= 0)
		cout << "(" << m << "," << d << ")";
	else if(m >= 0)
		cout << "(" << m << ")";
	
	if(typeUnsigned)
		cout << " " << UNSIGNED_STRING;
}

// Adds column to output schema vector
void addToOutputSchema(vector<column> &outputSchema, string existingColName, string insertColName, string type, int m, int d, bool typeUnsigned, bool changedFromExisting, bool addedFromExisting)
{
	column outputCol;
	typeAndMD typeMD;

	// Set m and d to -1 if they should not be used.                                                                                                                                     
    if(mTypes.find(type) == mTypes.end())                                                                                                                                         
    	m = -1;

	if(dTypes.find(type) == dTypes.end())                                                                                                                                         
     	d = -1;

	typeMD.type = type;
	typeMD.m = m;
	typeMD.d = d;
	typeMD.unsignedVal = typeUnsigned;
	
	outputCol.existingName = existingColName;
	outputCol.newName = insertColName;
	outputCol.typeMD = typeMD;
	outputCol.changedFromExisting = changedFromExisting;
	outputCol.addedFromExisting = addedFromExisting;


	outputSchema.push_back(outputCol);
}

void printOutputSchema(const vector<column> &outputSchema)
{
	for(unsigned int i = 0; i < outputSchema.size(); ++i)
	{
		column c = outputSchema[i];
		cout << "Column Names: " << c.existingName << "," << c.newName << " TypeWrapper::Type: " << c.typeMD.type << " M: " << c.typeMD.m 
			 << " D: " << c.typeMD.d << " Unsigned Bit: " << c.typeMD.unsignedVal << endl;
	}
}

int getMaxDigitsForType(typeAndMD typeMD, string outputType, bool outputUnsignedVal)
{
	string type = typeMD.type;
//	bool unsignedVal = typeMD.unsignedVal;
	
	if(type == "TINYINT")
	{
		return 3;
	}
	else if(type == "SMALLINT")
	{
		return 5;
	}
	else if(type == "MEDIUMINT")
	{
		// Need to check if output type is a d type and signed then take away 1 bit
		if(dTypes.find(outputType) != dTypes.end() && !outputUnsignedVal)
			return 7;
		else
			return 8;
	}
	else if(type == "INT" || type == "INTEGER")
	{
		return 10;
	}
	else if(type == "BIGINT")
	{
		// Need to check if output type is a d type and signed then take away 1 bit
		if(dTypes.find(outputType) != dTypes.end() && !outputUnsignedVal)
			return 19;
		else	
			return 20;
	}

	return -1;
}

int findNewMForTypeMD(typeAndMD typeMD, string outputType, bool outputUnsignedVal)
{
	int newM = typeMD.m;

	// if typeMD type is d supported type and the output type is not a d supported type
	if(dTypes.find(typeMD.type) != dTypes.end() && dTypes.find(outputType) == dTypes.end())
	{
		// Add one for decimal
		if(typeMD.unsignedVal)
		{
			newM += 1 ;
		}
		// Add two for decimal and negative sign
		else
		{
			newM += 2;
		}
	}
	// if existing type is an int type and output type is not an int type
	else if(intTypes.find(typeMD.type) != intTypes.end() && intTypes.find(outputType) == intTypes.end())
	{
		// Add the number of digits for the max possible number representable for this type
		newM = getMaxDigitsForType(typeMD, outputType, outputUnsignedVal);
	}

	return newM;
}

int findOutputM(typeAndMD existingTypeMD, typeAndMD insertTypeMD, string outputType, bool outputUnsignedVal)
{
	return max(findNewMForTypeMD(existingTypeMD, outputType, outputUnsignedVal), findNewMForTypeMD(insertTypeMD, outputType, outputUnsignedVal));
}

typeManager::typeManager() 
  : lcsO()
{
    initializeStringsAndEnumsMaps();

    regcomp(&INTEGER_REGEX, "^-{0,1}[0-9]+$", REG_EXTENDED | REG_NOSUB);
    regcomp(&DECIMAL_REGEX, "^-{0,1}[0-9]*\\.[0-9]+$", REG_EXTENDED | REG_NOSUB);
}

typeAndMD typeManager::leastCommonTypeAndMD(typeAndMD &type1, typeAndMD &type2) {
    if(type1.typeEnum<0)
        return type2;
    else if(type2.typeEnum<0)
        return type1;

    string lcsType;
    int type, finalM, finalD;
    bool typeUnsigned;

	type = lcsO.getLeastCommonSupertype(type1.typeEnum, type2.typeEnum);
 	lcsType = enumToString[static_cast<TypeWrapper::Type>(type)];

			// if both are unsigned then output unsigned
	typeUnsigned = type1.unsignedVal && type2.unsignedVal;
				
    finalM = findOutputM(type1, type2, lcsType, typeUnsigned);
    finalD = max(type1.d, type2.d);

    typeAndMD ret = {static_cast<TypeWrapper::Type>(type),lcsType,finalM, finalD, typeUnsigned};
    return ret;
}

inline ParsedType typeManager::match(const char* d, unsigned int length, int & dotPosition) {
  if(regexec(&INTEGER_REGEX, d, 0, NULL, 0)==0)
    return P_INTEGER;
  else if(regexec(&DECIMAL_REGEX, d, 0, NULL, 0)==0) {
    dotPosition = (int)(strchr(d,'.')-d);
    return P_DECIMAL;  
  } else {
    return P_ELSE;
  }
}

typeAndMD typeManager::inferType(char* value, unsigned int length) {
  if(length==0)
    return (typeAndMD){(TypeWrapper::Type) -1, "NULL",-1,-1,false};

  int position = 0;
  ParsedType t = match(value, length, position);

  switch(t) {
    case P_INTEGER:
    {
      // If definitely too big, just return varchar
      if(length>19)  
        return (typeAndMD){TypeWrapper::VARCHAR, "VARCHAR", length, -1, false};    

      if(value[0]=='-')
          length--;

      long long v = atoll(value);

      if(v<0) {
        if(v>=-128)   
          return (typeAndMD){TypeWrapper::TINYINT, "TINYINT", length, -1, false};
        else if(v>=-32768)   
          return (typeAndMD){TypeWrapper::SMALLINT, "SMALLINT", length, -1, false};
        else if(v>=-8388608)   
          return (typeAndMD){TypeWrapper::MEDIUMINT, "MEDIUMINT", length, -1, false};
        else   
          return (typeAndMD){TypeWrapper::INT, "INT", length, -1, false};
      } else {
        if(v<=255)
            return (typeAndMD){TypeWrapper::TINYINT, "TINYINT", length, -1, true};
        else if(v<=65535)
            return (typeAndMD){TypeWrapper::SMALLINT, "SMALLINT", length, -1, true};
        else if(v<=16777215)
            return (typeAndMD){TypeWrapper::MEDIUMINT, "MEDIUMINT", length, -1, true};
        else if(v<=2147483647)
            return (typeAndMD){TypeWrapper::INT, "INT", length, -1, true};
        else if(v<=18446744073709551615)
            return (typeAndMD){TypeWrapper::BIGINT, "BIGINT", length, -1, true};
        else
            return (typeAndMD){TypeWrapper::VARCHAR, "VARCHAR", length, -1, false};    
      }   
    }
    case P_DECIMAL:
    {
      int l = length-1;
      int scale = l - position;

      if(value[0]=='-')
        l--;
   
      return (typeAndMD){TypeWrapper::DECIMAL, "DECIMAL", l, scale, false};
    }   
    case P_ELSE:
      return (typeAndMD){TypeWrapper::VARCHAR, "VARCHAR", length, -1, false};    
  }

  return (typeAndMD){(TypeWrapper::Type)-1, NULL, -1, -1, false};    
}

bool changedFromExisting(typeAndMD existingTypeMD, typeAndMD resultTypeMD)
{
    bool changesFound = false;

    if(existingTypeMD.type != resultTypeMD.type)
        changesFound = true;
    else
    {
        if((existingTypeMD.m >= 0 && existingTypeMD.m < resultTypeMD.m)
           || (existingTypeMD.d >= 0 && existingTypeMD.d < resultTypeMD.d)
           || (existingTypeMD.unsignedVal != resultTypeMD.unsignedVal))
            changesFound = true;
    }

    return changesFound;
}

vector<column>::iterator foundInExistingSchema(vector<column> &existingSchemaCols, string insertName)
{
	vector<column>::iterator it;

	for(it = existingSchemaCols.begin(); it != existingSchemaCols.end(); ++it)
	{
		if(it->existingName == insertName)
			return it;
	}

	return it;
}

vector<column> typeManager::generateNewSchema(string existingSchema, string insertSchema)
{
	vector<column> outputSchema; 

	string delimiter = ", ";
	size_t existingPos = 0;
	size_t insertPos = 0;
	string token, field, existingType, insertType, m, d;
	//bool firstOutput = true;

	vector<column> existingSchemaCols;
	vector<column> insertSchemaCols;

	while((existingPos = existingSchema.find(delimiter)) != string::npos)
		addColToVector(existingSchemaCols, existingSchema, existingPos, delimiter.length());	
	
	// Add last col (no comma following)
	if(existingSchema.length() > 0)
		addColToVector(existingSchemaCols, existingSchema, string::npos, 0);

	while((insertPos = insertSchema.find(delimiter)) != string::npos)
        addColToVector(insertSchemaCols, insertSchema, insertPos, delimiter.length());
	
	// Add last col (no comma following)`
    if(insertSchema.length() > 0)
        addColToVector(insertSchemaCols, insertSchema, string::npos, 0);

	vector<column>::iterator it;
	for(unsigned int i = 0; i < insertSchemaCols.size(); ++i)
	{
		column curCol = insertSchemaCols[i];

		// Found a column name match from existingSchema in insertSchema
		if((it = foundInExistingSchema(existingSchemaCols, curCol.existingName)) != existingSchemaCols.end()) 
		{
			typeAndMD type = leastCommonTypeAndMD(it->typeMD, curCol.typeMD);
			
			bool modifiedType = changedFromExisting(it->typeMD, type);
			addToOutputSchema(outputSchema, it->existingName, curCol.existingName, type.type, type.m, type.d, type.unsignedVal, modifiedType, false);
		
			// Remove elt from insertSchemaCols
			existingSchemaCols.erase(it);
		}
		// Can't find column name from existingSchema in insertSchema
		else
		{
			addToOutputSchema(outputSchema, "", curCol.existingName, curCol.typeMD.type, curCol.typeMD.m, curCol.typeMD.d, curCol.typeMD.unsignedVal, false, true);
		}
	}

	// Add remaining cols from existingSchema that were not matched successfully to something in the insertSchema
	for(it = existingSchemaCols.begin(); it != existingSchemaCols.end(); ++it)
	{
		addToOutputSchema(outputSchema, it->existingName, "", it->typeMD.type, it->typeMD.m, it->typeMD.d, it->typeMD.unsignedVal, false, false);
	}
	
	return outputSchema;
}
/*
int main(int argc, char *argv[])
{
	if(argc < 3)
	{	
		cout << "Enter the two schemas as command line arguments" << endl;
		return -1;
	}

	initializeStringsAndEnumsMaps();
	
	string existingSchema = argv[1];
	string insertSchema = argv[2];

	vector<column> outputSchema;

	outputSchema = generateNewSchema(existingSchema, insertSchema);
	printOutputSchema(outputSchema);

	return 0;
}
*/
