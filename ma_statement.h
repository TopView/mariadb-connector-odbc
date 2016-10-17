/************************************************************************************
   Copyright (C) 2013 SkySQL AB
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc., 
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/
#ifndef _ma_statement_h_
#define _ma_statement_h_

#define MADB_FETCH_TYPE_FETCH       1
#define MADB_FETCH_TYPE_BULK        2
#define MADB_FETCH_TYPE_SETPOS      3


struct st_ma_stmt_methods
{
  SQLRETURN (*Prepare)(MADB_Stmt *Stmt, char *StatementText, SQLINTEGER TextLength);
  SQLRETURN (*Execute)(MADB_Stmt *Stmt);
  SQLRETURN (*Fetch)(MADB_Stmt *Stmt, my_bool KeepPosition);
  SQLRETURN (*BindColumn)(MADB_Stmt *Stmt, SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
                     SQLPOINTER TargetValuePtr, SQLLEN BufferLength, SQLLEN *StrLen_or_Ind);
  SQLRETURN (*BindParam)(MADB_Stmt *Stmt,  SQLUSMALLINT ParameterNumber, SQLSMALLINT InputOutputType, SQLSMALLINT ValueType,
                        SQLSMALLINT ParameterType, SQLULEN ColumnSize, SQLSMALLINT DecimalDigits, SQLPOINTER ParameterValuePtr,
                        SQLLEN BufferLength, SQLLEN *StrLen_or_IndPtr);
  SQLRETURN (*ExecDirect)(MADB_Stmt *Stmt, char *StatementText, SQLINTEGER TextLength);
  SQLRETURN (*GetData)(SQLHSTMT StatementHandle, SQLUSMALLINT Col_or_Param_Num, SQLSMALLINT TargetType,
                       SQLPOINTER TargetValuePtr, SQLLEN BufferLength, SQLLEN * StrLen_or_IndPtr);
  SQLRETURN (*RowCount)(MADB_Stmt *Stmt, SQLLEN *RowCountPtr);
  SQLRETURN (*ParamCount)(MADB_Stmt *Stmt, SQLSMALLINT *ParamCountPtr);
  SQLRETURN (*ColumnCount)(MADB_Stmt *Stmt, SQLSMALLINT *ColumnCountPtr);
  SQLRETURN (*GetAttr)(MADB_Stmt *Stmt, SQLINTEGER Attribute, SQLPOINTER Value, SQLINTEGER BufferLength,
                       SQLINTEGER *StringLength);
  SQLRETURN (*SetAttr)(MADB_Stmt *Stmt, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER StringLength);
  SQLRETURN (*StmtFree)(MADB_Stmt *Stmt, SQLUSMALLINT Option);
  SQLRETURN (*ColAttribute)(MADB_Stmt *Stmt, SQLUSMALLINT ColumnNumber, SQLUSMALLINT FieldIdentifier, SQLPOINTER CharacterAttributePtr,
             SQLSMALLINT BufferLength, SQLSMALLINT *StringLengthPtr, SQLLEN *NumericAttributePtr,
             my_bool isWchar);
  SQLRETURN (*ColumnPrivileges)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                                char *SchemaName, SQLSMALLINT NameLength2, char *TableName,
                                SQLSMALLINT NameLength3, char *ColumnName, SQLSMALLINT NameLength4);
  SQLRETURN (*TablePrivileges)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                               char *SchemaName, SQLSMALLINT NameLength2,
                               char *TableName, SQLSMALLINT NameLength3);
  SQLRETURN (*Tables)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                      char *SchemaName, SQLSMALLINT NameLength2, char *TableName,
                      SQLSMALLINT NameLength3, char *TableType, SQLSMALLINT NameLength4);
  SQLRETURN (*Statistics)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                          char *SchemaName, SQLSMALLINT NameLength2,
                          char *TableName, SQLSMALLINT NameLength3,
                          SQLUSMALLINT Unique, SQLUSMALLINT Reserved);
  SQLRETURN (*Columns)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                       char *SchemaName, SQLSMALLINT NameLength2,
                       char *TableName, SQLSMALLINT NameLength3,
                       char *ColumnName, SQLSMALLINT NameLength4);
  SQLRETURN (*ProcedureColumns)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                                char *SchemaName, SQLSMALLINT NameLength2, char *ProcName,
                                SQLSMALLINT NameLength3, char *ColumnName, SQLSMALLINT NameLength4);
  SQLRETURN (*PrimaryKeys)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                               char *SchemaName, SQLSMALLINT NameLength2, char *TableName,
                               SQLSMALLINT NameLength3);
  SQLRETURN (*SpecialColumns)(MADB_Stmt *Stmt, SQLUSMALLINT IdentifierType,
                              char *CatalogName, SQLSMALLINT NameLength1, 
                              char *SchemaName, SQLSMALLINT NameLength2,
                              char *TableName, SQLSMALLINT NameLength3,
                              SQLUSMALLINT Scope, SQLUSMALLINT Nullable);
  SQLRETURN (*Procedures)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                          char *SchemaName, SQLSMALLINT NameLength2, char *ProcName,
                          SQLSMALLINT NameLength3);
  SQLRETURN (*ForeignKeys)(MADB_Stmt *Stmt, char *PKCatalogName, SQLSMALLINT NameLength1,
                           char *PKSchemaName, SQLSMALLINT NameLength2, char *PKTableName,
                           SQLSMALLINT NameLength3, char *FKCatalogName, SQLSMALLINT NameLength4,
                           char *FKSchemaName, SQLSMALLINT NameLength5,  char *FKTableName,
                           SQLSMALLINT NameLength6);
  SQLRETURN (*DescribeCol)(MADB_Stmt *Stmt, SQLUSMALLINT ColumnNumber, void *ColumnName,
                         SQLSMALLINT BufferLength, SQLSMALLINT *NameLengthPtr,
                         SQLSMALLINT *DataTypePtr, SQLULEN *ColumnSizePtr, SQLSMALLINT *DecimalDigitsPtr,
                         SQLSMALLINT *NullablePtr, my_bool isWChar);
  SQLRETURN (*SetCursorName)(MADB_Stmt *Stmt, char *Buffer, SQLINTEGER BufferLength);
  SQLRETURN (*GetCursorName)(MADB_Stmt *Stmt, void *CursorName, SQLSMALLINT BufferLength, 
                             SQLSMALLINT *NameLengthPtr, my_bool isWChar);
  SQLRETURN (*SetPos)(MADB_Stmt *Stmt, SQLSETPOSIROW Row, SQLUSMALLINT Operation,
                      SQLUSMALLINT LockType, int ArrayOffset);
  SQLRETURN (*FetchScroll)(MADB_Stmt *Stmt, SQLSMALLINT FetchOrientation,
                           SQLLEN FetchOffset);
  SQLRETURN (*ParamData)(MADB_Stmt *Stmt, SQLPOINTER *ValuePtrPtr);
  SQLRETURN (*PutData)(MADB_Stmt *Stmt, SQLPOINTER DataPtr, SQLLEN StrLen_or_Ind);
  SQLRETURN (*BulkOperations)(MADB_Stmt *Stmt, SQLSMALLINT Operation);
  SQLRETURN (*RefreshDynamicCursor)(MADB_Stmt *Stmt);
  SQLRETURN (*RefreshRowPtrs)(MADB_Stmt *Stmt);
  SQLRETURN (*GetOutParams)(MADB_Stmt *Stmt, int CurrentOffset);
};

SQLRETURN    MADB_StmtInit          (MADB_Dbc *Connection, SQLHANDLE *pHStmt);
SQLUSMALLINT MapColAttributeDescType(SQLUSMALLINT FieldIdentifier);
MYSQL_RES*   FetchMetadata          (MADB_Stmt *Stmt);

#define MADB_MAX_CURSOR_NAME 64 * 3 + 1
#define MADB_CHECK_STMT_HANDLE(a,b)\
  if (!(a) || !(a)->b)\
    return SQL_INVALID_HANDLE

#define MADB_STMT_COLUMN_COUNT(aStmt) aStmt->Ird->Header.Count
#define MADB_STMT_PARAM_COUNT(aStmt)  aStmt->ParamCount

#define MADB_TRANSFER_OCTET_LENGTH\
  "CAST(CASE @dt"\
  "  WHEN " XSTR(SQL_BIT) " THEN 1 "\
  "  WHEN " XSTR(SQL_TINYINT) " THEN 1 "\
  "  WHEN " XSTR(SQL_SMALLINT) " THEN 2 "\
  "  WHEN " XSTR(SQL_INTEGER) " THEN IF(DATA_TYPE='mediumint',3,4) "\
  "  WHEN " XSTR(SQL_BIGINT) " THEN 20 "\
  "  WHEN " XSTR(SQL_REAL) " THEN 4 "\
  "  WHEN " XSTR(SQL_FLOAT) " THEN 8 "\
  "  WHEN " XSTR(SQL_DOUBLE) " THEN 8 "\
  "  WHEN " XSTR(SQL_TYPE_DATE) " THEN 6 "\
  "  WHEN " XSTR(SQL_TYPE_TIME) " THEN 6 "\
  "  WHEN " XSTR(SQL_TYPE_TIMESTAMP) " THEN 16 "\
  "  WHEN " XSTR(SQL_GUID) " THEN 16 "\
  "  WHEN " XSTR(SQL_DECIMAL) " THEN @ColSize + 2 "\
  "  WHEN " XSTR(SQL_BINARY) " THEN IF(DATA_TYPE='bit', CAST(((NUMERIC_PRECISION + 7) / 8) AS SIGNED), CHARACTER_OCTET_LENGTH) "\
  "  ELSE CHARACTER_OCTET_LENGTH "\
  "END  AS SIGNED)"


/* CASE for DATA_TYPE glued in 2 parts for ODBC v2 or v3 */
#define MADB_SQL_DATATYPEp1\
  "CASE DATA_TYPE"\
  "  WHEN 'bit' THEN @dt:= IF(NUMERIC_PRECISION=1," XSTR(SQL_BIT) ", " XSTR(SQL_BINARY) ")"\
  "  WHEN 'tinyint' THEN @dt:=" XSTR(SQL_TINYINT)\
  "  WHEN 'smallint' THEN @dt:=" XSTR(SQL_SMALLINT)\
  "  WHEN 'year' THEN "  XSTR(SQL_SMALLINT)\
  "  WHEN 'mediumint' THEN @dt:=" XSTR(SQL_INTEGER)\
  "  WHEN 'int' THEN @dt:=" XSTR(SQL_INTEGER)\
  "  WHEN 'bigint' THEN @dt:=" XSTR(SQL_BIGINT)\
  "  WHEN 'blob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'tinyblob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'mediumblob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'longblob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'text' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'tinytext' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'mediumtext' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'longtext' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'decimal' THEN @dt:=" XSTR(SQL_DECIMAL)\
  "  WHEN 'float' THEN @dt:=" XSTR(SQL_REAL)\
  "  WHEN 'double' THEN @dt:=" XSTR(SQL_DOUBLE)\
  "  WHEN 'binary' THEN @dt:=" XSTR(SQL_BINARY)\
  "  WHEN 'varbinary' THEN @dt:=" XSTR(SQL_VARBINARY)\
  "  WHEN 'char' THEN @dt:=" XSTR(SQL_CHAR)\
  "  WHEN 'enum' THEN @dt:=" XSTR(SQL_CHAR)\
  "  WHEN 'set' THEN @dt:=" XSTR(SQL_CHAR)\
  "  WHEN 'varchar' THEN @dt:=" XSTR(SQL_VARCHAR)
