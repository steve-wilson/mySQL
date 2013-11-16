#include <iostream>
#include <string>
#include <set>
#include <map>
#include <deque>
#include <sstream>
#include <vector>
#include <locale>

using namespace std;

#define UNSIGNED_STRING "UNSIGNED"

enum Type {
  	BIT = 0,
 	TINYINT,
  	SMALLINT,
	MEDIUMINT,
	INT,
	INTEGER,
  	BIGINT,
	REAL,
	DOUBLE,
	DOUBLE_PRECISION,
	FLOAT,
	DECIMAL,
	DEC,
	NUMERIC,
  	DATE,
	TIME,
	TIMESTAMP,
	DATETIME,
	YEAR,
	CHAR,
	VARCHAR,
	BINARY,
	VARBINARY,
	TINYBLOB,
	BLOB,
	MEDIUMBLOB,
	LONGBLOB,
	TINYTEXT,
	TEXT,
	MEDIUMTEXT,
	LONGTEXT,
	TYPECOUNT
};


string mTypesArray[] = { "BIT", "TINYINT", "SMALLINT", "MEDIUMINT", "INT", "INTEGER", "BIGINT", "CHAR", "VARCHAR",
	"BINARY", "VARBINARY", "REAL", "DOUBLE", "FLOAT", "DEC", "DECIMAL", "NUMERIC" };
string dTypesArray[] = { "REAL", "DOUBLE", "FLOAT", "DEC", "DECIMAL", "NUMERIC" };
string intTypesArray[] = { "TINYINT", "SMALLINT", "MEDIUMINT", "INT", "INTEGER", "BIGINT" };

set<string> mTypes(mTypesArray, mTypesArray + sizeof(mTypesArray) / sizeof(mTypesArray[0]) );
set<string> dTypes(dTypesArray, dTypesArray + sizeof(dTypesArray) / sizeof(dTypesArray[0]) );
set<string> intTypes(intTypesArray, intTypesArray + sizeof(intTypesArray) / sizeof(intTypesArray[0]) );
		
		

struct typeAndMD
{
	string type;
	int m;
	int d;
	bool unsignedVal;
};	

struct column
{
	string name;
	typeAndMD typeMD;
};

map<string, Type> stringToEnum;
map<Type, string> enumToString;

// Graph class represents a directed graph using adjacency list representation
class Graph
{
    map<Type, set<Type> > adj;    // Maps vertex to vector of adjacency list
    void topologicalSortUtil(Type v, set<Type> &visited, deque<Type> &orderStack);
public:
    void addEdge(string v, string w);   // function to add an edge to graph
    void DFS(Type v, set<Type> &visited);    // DFS traversal of the vertices reachable from v
    void topologicalSort(set<Type> &visited, deque<Type> &orderStack); 	// prints a Topological Sort of the complete graph	
};

// Least common supertype class computes matrix of least common supertypes
class lcs
{
    map<Type, set<Type> > typeToParents;	// A map from type to all its parent types
	deque<Type> orderStack;			// Topological ordering of graph
	Type lct[TYPECOUNT][TYPECOUNT];		// Matrix containing all least common type for all input type pairs
	void initializeGraph(Graph &g); 	// Adds all edges for type graph
    Type generateLeastCommonSupertypes(Type type1, Type type2); // Finds first type of parent based off topological ordering
public:
    lcs();
	Type getLeastCommonSupertype(Type type1, Type type2); 	// Returns lookup from lct matrix
	void printTypesAndParents();	// Prints all parent types of a type
	void printOrderStack();  	// Prints topological ordering
	void printlctMatrix();		// Prints all combinations least common type
};

void lcs::printlctMatrix()
{
	for(Type t1 = BIT; t1 < TYPECOUNT; t1 = Type(t1 + 1))
		for(Type t2 = BIT; t2 < TYPECOUNT; t2 = Type(t2 + 1))
		{
			Type type = getLeastCommonSupertype(t1, t2);
			if(type < TYPECOUNT)
				cout << "lct of " << enumToString[t1] << " and " << enumToString[t2] << " = " << enumToString[static_cast<Type>(type)] << endl;
		}
}

void lcs::printOrderStack()
{
	for(int i = 0; i < orderStack.size(); ++i)
	{
		cout << orderStack[i] << " ";
	}

	cout << endl;
}

