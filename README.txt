
-------------
Usage of tool
-------------
Load Data Command:

LOAD DATA [LOW_PRIORITY | CONCURRENT] [LOCAL] INFILE 'file_name'
    [REPLACE | IGNORE]
    INTO TABLE tbl_name
    [SCHEMA_MERGE 
        [STRICT|RELAXED]
        [ALTER|DUMMY|VIEW|AUTO]
        [INFER_SAMPLE_SIZE size]
    ]
     [PARTITION (partition_name,...)]
    [CHARACTER SET charset_name]
    [{FIELDS | COLUMNS}
        [TERMINATED BY 'string']
        [[OPTIONALLY] ENCLOSED BY 'char']
        [ESCAPED BY 'char']
    ]
    [LINES
        [STARTING BY 'string']
        [TERMINATED BY 'string']
    ]
    [IGNORE number {LINES | ROWS}]
    [(col_name_or_user_var,...)]
    [SET col_name = expr,...]

Finalize Command:

FINALIZE_SCHEMA tbl_name

--------
Examples
--------

The following tests demonstrate how to use our system and shows some essential features of our project. The commands assume that the data files are located in some directory, $DATADIR, and can be reached at ‘$DATADIR/FILENAME’.

NOTE: Some example data sets, including "adults" can be found at mySQL/datasets/

// This shows how to use our code for the DUMMY METHOD

LOAD DATA INFILE '$DATADIR/adult_0.csv' INTO TABLE adult_dummy SCHEMA_MERGE DUMMY;

show tables;
// As you can see this creates an underlying table notated with underscores which contains 
// dummy columns and a view which includes everything except the dummy columns.

DESCRIBE ___adult_dummy___1;
// This shows that the underlying table added dummy columns

DESCRIBE adult_dummy;
// The view however does not have these dummy columns

// Now were going to load another dataset that adds additional columns and modifies the 
// already existing salary column
LOAD DATA INFILE '$DATADIR/adult_1.csv' INTO TABLE adult_dummy SCHEMA_MERGE DUMMY;

DESCRIBE ___adult_dummy___1;
// The underlying table renamed these columns to support the changes from importing the  // data. Note the salary field was modified from an integer to a decimal type and now has 
// two columns for it where one is modified.

DESCRIBE adult_dummy;
// The view combines the modified and salary columns into a single salary column

// Importing another time with multiple changes will cause the underlying table to be 
// expanded
LOAD DATA INFILE '$DATADIR/adult_2.csv' INTO TABLE adult_dummy SCHEMA_MERGE DUMMY;

DESCRIBE ___adult_dummy___1;
// The table has now been expanded removing the modified column for the salary and 
// adding additional dummy columns again

DROP TABLE adult_dummy;

DROP VIEW adult_dummy;

FINALIZE_SCHEMA adult_dummy;
// Materializes the view and drops the underlying table with dummy columns

show tables;
select * from adult_dummy;
DROP TABLE adult_dummy;

// The following statements use and show results from the View method

LOAD DATA INFILE '$DATADIR/adult_0.csv' 
    INTO TABLE adult SCHEMA_MERGE VIEW;
LOAD DATA INFILE '$DATADIR/adult_1.csv' 
    INTO TABLE adult SCHEMA_MERGE VIEW;
LOAD DATA INFILE '$DATADIR/adult_2.csv' 
    INTO TABLE adult SCHEMA_MERGE VIEW;
show tables;
DESCRIBE adult;
select
    (select count(*) from ___adult___1) as subtable1, 
    (select count(*) from ___adult___2) as subtable2,
    (select count(*) from ___adult___3) as subtable3,
    (select count(*) from adult) as total;

FINALIZE_SCHEMA adult;
show tables;
select * from adult;
DROP TABLE adult;

------------
Project Code
------------

mySQL/sql/sql_yacc.cc:
mySQL/sql/lex.h:
Parser modifications are done in these files. This allows us to use our custom SQL statements: SCHEMA_MERGE with all corresponding parameters and also the FINALIZE_SCHEMA command. We modify data structures to ensure control flow into our functions as necessary.

mySQL/sql/sql_load.cc:
The parser calls the mysql_load function defined here.  Our parser changes set flags that are passed through the sql_exchange object.  We modified the mysql_load function to call our functions when the schema_merge flag is included.  We also extracted the READ_INFO class (and changed it to READER) that had been defined here and moved it to reader.h.  

mySQL/sql/reader.h:
mySQL/sql/reader.cc:
This class was also extended to allow setting checkpoints while reading to jump back to.  This is explained more in our paper.

mySQL/sql/adapt_schema.h
mySQL/sql/adapt_schema.cc
This class is called from the mysql_load function in order to determine what changes need to be made to the tables to accommodate the schema.  From here we call prepareNaive, prepareViews, or prepareDummy.

mySQL/sql/loadcsv.h
mySQL/sql/loadcsv.cc:
The file reads through a csv file and calculates a schema for the data in the file.

mySQL/sql/computeNextSchema.h
mySQL/sql/computeNextSchema.cc:
Here we implement the methods to parse mysql types from strings and also implement the code to compute the least common supertype.

mySQL/sql/prepare_naive.cc
mySQL/sql/prepare_view.cc
mySQL/sql/prepare_dummy.cc:
These are where we implement each method

mySQL/sql/subtables.h
mySQL/sql/subtables.c
We define helper methods for creating subtables that the views build from.

mySQL/sql/sql_finalize.h
mySQL/sql/sql_finalize.cc:
The finalize_schema <tablename> command is defined here.

mySQL/sql/sql_prepare.h
mySQL/sql/sql_prepare.cc:
We extracted an internal method from sql_prepare.cc and moved it to sql_prepare.h in order to allow us to run sql commands from the server.

