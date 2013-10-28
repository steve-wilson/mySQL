#include <map>
#include <regex.h>

#define TYPECOUNT 8

enum Type {
  TNULL=0,
  NVARCHAR,
  BIT,
  SMALLINT,
  USMALLINT,
  INTEGER,
  DECIMAL,
  DATE
};

class TypeInstance {
  Type type;
  int precision;
  int scale;

  friend ostream& operator<<(ostream&, TypeInstance&);

  public:

  TypeInstance(Type t) {
    type = t;
    precision = -1;
    scale = -1;
  } 

  TypeInstance(Type t, int p) {
    type = t;
    precision = p;
    scale = -1;
  }

  TypeInstance(Type t, int p, int s) {
    type = t;
    precision = p;
    scale = s;
  }

  bool hasScale() {
    return scale!=-1;
  }

  bool hasPrecision() {
    return precision!=-1;
  }

  bool isUnsigned() {
    switch(type) {
      case USMALLINT:
        return true;
    }

    return false;
  }

  Type getType(){
    return type;
  }

  int getPrecision() {
    return precision;
  }

  int getScale() {
    return scale;
  }

  string toString() {
    switch(type) {
      case TNULL: return "NULL";
      case NVARCHAR: return "NVARCHAR";
      case BIT: return "BIT";
      case SMALLINT: return "SMALLINT";
      case USMALLINT: return "SMALLINT";
      case INTEGER: return "INTEGER";
      case DECIMAL: return "DECIMAL";
      case DATE: return "DATE";
    }

    return NULL;
  }
};

std::ostream & operator<<(std::ostream & Str, TypeInstance& v) { 
  Str << v.toString();
  if(v.hasPrecision()) {
    if(v.hasScale()) {
      Str << "(" << v.precision << "," << v.scale << ")";
    } else {
      Str << "(" << v.precision << ")";
    }
  }			
			
  Str << (v.isUnsigned()? " UNSIGNED" : "");
  return Str;
}

class TypeManager {
  std::map<Type,int> rank;
  std::map<Type,Type> parent;
  std::map<Type,vector<Type> > children;

  regex_t INTEGER_REGEX;
  regex_t DECIMAL_REGEX;

  Type lcs[TYPECOUNT][TYPECOUNT];

  Type rootType;

  Type lct(Type type1, Type type2) {
    if(rank[type1]==rank[type2])
      return type1;
		
    if(type1==TNULL)
    return type2;
    if(type2==TNULL)
      return type1;
		
    int min = std::min(rank[type1], rank[type2]);
    int max = std::max(rank[type1], rank[type2]);
    Type minType = rank[type1]==min? type1 : type2;
		
    Type cur = minType;
    while(cur!=0 && rank[cur]<max)
      cur = parent[cur];
		
    return cur;
  }

  int pot(Type root, int value) {
    for(unsigned int i=0; i<children[root].size(); i++)
      value = pot(children[root][i], value);
		
    rank[root] = ++value;

    return rank[root];
  }

  void setParent(Type s, Type t) {
    parent[s] = t;
    children[t].push_back(s);
  }

  int fudgeFactor(Type t) {
    switch(t) {
      case TNULL:
      case NVARCHAR: 
      case BIT:
      case DATE: return 0;
      case SMALLINT:
      case USMALLINT:
      case INTEGER: return 1;
      case DECIMAL: return 2;
    }

    return 0;
  } 

  public:

  TypeManager() {
    regcomp(&INTEGER_REGEX, "^-{0,1}[0-9]+$", REG_EXTENDED | REG_NOSUB);
    regcomp(&DECIMAL_REGEX, "^-{0,1}[0-9]*\\.[0-9]+$", REG_EXTENDED | REG_NOSUB);

    rank[NVARCHAR]=0;
    rank[BIT]=0;
    rank[SMALLINT]=0;
    rank[USMALLINT]=0;
    rank[INTEGER]=0;
    rank[DECIMAL]=0;
    rank[TNULL]=0;
    rank[DATE]=0;

    rootType = NVARCHAR;

    setParent(BIT, USMALLINT);
    setParent(USMALLINT, INTEGER);
    setParent(SMALLINT, INTEGER);
    setParent(INTEGER, DECIMAL);
    setParent(DECIMAL, rootType);
    setParent(DATE, rootType);

    pot(rootType, 0);

    for(int i=0; i<TYPECOUNT; i++) {
      for(int j=0; j<TYPECOUNT; j++) {
        lcs[i][j] = lct((Type)i,(Type)j);
      }
    }
  }
  
  bool match(regex_t* exp, string value) {
    return regexec(exp, value.c_str(), 0, NULL, 0)==0;
  }

  int parseInt(string value) {
    int i;
    sscanf(value.c_str(), "%d", &i);
    return i;
  }

 /**
  * Support Types: INTEGER, DECIMAL, STRING
  * 
  * @param value
  * @return
  */
  TypeInstance* inferType(string value) {
    if(value.empty())
      return new TypeInstance(TNULL);
	
    if(match(&INTEGER_REGEX, value)) {
      int length = value.length();
      if(value.c_str()[0]=='-')
        length--;
			
      int v = parseInt(value);
      if(v<0 && v>=-32768)
        return new TypeInstance(SMALLINT, length);
      else if(v<65535)
        return new TypeInstance(USMALLINT, length);
      else
        return new TypeInstance(INTEGER, length);
    } else if(match(&DECIMAL_REGEX, value)) {
      int length = value.length()-1;
      int scale = value.length() - value.find(".") - 1;
      if(value.c_str()[0]=='-')
        length--;

       return new TypeInstance(DECIMAL, length, scale);
    } else {
      return new TypeInstance(NVARCHAR, value.length());
    }
  }

  std::map<Type,int> getMap() {
    return rank;
  }
  
  inline Type getLeastCommonType(Type type1, Type type2) {
    return lcs[type1][type2];
  }

  TypeInstance* leastCommonSuperType(TypeInstance* original, TypeInstance* newType) {
    Type type = getLeastCommonType(original->getType(), newType->getType());

    // Make sure the precision of the new type fits all possible data
    int op = original->getPrecision();
    if(type!=original->getType())
      op += fudgeFactor(original->getType());
		
    int np = newType->getPrecision();
    if(type!=newType->getType())
      np += fudgeFactor(newType->getType());
		
    int precision = std::max(op, np);
    int scale = std::max(original->getScale(), newType->getScale());
		
    return new TypeInstance(type, precision, scale);
  }
};
