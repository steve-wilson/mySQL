#ifndef COMPUTENEXTSCHEMA
#define COMPUTENEXTSCHEMA

#include <iostream>
#include <string>
#include <set>
#include <map>
#include <deque>
#include <sstream>
#include <vector>
#include <locale>
#include "regex.h"

#define UNSIGNED_STRING "UNSIGNED"

using namespace std;

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

struct typeAndMD
{
    Type typeEnum;
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

// Graph class represents a directed graph using adjacency list           representation 
class Graph
{
   map<Type, set<Type> > adj;    // Maps vertex to vector of adjacency list
  void topologicalSortUtil(Type v, set<Type> &visited, deque<Type>         
  &orderStack);
public:
  void addEdge(string v, string w);   // function to add an edge to graph
  void DFS(Type v, set<Type> &visited);    // DFS traversal of the             vertices reachable from v
  void topologicalSort(set<Type> &visited, deque<Type> &orderStack);  //       prints a Topological Sort of the complete graph    
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

enum ParsedType {P_INTEGER,P_DECIMAL,P_ELSE};

class typeManager
{
    lcs lcsO;
    regex_t INTEGER_REGEX;
    regex_t DECIMAL_REGEX;

    inline ParsedType match(const char* d, unsigned int length, int & dotPosition);
public:
    typeManager();

    typeAndMD inferType(char* value, unsigned int length);
    vector<column> generateNewSchema(string existingSchema, string insertSchema);
    typeAndMD leastCommonTypeAndMD(typeAndMD &type1, typeAndMD &type2);
};

string toString(typeAndMD& type);
#endif