void lcs::printTypesAndParents()
{
	map<Type, set<Type> >::iterator it;
    for(it = typeToParents.begin(); it != typeToParents.end(); ++it)
    {
        cout << it->first;
        set<Type>::iterator it2;
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

lcs::lcs()
{
	Graph g;
	initializeGraph(g);

	set<Type> visitedTypes;
	g.topologicalSort(visitedTypes, orderStack);
	
	for(Type t = BIT; t < TYPECOUNT; t = Type(t + 1))
    {
        set<Type> visited;
        g.DFS(t, visited);
        typeToParents.insert(pair<Type, set<Type> >(t, visited));
    }

	// For each pair of types compute least common subtype
    for(Type t1 = BIT; t1 < TYPECOUNT; t1 = Type(t1 + 1))
        for(Type t2 = BIT; t2 < TYPECOUNT; t2 = Type(t2 + 1))
            lct[t1][t2] = generateLeastCommonSupertypes(t1, t2);
}

Type lcs::getLeastCommonSupertype(Type type1, Type type2)
{
	return lct[type1][type2];
}

Type lcs::generateLeastCommonSupertypes(Type type1, Type type2)
{
    if(type1 == type2)
		return type1;
    else 
    {
		for(int i = 0; i < orderStack.size(); ++i)
		{
			Type current = orderStack[i];
			
			if(typeToParents[type1].find(current) != typeToParents[type1].end() &&
			   typeToParents[type2].find(current) != typeToParents[type2].end())
				return current;
		}
	}
	return TYPECOUNT;
}

void Graph::addEdge(string from, string to)
{
    set<Type> adjList;
 
    // Check if we need to insert v into map adj
    adj.insert(pair<Type, set<Type> >(stringToEnum[from], adjList));

    adj[stringToEnum[from]].insert(stringToEnum[to]); // Add wint to vint’s list.
}

// DFS traversal of the vertices reachable from v
void Graph::DFS(Type v, set<Type> &visited)
{
    // Mark the current node as visited.
    visited.insert(v);
 
    // Recur for all the vertices adjacent to this vertex
    set<Type>::iterator i;
    for(i = adj[v].begin(); i != adj[v].end(); ++i)
        // Check if node is not visited
		if(visited.find(*i) == visited.end())
          	DFS(*i, visited);
}

// A recursive function used by topologicalSort
void Graph::topologicalSortUtil(Type v, set<Type> &visited, deque<Type> &orderStack)
{
    // Mark the current node as visited.
    visited.insert(v);
 
    // Recur for all the vertices adjacent to this vertex
    set<Type>::iterator i;
    for(i = adj[v].begin(); i != adj[v].end(); ++i)
        // Check if node is not visited
        if(visited.find(*i) == visited.end())
            topologicalSortUtil(*i, visited, orderStack);
 
    // Push current vertex to stack which stores result
    orderStack.push_front(v);
}
 
// The function to do Topological Sort. It uses recursive topologicalSortUtil()
void Graph::topologicalSort(set<Type> &visited, deque<Type> &orderStack)
{
    // Call the recursive helper function to store Topological Sort
    // starting from all vertices one by one
    for (Type t = BIT; t < TYPECOUNT; t = Type(t + 1))
	 	// Check if node is not visited
        if(visited.find(t) == visited.end())        
	    	topologicalSortUtil(t, visited, orderStack);
}

void initializeStringsAndEnumsMaps()
{
	stringToEnum.insert(pair<string, Type>("BIT", BIT));
	enumToString.insert(pair<Type, string>(BIT, "BIT"));
	stringToEnum.insert(pair<string, Type>("TINYINT", TINYINT));
	enumToString.insert(pair<Type, string>(TINYINT, "TINYINT"));
	stringToEnum.insert(pair<string, Type>("SMALLINT", SMALLINT));
	enumToString.insert(pair<Type, string>(SMALLINT, "SMALLINT"));
	stringToEnum.insert(pair<string, Type>("MEDIUMINT", MEDIUMINT));
	enumToString.insert(pair<Type, string>(MEDIUMINT, "MEDIUMINT"));
	stringToEnum.insert(pair<string, Type>("INT", INT));
    enumToString.insert(pair<Type, string>(INT, "INT"));
	stringToEnum.insert(pair<string, Type>("INTEGER", INTEGER));
	enumToString.insert(pair<Type, string>(INTEGER, "INTEGER"));
	stringToEnum.insert(pair<string, Type>("BIGINT", BIGINT));
	enumToString.insert(pair<Type, string>(BIGINT, "BIGINT"));
	stringToEnum.insert(pair<string, Type>("REAL", REAL));
	enumToString.insert(pair<Type, string>(REAL, "REAL"));
	stringToEnum.insert(pair<string, Type>("DOUBLE", DOUBLE));
	enumToString.insert(pair<Type, string>(DOUBLE, "DOUBLE"));
	stringToEnum.insert(pair<string, Type>("DOUBLE_PRECISION", DOUBLE_PRECISION));
	enumToString.insert(pair<Type, string>(DOUBLE_PRECISION, "DOUBLE_PRECISION"));
	stringToEnum.insert(pair<string, Type>("FLOAT", FLOAT));
	enumToString.insert(pair<Type, string>(FLOAT, "FLOAT"));
	stringToEnum.insert(pair<string, Type>("DECIMAL", DECIMAL));
	enumToString.insert(pair<Type, string>(DECIMAL, "DECIMAL"));
	stringToEnum.insert(pair<string, Type>("DEC", DEC));
	enumToString.insert(pair<Type, string>(DEC, "DEC"));
	stringToEnum.insert(pair<string, Type>("NUMERIC", NUMERIC));
	enumToString.insert(pair<Type, string>(NUMERIC, "NUMERIC"));
	stringToEnum.insert(pair<string, Type>("DATE", DATE));
	enumToString.insert(pair<Type, string>(DATE, "DATE"));
	stringToEnum.insert(pair<string, Type>("TIME", TIME));
	enumToString.insert(pair<Type, string>(TIME, "TIME"));
	stringToEnum.insert(pair<string, Type>("TIMESTAMP", TIMESTAMP));
	enumToString.insert(pair<Type, string>(TIMESTAMP, "TIMESTAMP"));
	stringToEnum.insert(pair<string, Type>("DATETIME", DATETIME));
	enumToString.insert(pair<Type, string>(DATETIME, "DATETIME"));
	stringToEnum.insert(pair<string, Type>("YEAR", YEAR));
	enumToString.insert(pair<Type, string>(YEAR, "YEAR"));
	stringToEnum.insert(pair<string, Type>("CHAR", CHAR));
	enumToString.insert(pair<Type, string>(CHAR, "CHAR"));
	stringToEnum.insert(pair<string, Type>("VARCHAR", VARCHAR));
	enumToString.insert(pair<Type, string>(VARCHAR, "VARCHAR"));
	stringToEnum.insert(pair<string, Type>("BINARY", BINARY));
	enumToString.insert(pair<Type, string>(BINARY, "BINARY"));
	stringToEnum.insert(pair<string, Type>("VARBINARY", VARBINARY));
	enumToString.insert(pair<Type, string>(VARBINARY, "VARBINARY"));
	stringToEnum.insert(pair<string, Type>("TINYBLOB", TINYBLOB));
    enumToString.insert(pair<Type, string>(TINYBLOB, "TINYBLOB"));
    stringToEnum.insert(pair<string, Type>("BLOB", BLOB));
    enumToString.insert(pair<Type, string>(BLOB, "BLOB"));
	stringToEnum.insert(pair<string, Type>("MEDIUMBLOB", MEDIUMBLOB));
    enumToString.insert(pair<Type, string>(MEDIUMBLOB, "MEDIUMBLOB"));
	stringToEnum.insert(pair<string, Type>("LONGBLOB", LONGBLOB));
    enumToString.insert(pair<Type, string>(LONGBLOB, "LONGBLOB"));
	stringToEnum.insert(pair<string, Type>("TINYTEXT", TINYTEXT));
	enumToString.insert(pair<Type, string>(TINYTEXT, "TINYTEXT"));
	stringToEnum.insert(pair<string, Type>("TEXT", TEXT));
	enumToString.insert(pair<Type, string>(TEXT, "TEXT"));
	stringToEnum.insert(pair<string, Type>("MEDIUMTEXT", MEDIUMTEXT));
	enumToString.insert(pair<Type, string>(MEDIUMTEXT, "MEDIUMTEXT"));
	stringToEnum.insert(pair<string, Type>("LONGTEXT", LONGTEXT));
	enumToString.insert(pair<Type, string>(LONGTEXT, "LONGTEXT"));
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
	
	return typeMD;
}

void stringToUpper(string &s)
{
	for(int i = 0; i < s.length(); ++i)
		s[i] = toupper(s[i]);
}

void getNameAndTypeMD(string &schema, size_t endingPos, size_t delimiterLength, string &name, typeAndMD &typeMD)
{
	string typeString;

	name = getField(schema, 0, " ");
	stringToUpper(name);
	
	typeString = getType(schema, " ", endingPos);	
	typeMD = getTypeMDObjFromString(typeString);
}

void addColToVector(vector<column> &existingSchemaCols, string &existingSchema, size_t existingPos, size_t delimiterLength)
{
	column col;
	string name;
	typeAndMD typeMD;

	getNameAndTypeMD(existingSchema, existingPos, delimiterLength, name, typeMD);
	col.name = name;
	col.typeMD = typeMD;

	existingSchemaCols.push_back(col);   	
	existingSchema.erase(0, existingPos + delimiterLength);
}
  
void addColToMap(map<string, typeAndMD> &insertSchemaNameToType, string &insertSchema, size_t insertPos, size_t delimiterLength)
{
    string name;
   	typeAndMD typeMD;

    getNameAndTypeMD(insertSchema, insertPos, delimiterLength, name, typeMD);
   
	insertSchemaNameToType.insert(pair<string, typeAndMD>(name, typeMD));
    insertSchema.erase(0, insertPos + delimiterLength);
}

// Used for testing for printing the schema in string form
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
void addToOutputSchema(vector<column> &outputSchema, string existingColName, string insertColName, string type, int m, int d, bool typeUnsigned)
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

	outputCol.name = existingColName + ',' + insertColName;
	outputCol.typeMD = typeMD;

	outputSchema.push_back(outputCol);
}

void printOutputSchema(const vector<column> &outputSchema)
{
	for(int i = 0; i < outputSchema.size(); ++i)
	{
		column c = outputSchema[i];
		cout << "Column Names: " << c.name << " Type: " << c.typeMD.type << " M: " << c.typeMD.m 
			 << " D: " << c.typeMD.d << " Unsigned Bit: " << c.typeMD.unsignedVal << endl;
	}
}

int getMaxDigitsForType(typeAndMD typeMD, string outputType, bool outputUnsignedVal)
{
	string type = typeMD.type;
	bool unsignedVal = typeMD.unsignedVal;
	
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

vector<column> generateNewSchema(string existingSchema, string insertSchema)
{
	lcs lcs1;

    lcs1.printlctMatrix();

	vector<column> outputSchema; 

	string delimiter = ", ";
	size_t existingPos = 0;
	size_t insertPos = 0;
	string token, field, existingType, insertType, m, d;
	bool firstOutput = true;

	vector<column> existingSchemaCols;
	map<string, typeAndMD> insertSchemaNameToType;

	while((existingPos = existingSchema.find(delimiter)) != string::npos)
		addColToVector(existingSchemaCols, existingSchema, existingPos, delimiter.length());	
	
	// Add last col (no comma following)
	if(existingSchema.length() > 0)
		addColToVector(existingSchemaCols, existingSchema, string::npos, 0);

	while((insertPos = insertSchema.find(delimiter)) != string::npos)
        addColToMap(insertSchemaNameToType, insertSchema, insertPos, delimiter.length());
	
	// Add last col (no comma following)`
    if(insertSchema.length() > 0)
        addColToMap(insertSchemaNameToType, insertSchema, string::npos, 0);

	map<string, typeAndMD>::iterator it;
	for(int i = 0; i < existingSchemaCols.size(); ++i)
	{
		column curCol = existingSchemaCols[i];

		// Found a column name match from existingSchema in insertSchema
		if((it = insertSchemaNameToType.find(curCol.name)) != insertSchemaNameToType.end())
		{
			string lcsType;
			int type, finalM, finalD;
			bool typeUnsigned;
			
			type = lcs1.getLeastCommonSupertype(stringToEnum[curCol.typeMD.type], stringToEnum[it->second.type]);
 			lcsType = enumToString[static_cast<Type>(type)];

			// if both are unsigned then output unsigned
			typeUnsigned = curCol.typeMD.unsignedVal && it->second.unsignedVal;
			
			finalM = findOutputM(curCol.typeMD, it->second, lcsType, typeUnsigned);
			finalD = max(curCol.typeMD.d, it->second.d);
		
			addToOutputSchema(outputSchema, curCol.name, curCol.name, lcsType, finalM, finalD, typeUnsigned);

			// Remove col from insertSchema map.
			insertSchemaNameToType.erase(it);
		}
		// Can't find column name from existingSchema in insertSchema
		else
		{
			addToOutputSchema(outputSchema, curCol.name, "", curCol.typeMD.type, curCol.typeMD.m, curCol.typeMD.d, curCol.typeMD.unsignedVal);
		}
	}

	// Add remaining cols from insertSchema that were not matched successfully to something in the existingSchema
	for(it = insertSchemaNameToType.begin(); it != insertSchemaNameToType.end(); ++it)
	{
		addToOutputSchema(outputSchema, "", it->first, it->second.type, it->second.m, it->second.d, it->second.unsignedVal);
	}
	
	return outputSchema;
}

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