#define MADB_SQL_DATATYPEp2_ODBC3\
  "  WHEN 'date' THEN @dt:=" XSTR(SQL_TYPE_DATE)\
  "  WHEN 'time' THEN @dt:=" XSTR(SQL_TYPE_TIME)\
  "  WHEN 'datetime' THEN @dt:=" XSTR(SQL_TYPE_TIMESTAMP)\
  "  WHEN 'timestamp' THEN @dt:=" XSTR(SQL_TYPE_TIMESTAMP)\
  "  ELSE @dt:=" XSTR(SQL_LONGVARBINARY)\
  "END AS DATA_TYPE"
#define MADB_SQL_DATATYPEp2_ODBC2\
  "  WHEN 'date' THEN @dt:=" XSTR(SQL_DATE)\
  "  WHEN 'time' THEN @dt:=" XSTR(SQL_TIME)\
  "  WHEN 'datetime' THEN @dt:=" XSTR(SQL_TIMESTAMP)\
  "  WHEN 'timestamp' THEN @dt:=" XSTR(SQL_TIMESTAMP)\
  "  ELSE @dt:=" XSTR(SQL_LONGVARBINARY)\
  "END AS DATA_TYPE"

#define MADB_SQL_DATATYPE_ODBC3 MADB_SQL_DATATYPEp1 MADB_SQL_DATATYPEp2_ODBC3
#define MADB_SQL_DATATYPE_ODBC2 MADB_SQL_DATATYPEp1 MADB_SQL_DATATYPEp2_ODBC2

