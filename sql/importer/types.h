#include <map>
#include <string>
#include "regex.h"

#define TYPECOUNT 13

enum Type {
  TNULL=0,
  NVARCHAR,
  BIT,
  TINYINT,
  UTINYINT,
  SMALLINT,
  USMALLINT,
  MEDIUMINT,
  UMEDIANINT,
  INTEGER,
  UINTEGER,
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

  TypeInstance(Type t, int p, short s) {
    type = t;
    precision = p;
    scale = s;
  }

  inline bool hasScale() {
    return scale!=-1;
  }

  inline bool hasPrecision() {
    return precision!=-1;
  }

  inline bool isUnsigned() {
    switch(type) {
      case UTINYINT:
      case USMALLINT:
      case UMEDIUMINT:
      case UINTEGER:
        return true;
    }

    return false;
  }

  inline Type getType(){
    return type;
  }

  inline int getPrecision() {
    return precision;
  }

  inline int getScale() {
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

  inline int fudgeFactor(Type t) {
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

    rootType = NVARCHAR;

    setParent(BIT, UTINYINT);
    setParent(UTINYINT, USMALLINT);
    setParent(USMALLINT, UINTEGER);
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
  
  inline bool matchInt(const char* d, unsigned int length) {
    for(unsigned int i=0; i<length; i++) {
      if(d[i]<'0' || d[i]>'9')
        return false;
    }

    return true;
  }

  inline bool matchDouble(const char* d, unsigned int length) {
    bool dot = false;
    for(unsigned int i=0; i<length; i++) {
      if(d[i]<'0' || d[i]>'9') {
        if(d[i]=='.' && !dot)
          dot = true;
        else
          return false;
      }
    }

    return dot;
  }

  int parseInt(const char* value) {
    int i;
    sscanf(value, "%d", &i);
    return i;
  }

 /**
  * Support Types: INTEGER, DECIMAL, STRING
  * 
  * @param value
  * @return
  */
  inline TypeInstance inferType(string* value) {
    unsigned int length = value->length();
    const char* d = value->c_str();
    if(length==0)
      return TypeInstance(TNULL);

    if(matchInt(d, length)) {
      if(d[0]=='-')
        length--;

      int v = atoi(d);
      if(v<0 && v>=-32768)
        return TypeInstance(SMALLINT, length);
      else if(v<65535)
        return TypeInstance(USMALLINT, length);
      else
        return TypeInstance(INTEGER, length);
    } else if(matchDouble(d, length)) {
      int l = length-1;
      int scale = l - value->find(".") - 1;
      if(d[0]=='-')
        length--;

       return TypeInstance(DECIMAL, l, scale);
    } else {
      return TypeInstance(NVARCHAR, length);
   }

    return TypeInstance(TNULL);
  }

  std::map<Type,int> getMap() {
    return rank;
  }
  
  inline Type getLeastCommonType(Type type1, Type type2) {
    return lcs[type1][type2];
  }

  inline TypeInstance leastCommonSuperType(TypeInstance original, TypeInstance newType) {
    Type type = getLeastCommonType(original.getType(), newType.getType());

    // Make sure the precision of the new type fits all possible data
    int op = original.getPrecision();
    if(type!=original.getType())
      op += fudgeFactor(original.getType());
		
    int np = newType.getPrecision();
    if(type!=newType.getType())
      np += fudgeFactor(newType.getType());
		
    int precision = std::max(op, np);
    int scale = std::max(original.getScale(), newType.getScale());
		
    return TypeInstance(type, precision, scale);
  }
};
