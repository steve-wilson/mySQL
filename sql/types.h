#ifndef TYPES_H
#define TYPES_H

#include <map>
#include <string>
#include "regex.h"
#include <iostream>
#include <vector>

#define TYPECOUNT 8

using namespace std;

enum Type {
  TNULL=0,
  VARCHAR,
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
      case USMALLINT:
        return true;
      default:
        return false;
    }
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
      case VARCHAR: return "NVARCHAR";
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
      case VARCHAR: 
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

    rootType = VARCHAR;

    setParent(BIT, USMALLINT);
    setParent(USMALLINT, INTEGER);
    setParent(SMALLINT, INTEGER);
    setParent(INTEGER, DECIMAL);
    setParent(DECIMAL, rootType);

    pot(rootType, 0);

    for(int i=0; i<TYPECOUNT; i++) {
      for(int j=0; j<TYPECOUNT; j++) {
        lcs[i][j] = lct((Type)i,(Type)j);
      }
    }
  }

  enum ParsedType {P_INTEGER,P_DECIMAL,P_ELSE};  

  inline ParsedType match(const char* d, unsigned int length, int & dotPosition) {
    if(regexec(&INTEGER_REGEX, d, 0, NULL, 0)==0)
      return P_INTEGER;
    else if(regexec(&DECIMAL_REGEX, d, 0, NULL, 0)==0) {
      dotPosition = (int)(strchr(d,'.')-d);
      return P_DECIMAL;
    } else {
      return P_ELSE;
    }
/*
    bool dot = false;

    if(d[0]<'0' || d[0]>'9') {
      if(d[0]=='.') {
        dot = true;
        dotPosition = 0;
      } else if(d[0]!='-') {
        return P_ELSE;
      }
    }

    for(unsigned int i=1; i<length; i++) {
      if(d[i]<'0' || d[i]>'9') {
        if(d[i]=='.' && !dot) {
          dot = true;
          dotPosition = i;
        } else
          return P_ELSE;
      }
    }

    return dot? P_DECIMAL : P_INTEGER;
*/
  }

 /**
  * Support Types: INTEGER, DECIMAL, STRING
  * 
  * @param value
  * @return
  */
  inline TypeInstance inferType(char* value, unsigned int length) {
    if(length==0)
      return TypeInstance(TNULL);

    int position = 0;
    ParsedType t = match(value, length, position);

    switch(t) {
      case P_INTEGER:
      {
        if(value[0]=='-')
          length--;

        int v = atoi(value);
        if(v<0 && v>=-32768)
          return TypeInstance(SMALLINT, length);
        else if(v<65535)
          return TypeInstance(USMALLINT, length);
        else
          return TypeInstance(INTEGER, length);
      }
      case P_DECIMAL:
      {
        int l = length-1;
        int scale = l - position;
        if(value[0]=='-')
          l--;

         return TypeInstance(DECIMAL, l, scale);
      }
      case P_ELSE:
        return TypeInstance(VARCHAR, length);
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

#endif