#define MADB_SQL_DATATYPE(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ? MADB_SQL_DATATYPE_ODBC3 : MADB_SQL_DATATYPE_ODBC2)

/************** End of DATA_TYPE *************/

/************** SQLColumns       *************/
#define MADB_COLUMN_SIZE\
    "CASE" \
    "  WHEN DATA_TYPE = 'bit' THEN @ColSize:=CAST(((NUMERIC_PRECISION + 7) / 8) AS SIGNED) "\
    "  WHEN DATA_TYPE in ('tinyint', 'smallint', 'year', 'mediumint', 'int',"\
                         "'bigint', 'decimal', 'double') THEN @ColSize:=NUMERIC_PRECISION "\
    "  WHEN DATA_TYPE = 'float' THEN @ColSize:=7"\
    "  WHEN DATA_TYPE='date' THEN @ColSize:=10"\
    "  WHEN DATA_TYPE='time' THEN @ColSize:=8"\
    "  WHEN DATA_TYPE in ('timestamp', 'datetime') THEN @ColSize:=19 "\
    "  ELSE @ColSize:=CHARACTER_MAXIMUM_LENGTH "\
  "END "

#define MADB_CATALOG_COLUMNSp1 "SELECT TABLE_SCHEMA AS TABLE_CAT, NULL AS TABLE_SCHEM, TABLE_NAME, COLUMN_NAME, "
#define MADB_CATALOG_COLUMNSp3  ", UCASE(IF(COLUMN_TYPE LIKE '%%(%%)%%',  CONCAT(SUBSTRING(COLUMN_TYPE,1, LOCATE('(',COLUMN_TYPE) - 1 ), SUBSTRING(COLUMN_TYPE,1+locate(')',COLUMN_TYPE))), COLUMN_TYPE )) AS TYPE_NAME, "\
  MADB_COLUMN_SIZE " AS COLUMN_SIZE,"\
  MADB_TRANSFER_OCTET_LENGTH " AS BUFFER_LENGTH, "\
  "NUMERIC_SCALE DECIMAL_DIGITS, IF(CHARACTER_OCTET_LENGTH IS NOT NULL, NULL, 10) AS NUM_PREC_RADIX,"\
  "IF(DATA_TYPE='timestamp', 1, IF(IS_NULLABLE='YES',1,IF(EXTRA='auto_increment', 1, 0))) AS NULLABLE, "\
  "COLUMN_COMMENT AS REMARKS,"\
  "IF(COLLATION_NAME IS NOT NULL AND COLUMN_DEFAULT IS NOT NULL, CONCAT(CHAR(39), COLUMN_DEFAULT, CHAR(39)), COLUMN_DEFAULT) AS COLUMN_DEF,"\
  "CASE"\
  "  WHEN DATA_TYPE = 'date' THEN " XSTR(SQL_DATETIME)\
  "  WHEN DATA_TYPE = 'time' THEN " XSTR(SQL_DATETIME)\
  "  WHEN DATA_TYPE = 'datetime' THEN " XSTR(SQL_DATETIME)\
  "  WHEN DATA_TYPE = 'timestamp' THEN " XSTR(SQL_DATETIME)\
  " ELSE @dt "\
  "END AS SQL_DATA_TYPE,"\
  "CASE"\
  "  WHEN DATA_TYPE = 'date' THEN " XSTR(SQL_TYPE_DATE)\
  "  WHEN DATA_TYPE = 'time' THEN " XSTR(SQL_TYPE_TIME)\
  "  WHEN DATA_TYPE = 'datetime' THEN " XSTR(SQL_TYPE_TIMESTAMP)\
  "  WHEN DATA_TYPE = 'timestamp' THEN " XSTR(SQL_TYPE_TIMESTAMP)\
  " ELSE NULL "\
  "END AS SQL_DATETIME_SUB,"\
  "IF(CHARACTER_OCTET_LENGTH IS NOT NULL, CHARACTER_OCTET_LENGTH, IF(DATA_TYPE='bit' AND NUMERIC_PRECISION =1, NULL, CAST((NUMERIC_PRECISION + 7)/8 AS SIGNED))) AS CHAR_OCTET_LENGTH, "\
  "ORDINAL_POSITION,"\
  "IF(DATA_TYPE='timestamp', 'YES', IF(IS_NULLABLE='YES','YES',IF(EXTRA='auto_increment', 'YES', 'NO'))) AS IS_NULLABLE FROM INFORMATION_SCHEMA.COLUMNS WHERE "

