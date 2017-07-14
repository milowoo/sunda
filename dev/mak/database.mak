
DBINCP = .
#DBLIBP = $(ORACLE_HOME)/lib32
DBLIBP = $(ORACLE_HOME)/lib
#DBLIBS = -lclntsh -lm `cat $(ORACLE_HOME)/lib/sysliblist`
DBD    = _DB_ORA
DBDEF  = -DORA9 -D$(DBD)
DBFIX  = .pc
DBTYPE = ora