#define MADB_CATALOG_COLUMNS_ODBC3 MADB_CATALOG_COLUMNSp1 MADB_SQL_DATATYPE_ODBC3 MADB_CATALOG_COLUMNSp3
#define MADB_CATALOG_COLUMNS_ODBC2 MADB_CATALOG_COLUMNSp1 MADB_SQL_DATATYPE_ODBC2 MADB_CATALOG_COLUMNSp3
#define MADB_CATALOG_COLUMNS(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
                                        MADB_CATALOG_COLUMNS_ODBC3 : MADB_CATALOG_COLUMNS_ODBC2)
/************** End of SQLColumns ************/

/************** SQLProcedureColumns **********/
#define MADB_PROCEDURE_COLUMNSp1 \
  "SELECT SPECIFIC_SCHEMA AS PROCEDURE_CAT, NULL AS PROCEDURE_SCHEM, "\
  "SPECIFIC_NAME PROCEDURE_NAME, PARAMETER_NAME COLUMN_NAME, "\
  "CASE PARAMETER_MODE "\
  "  WHEN 'IN' THEN " XSTR(SQL_PARAM_INPUT)\
  "  WHEN 'OUT' THEN " XSTR(SQL_PARAM_OUTPUT)\
  "  WHEN 'INOUT' THEN " XSTR(SQL_PARAM_INPUT_OUTPUT)\
  "  ELSE IF(PARAMETER_MODE IS NULL, " XSTR(SQL_RETURN_VALUE) ", " XSTR(SQL_PARAM_TYPE_UNKNOWN) ")"\
  "END COLUMN_TYPE, "
#define MADB_PROCEDURE_COLUMNSp3\
  ", DATA_TYPE TYPE_NAME, "\
  MADB_COLUMN_SIZE " AS COLUMN_SIZE, "\
  MADB_TRANSFER_OCTET_LENGTH " AS BUFFER_LENGTH, "\
  "NUMERIC_SCALE DECIMAL_DIGITS, 10 NUM_PREC_RADIX,"\
  XSTR(SQL_NULLABLE_UNKNOWN) " NULLABLE,"\
  "NULL REMARKS, NULL COLUMN_DEF,"\
  "CASE"\
  "  WHEN DATA_TYPE = 'date' THEN " XSTR(SQL_DATE)\
  "  WHEN DATA_TYPE = 'time' THEN " XSTR(SQL_TIME)\
  "  WHEN DATA_TYPE = 'datetime' THEN " XSTR(SQL_DATETIME)\
  "  WHEN DATA_TYPE = 'timestamp' THEN " XSTR(SQL_TIMESTAMP)\
  " ELSE @dt "\
  "END AS SQL_DATA_TYPE, NULL SQL_DATETIME_SUB,"\
  "CHARACTER_OCTET_LENGTH CHAR_OCTET_LENGTH, "\
  "ORDINAL_POSITION, 'YES' IS_NULLABLE FROM INFORMATION_SCHEMA.PARAMETERS "

#define MADB_PROCEDURE_COLUMNS_ODBC3 MADB_PROCEDURE_COLUMNSp1 MADB_SQL_DATATYPE_ODBC3 MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS_ODBC2 MADB_PROCEDURE_COLUMNSp1 MADB_SQL_DATATYPE_ODBC2 MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
                                        MADB_PROCEDURE_COLUMNS_ODBC3 : MADB_PROCEDURE_COLUMNS_ODBC2)
/************** SQLProcedureColumns **********/

#endif
