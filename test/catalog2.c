/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2016 MariaDB Corporation AB

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "tap.h"

/* Columns order num for SQLColumns */
#define DATA_TYPE 5

/*
  Bug #37621 - SQLDescribeCol returns incorrect values of SQLTables data
*/
ODBC_TEST(t_bug37621)
{
  SQLCHAR szColName[128];
  SQLSMALLINT iName, iType, iScale, iNullable;
  SQLULEN uiDef;

  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug37621");
  OK_SIMPLE_STMT(Stmt, "create table t_bug37621 (x int)");
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, 0, NULL, 0,
			   (SQLCHAR *)"t_bug37621", SQL_NTS, NULL, 0));
/*
  Check column properties for the REMARKS column
*/
  CHECK_STMT_RC(Stmt, SQLDescribeCol(Stmt, 5, szColName, sizeof(szColName), 
          &iName, &iType, &uiDef, &iScale, &iNullable));

  IS_STR(szColName, "REMARKS", 8);
  is_num(iName, 7);
  if (iType != SQL_VARCHAR && iType != SQL_WVARCHAR)
    return FAIL;
  /* This can fail for the same reason as t_bug32864 */
  is_num(uiDef, 2048);
  is_num(iScale, 0);
  is_num(iNullable, 0);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug37621");

  return OK;
}


/*
Bug #34272 - SQLColumns returned wrong values for (some) TYPE_NAME
and (some) IS_NULLABLE
*/
ODBC_TEST(t_bug34272)
{
  SQLCHAR dummy[20];
  SQLULEN col6, col18, length;

  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug34272");
  OK_SIMPLE_STMT(Stmt, "create table t_bug34272 (x int unsigned)");

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
    (SQLCHAR *)"t_bug34272", SQL_NTS, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLDescribeCol(Stmt, 6, dummy, sizeof(dummy), NULL, NULL,
    &col6, NULL, NULL));
  CHECK_STMT_RC(Stmt, SQLDescribeCol(Stmt, 18, dummy, sizeof(dummy), NULL, NULL,
    &col18, NULL, NULL));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 6, SQL_C_CHAR, dummy, col6+1, &length));
  is_num(length,12);
  IS_STR(dummy, "INT UNSIGNED", length+1);

  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 18, SQL_C_CHAR, dummy, col18+1, &length));
  is_num(length,3);
  IS_STR(dummy, "YES", length+1);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt,SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug34272");

  return OK;
}


/*
  Bug #49660 - constraints with same name for tables with same name but in different db led
  to doubling of results of SQLForeignKeys
*/
ODBC_TEST(t_bug49660)
{
  SQLLEN rowsCount;
  
  OK_SIMPLE_STMT(Stmt, "drop database if exists bug49660");
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug49660");
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug49660_r");

  OK_SIMPLE_STMT(Stmt, "create database bug49660");
  OK_SIMPLE_STMT(Stmt, "create table bug49660.t_bug49660_r (id int unsigned not null primary key, name varchar(10) not null) ENGINE=InnoDB");
  OK_SIMPLE_STMT(Stmt, "create table bug49660.t_bug49660 (id int unsigned not null primary key, refid int unsigned not null,"
                "foreign key t_bug49660fk (id) references bug49660.t_bug49660_r (id)) ENGINE=InnoDB");

  OK_SIMPLE_STMT(Stmt, "create table t_bug49660_r (id int unsigned not null primary key, name varchar(10) not null) ENGINE=InnoDB");
  OK_SIMPLE_STMT(Stmt, "create table t_bug49660 (id int unsigned not null primary key, refid int unsigned not null,"
                "foreign key t_bug49660fk (id) references t_bug49660_r (id)) ENGINE=InnoDB");

  CHECK_STMT_RC(Stmt, SQLForeignKeys(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                                NULL, 0, (SQLCHAR *)"t_bug49660", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowsCount));
  is_num(rowsCount, 1); 
  /* Going another way around - sort of more reliable */
  FAIL_IF(SQLFetch(Stmt) == SQL_NO_DATA_FOUND, "expected data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt,SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "drop database if exists bug49660");
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug49660");
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug49660_r");

  return OK;
}


/*
  Bug #51422 - SQLForeignKeys returned keys pointing to unique fields
*/
ODBC_TEST(t_bug51422)
{
  SQLLEN rowsCount;
  
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug51422");
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug51422_r");

  OK_SIMPLE_STMT(Stmt, "create table t_bug51422_r (id int unsigned not null primary key, ukey int unsigned not null,"
                "name varchar(10) not null, UNIQUE KEY uk(ukey)) ENGINE=InnoDB");
  OK_SIMPLE_STMT(Stmt, "create table t_bug51422 (id int unsigned not null primary key, refid int unsigned not null,"
                "foreign key t_bug51422fk (id) references t_bug51422_r (ukey))  ENGINE=InnoDB");

  CHECK_STMT_RC(Stmt, SQLForeignKeys(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                                NULL, 0, (SQLCHAR *)"t_bug51422", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowsCount));
  is_num(rowsCount, 0);

  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug51422");
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug51422_r");

  return OK;
}


/*
  Bug #36441 - SQLPrimaryKeys returns mangled strings 
*/
ODBC_TEST(t_bug36441)
{
#define BUF_LEN 24

  const SQLCHAR key_column_name[][14]= {"pk_for_table1", "c1_for_table1"};

  SQLCHAR     catalog[BUF_LEN], schema[BUF_LEN], table[BUF_LEN], column[BUF_LEN];
  SQLLEN      catalog_len, schema_len, table_len, column_len;
  SQLCHAR     keyname[BUF_LEN];
  SQLSMALLINT key_seq, i;
  SQLLEN      keyname_len, key_seq_len, rowCount;

OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug36441_0123456789");
  OK_SIMPLE_STMT(Stmt, "create table t_bug36441_0123456789("
	              "pk_for_table1 integer not null auto_increment,"
	              "c1_for_table1 varchar(128) not null unique,"
	              "c2_for_table1 binary(32) null,"
                "unique_key int unsigned not null unique,"
	              "primary key(pk_for_table1, c1_for_table1))");

  CHECK_STMT_RC(Stmt, SQLPrimaryKeys(Stmt, NULL, SQL_NTS, NULL, SQL_NTS, "t_bug36441_0123456789", SQL_NTS));

  /* Test of SQLRowCount with SQLPrimaryKeys */
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));
  is_num(rowCount, 2);

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR , catalog, sizeof(catalog), &catalog_len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR , schema , sizeof(schema) , &schema_len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR , table  , sizeof(table)  , &table_len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR , column , sizeof(column) , &column_len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT,&key_seq, sizeof(key_seq), &key_seq_len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 6, SQL_C_CHAR , keyname, sizeof(keyname), &keyname_len));

  for(i= 0; i < 2; ++i)
  {
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    is_num(catalog_len, strlen(my_schema));
    is_num(schema_len, SQL_NULL_DATA);
    IS_STR(table, "t_bug36441_0123456789", 3);
    IS_STR(column, key_column_name[i], 4);
    is_num(key_seq, i+1);
    IS_STR(keyname, "PRIMARY", 6);
  }

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug36441_0123456789");

  return OK;

#undef BUF_LEN
}


/*
  Bug #53235 - SQLColumns returns wrong transfer octet length
*/
ODBC_TEST(t_bug53235)
{
  int col_size, buf_len;

  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug53235");
  OK_SIMPLE_STMT(Stmt, "create table t_bug53235 (x decimal(10,3))");
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
			   (SQLCHAR *)"t_bug53235", SQL_NTS, NULL, 0));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  col_size= my_fetch_int(Stmt, 7);
  buf_len= my_fetch_int(Stmt, 8);

  is_num(col_size, 10);
  is_num(buf_len, 12);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug53235");

  return OK;
}


/*
  Bug #50195 - SQLTablePrivileges requires select priveleges
*/
ODBC_TEST(t_bug50195)
{
  SQLHDBC     hdbc1;
  SQLHSTMT    hstmt1;
  const char  expected_privs[][12]= {"ALTER", "CREATE", "CREATE VIEW", "DELETE", "DROP", "INDEX",
                                    "INSERT", "REFERENCES", "SHOW VIEW", "TRIGGER", "UPDATE"};
  int         i;
  SQLCHAR     priv[12];
  SQLLEN      len;

  (void)SQLExecDirect(Stmt, (SQLCHAR *)"DROP USER bug50195@127.0.0.1", SQL_NTS);
  (void)SQLExecDirect(Stmt, (SQLCHAR *)"DROP USER bug50195@localhost", SQL_NTS);

  OK_SIMPLE_STMT(Stmt, "grant all on *.* to bug50195@127.0.0.1 IDENTIFIED BY 'a'");
  OK_SIMPLE_STMT(Stmt, "grant all on *.* to bug50195@localhost IDENTIFIED BY 'a'");

  OK_SIMPLE_STMT(Stmt, "revoke select on *.* from bug50195@127.0.0.1");
  OK_SIMPLE_STMT(Stmt, "revoke select on *.* from bug50195@localhost");

  /* revoking "global" select is enough, but revoking smth from mysql.tables_priv
     to have not empty result of SQLTablePrivileges */
  OK_SIMPLE_STMT(Stmt, "grant all on mysql.tables_priv to bug50195@127.0.0.1");
  OK_SIMPLE_STMT(Stmt, "grant all on mysql.tables_priv to bug50195@localhost");
  OK_SIMPLE_STMT(Stmt, "revoke select on mysql.tables_priv from bug50195@127.0.0.1");
  OK_SIMPLE_STMT(Stmt, "revoke select on mysql.tables_priv from bug50195@localhost");

  OK_SIMPLE_STMT(Stmt, "FLUSH PRIVILEGES");

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc1));

  CHECK_DBC_RC(hdbc1, SQLConnect(hdbc1, my_dsn, SQL_NTS, "bug50195", SQL_NTS, "a", SQL_NTS));

  CHECK_DBC_RC(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  CHECK_STMT_RC(hstmt1, SQLTablePrivileges(hstmt1, "mysql", SQL_NTS, 0, 0, "tables_priv", SQL_NTS));

  /* Testing SQLTablePrivileges a bit, as we don't have separate test of it */

  for(i= 0; i < 11; ++i)
  {
    CHECK_STMT_RC(hstmt1, SQLFetch(hstmt1));
    CHECK_STMT_RC(hstmt1, SQLGetData(hstmt1, 6, SQL_C_CHAR, priv, sizeof(priv), &len));
    IS_STR(priv, expected_privs[i], len);
  }
  
  FAIL_IF(SQLFetch(hstmt1) != 100, "No data expected");

  CHECK_STMT_RC(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  CHECK_DBC_RC(hdbc1, SQLDisconnect(hdbc1));
  CHECK_DBC_RC(hdbc1, SQLFreeConnect(hdbc1));

  OK_SIMPLE_STMT(Stmt, "DROP USER bug50195@127.0.0.1");
  OK_SIMPLE_STMT(Stmt, "DROP USER bug50195@localhost");
  
  return OK;
}


ODBC_TEST(t_sqlprocedurecolumns)
{
  SQLRETURN rc= 0;
  SQLCHAR szName[255]= {0};

  typedef struct 
  {
    char *c01_procedure_cat;
    char *c02_procedure_schem;
    char *c03_procedure_name;
    char *c04_column_name;
    short c05_column_type;
    short c06_data_type;
    char *c07_type_name;
    unsigned long c08_column_size;
    unsigned long c09_buffer_length;
    short c10_decimal_digits;
    short c11_num_prec_radix;
    short c12_nullable;
    char *c13_remarks;
    char *c14_column_def;
    short c15_sql_data_type;
    short c16_sql_datetime_sub;
    unsigned long c17_char_octet_length;
    int c18_ordinal_position;
    char *c19_is_nullable;
  }sqlproccol;

  int total_params= 0, iter= 0;

  sqlproccol data_to_check[] = {
    /*cat    schem  proc_name                  col_name     col_type         data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param1", SQL_PARAM_INPUT, SQL_TINYINT, "tinyint",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
     3,    1,       0,  10,    SQL_NULLABLE, "", 0,  SQL_TINYINT,  0,  0,    1,  "YES"},

    /*cat    schem  proc_name                  col_name     col_type          data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param2", SQL_PARAM_OUTPUT, SQL_SMALLINT, "smallint",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
     5,    2,       0,  10,    SQL_NULLABLE, "", 0,  SQL_SMALLINT,  0,  0,    2,  "YES"},

    /*cat    schem  proc_name                  col_name     col_type          data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param3", SQL_PARAM_INPUT,  SQL_INTEGER, "mediumint",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
     7,    3,       0,  10,    SQL_NULLABLE, "", 0,  SQL_INTEGER,  0,  0,    3,  "YES"},

    /*cat    schem  proc_name                  col_name      col_type                data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param 4", SQL_PARAM_INPUT_OUTPUT, SQL_INTEGER, "int",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
     10,     4,       0,  10,  SQL_NULLABLE, "", 0,  SQL_INTEGER,  0,  0,    4,  "YES"},

    /*cat    schem  proc_name                  col_name     col_type          data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param5", SQL_PARAM_OUTPUT, SQL_BIGINT, "bigint",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
     19,     20,      0,  10,  SQL_NULLABLE, "", 0,  SQL_BIGINT,   0,  0,    5,  "YES"},

    /*cat    schem  proc_name                  col_name     col_type         data_type  type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param6", SQL_PARAM_INPUT, SQL_REAL,  "float",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      7,   4,       2,  0,     SQL_NULLABLE, "", 0,  SQL_REAL,     0,  0,    6,  "YES"},

    /*cat    schem  proc_name                  col_name     col_type          data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param7", SQL_PARAM_OUTPUT, SQL_DOUBLE,  "double",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      15,  8,       0,  0,     SQL_NULLABLE, "", 0,  SQL_DOUBLE,   0,  0,    7,  "YES"},

    /*cat    schem  proc_name                  col_name     col_type          data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param8", SQL_PARAM_INPUT, SQL_DECIMAL,  "decimal",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      10,  11,       3,  10,    SQL_NULLABLE, "", 0,  SQL_DECIMAL,   0,  0,    8,  "YES"},

    /*cat    schem  proc_name                  col_name     col_type          data_type type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param9", SQL_PARAM_INPUT, SQL_CHAR,  "char",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      32,  32,      0,  0,     SQL_NULLABLE, "", 0,  SQL_CHAR,     0,  32,    9,  "YES"},

    /*cat    schem  proc_name                  col_name      col_type           data_type    type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param10", SQL_PARAM_OUTPUT, SQL_VARCHAR, "varchar",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      64,  64,      0,  0,     SQL_NULLABLE, "", 0,  SQL_VARCHAR,  0,  64,   10, "YES"},

    /*cat    schem  proc_name                  col_name      col_type         data_type          type_name */
    {my_schema, 0,     "procedure_columns_test1", "re_param11", SQL_PARAM_INPUT, SQL_LONGVARBINARY, "long varbinary",
    /*size      buf_len   dec radix  nullable      rem def sql_data_type       sub octet     pos nullable*/
      16777215, 16777215, 0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARBINARY,  0,  16777215, 12, "YES"},

    /*cat    schem  proc_name                  col_name     col_type         data_type          type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramA", SQL_PARAM_INPUT, SQL_LONGVARBINARY, "blob",
    /*size      buf_len   dec radix  nullable      rem def sql_data_type       sub octet     pos nullable*/
      65535,    65535,    0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARBINARY,  0,  65535,    1, "YES"},

    /*cat    schem  proc_name                  col_name     col_type         data_type          type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramB", SQL_PARAM_INPUT, SQL_LONGVARBINARY, "longblob",
    /*size        buf_len      dec radix  nullable      rem def sql_data_type       sub octet        pos nullable*/
     4294967295L, 2147483647L, 0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARBINARY,  0,  2147483647L, 2, "YES"},

    /*cat    schem  proc_name                  col_name     col_type               data_type          type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramC", SQL_PARAM_INPUT_OUTPUT, SQL_LONGVARBINARY, "tinyblob",
    /*size   buf_len dec radix  nullable      rem def sql_data_type       sub octet pos nullable*/
     255,    255,    0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARBINARY,  0,  255,  3, "YES"},

    /*cat    schem  proc_name                  col_name     col_type         data_type          type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramD", SQL_PARAM_INPUT, SQL_LONGVARBINARY, "mediumblob",
    /*size     buf_len   dec radix  nullable      rem def sql_data_type       sub octet     pos nullable*/
     16777215, 16777215, 0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARBINARY,  0,  16777215, 4, "YES"},

     /*cat    schem  proc_name                  col_name    col_type        data_type          type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramE", SQL_PARAM_INPUT, SQL_VARBINARY, "varbinary",
    /*size   buf_len dec radix  nullable      rem def sql_data_type   sub octet pos nullable*/
     128,    128,    0,  0,     SQL_NULLABLE, "", 0,  SQL_VARBINARY,  0,  128,  5, "YES"},

     /*cat    schem  proc_name                  col_name    col_type          data_type   type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramF", SQL_PARAM_OUTPUT, SQL_BINARY, "binary",
    /*size   buf_len dec radix  nullable      rem def sql_data_type   sub octet pos nullable*/
     1,      1,      0,  0,     SQL_NULLABLE, "", 0,  SQL_BINARY,     0,  1,    6, "YES"},

     /*cat    schem  proc_name                 col_name     col_type          data_type   type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramG", SQL_PARAM_INPUT,  SQL_BINARY, "binary",
    /*size   buf_len dec radix  nullable      rem def sql_data_type   sub octet pos nullable*/
     8,      8,      0,  0,     SQL_NULLABLE, "", 0,  SQL_BINARY,     0,  8,    7, "YES"},

    /*cat    schem  proc_name                  col_name      col_type          data_type          type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_param H", SQL_PARAM_INPUT, SQL_LONGVARCHAR, "long varchar",
    /*size     buf_len   dec radix  nullable      rem def sql_data_type       sub octet     pos nullable*/
     16777215, 16777215, 0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARCHAR,  0,  16777215, 8, "YES"},

    /*cat    schem  proc_name                  col_name      col_type         data_type       type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramI", SQL_PARAM_INPUT, SQL_LONGVARCHAR, "text",
    /*size      buf_len   dec radix  nullable      rem def sql_data_type    sub octet  pos nullable*/
      65535,    65535,    0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARCHAR, 0,  65535,  9, "YES"},

    /*cat    schem  proc_name                  col_name     col_type         data_type       type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramJ", SQL_PARAM_INPUT, SQL_LONGVARCHAR, "mediumtext",
    /*size     buf_len   dec radix  nullable      rem def sql_data_type    sub octet     pos nullable*/
     16777215, 16777215, 0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARCHAR, 0,  16777215, 10, "YES"},

     /*cat    schem  proc_name                  col_name    col_type              data_type        type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramK", SQL_PARAM_INPUT_OUTPUT, SQL_LONGVARCHAR, "longtext",
    /*size        buf_len      dec radix  nullable      rem def sql_data_type    sub octet        pos nullable*/
     4294967295L, 2147483647L, 0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARCHAR, 0,  2147483647L, 11, "YES"},

    /*cat    schem  proc_name                  col_name     col_type         data_type        type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramL", SQL_PARAM_INPUT, SQL_LONGVARCHAR, "tinytext",
    /*size   buf_len dec radix  nullable      rem def sql_data_type    sub octet pos nullable*/
     255,    255,    0,  0,     SQL_NULLABLE, "", 0,  SQL_LONGVARCHAR, 0,  255,  12, "YES"},

    /*cat    schem  proc_name                  col_name     col_type         data_type    type_name */
    {my_schema, 0,     "procedure_columns_test2", "re_paramM", SQL_PARAM_INPUT, SQL_NUMERIC,  "numeric",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      8,   10,      2,  10,    SQL_NULLABLE, "", 0,  SQL_NUMERIC,   0,  0,   13,  "YES"},

    /*cat    schem  proc_name                  col_name       col_type         data_type     type_name */
    {my_schema, 0,     "procedure_columns_test3", "re_param_00", SQL_PARAM_INPUT, SQL_TYPE_TIMESTAMP, "datetime",
    /*size buf_len  dec radix  nullable      rem def sql_data_type  sub                  octet pos nullable*/
      19,  16,      0,  10,     SQL_NULLABLE, "", 0, SQL_DATETIME,  SQL_TYPE_TIMESTAMP,  0,    1,  "YES"},

    /*cat    schem  proc_name                  col_name       col_type          data_type      type_name */
    {my_schema, 0,     "procedure_columns_test3", "re_param_01", SQL_PARAM_OUTPUT, SQL_TYPE_DATE, "date",
    /*size buf_len  dec radix  nullable      rem def sql_data_type  sub octet pos nullable*/
      10,  6,       0,  0,     SQL_NULLABLE, "", 0,  SQL_DATETIME,  SQL_TYPE_DATE,  0,    2,  "YES"},

    /*cat    schem  proc_name                  col_name       col_type          data_type      type_name */
    {my_schema, 0,     "procedure_columns_test3", "re_param_02", SQL_PARAM_OUTPUT, SQL_TYPE_TIME, "time",
    /*size buf_len  dec radix  nullable      rem def sql_data_type  sub octet pos nullable*/
      8,   6,       0,  0,     SQL_NULLABLE, "", 0,  SQL_DATETIME,  SQL_TYPE_TIME,  0,    3,  "YES"},

    /*cat    schem  proc_name                  col_name       col_type                data_type           type_name */
    {my_schema, 0,     "procedure_columns_test3", "re_param_03", SQL_PARAM_INPUT_OUTPUT, SQL_TYPE_TIMESTAMP, "timestamp",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub                  octet pos nullable*/
      19,  16,      0,  0,     SQL_NULLABLE, "", 0,  SQL_DATETIME, SQL_TYPE_TIMESTAMP,  0,    4,  "YES"},

    /*cat    schem  proc_name                  col_name       col_type         data_type     type_name */
    {my_schema, 0,     "procedure_columns_test3", "re_param_04", SQL_PARAM_INPUT, SQL_SMALLINT, "year",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      4,   1,       0,  10,    SQL_NULLABLE, "", 0,  SQL_SMALLINT, 0,  0,    5,  "YES"},

    /*cat    schem  proc_name                       col_name        col_type          data_type    type_name */
    {my_schema, 0,     "procedure_columns_test4_func", "RETURN_VALUE", SQL_RETURN_VALUE, SQL_VARCHAR, "varchar",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      32,  32,      0,  0,     SQL_NULLABLE, "", 0,  SQL_VARCHAR,  0,  32,   0, "YES"},

     /*cat    schem  proc_name                       col_name    col_type         data_type    type_name */
    {my_schema, 0,     "procedure_columns_test4_func", "re_paramF", SQL_PARAM_INPUT, SQL_INTEGER, "int",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
     10,     4,       0,  10,  SQL_NULLABLE, "", 0,  SQL_INTEGER,  0,  0,    1,  "YES"},

    /*cat    schem  proc_name                               col_name        col_type          data_type    type_name */
    {my_schema, 0,     "procedure_columns_test4_func_noparam", "RETURN_VALUE", SQL_RETURN_VALUE, SQL_VARCHAR, "varchar",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      32,  32,      0,  0,     SQL_NULLABLE, "", 0,  SQL_VARCHAR,  0,  32,   0, "YES"},

    /*cat    schem  proc_name                  col_name           col_type         data_type type_name */
    {my_schema, 0,     "procedure_columns_test5", "re_param_set_01", SQL_PARAM_INPUT, SQL_CHAR,  "char",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      14,  14,      0,  0,     SQL_NULLABLE, "", 0,  SQL_CHAR,     0,  14,    1,  "YES"},

    /*cat    schem  proc_name                  col_name            col_type          data_type type_name */
    {my_schema, 0,     "procedure_columns_test5", "re_param_enum_02", SQL_PARAM_OUTPUT, SQL_CHAR,  "char",
    /*size buf_len  dec radix  nullable      rem def sql_data_type sub octet pos nullable*/
      7,   7,       0,  0,     SQL_NULLABLE, "", 0,  SQL_CHAR,     0,  7,    2,  "YES"},
  };
  
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test1");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test2");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test2_noparam");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test3");
  OK_SIMPLE_STMT(Stmt, "drop function if exists procedure_columns_test4_func");
  OK_SIMPLE_STMT(Stmt, "drop function if exists procedure_columns_test4_func_noparam");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test5");

  OK_SIMPLE_STMT(Stmt, "create procedure procedure_columns_test1(IN re_param1 TINYINT, OUT re_param2 SMALLINT," \
                "re_param3 MEDIUMINT, INOUT `re_param 4` INT UNSIGNED, OUT re_param5 BIGINT, re_param6 FLOAT(4,2)," \
                "OUT re_param7 DOUBLE(5, 3), IN re_param8 DECIMAL(10,3) unSIGned, re_param9 CHAR(32)," \
                "Out re_param10 VARCHAR(64) charset utf8, ignore_param INT, re_param11 long VARBINARY)" \
                "begin end;"
                );
  OK_SIMPLE_STMT(Stmt, "create procedure procedure_columns_test2(IN re_paramA bloB," \
                "IN re_paramB LONGBLOB, inout re_paramC TinyBlob, re_paramD mediumblob, IN re_paramE varbinary(128)," \
                "OUT re_paramF binary, re_paramG binary(8), `re_param H` LONG VARCHAR, IN re_paramI TEXT," \
                "re_paramJ mediumtext, INOUT re_paramK longtext, re_paramL tinytext, re_paramM numeric(8,2))" \
                "begin end;"
                );
  OK_SIMPLE_STMT(Stmt, "create procedure procedure_columns_test2_noparam()"\
                "begin end;"
                );
  
  OK_SIMPLE_STMT(Stmt, "create procedure procedure_columns_test3(IN re_param_00 datetime,"\
                "OUT re_param_01 date, OUT re_param_02 time, INOUT re_param_03 timestamp,"\
                "re_param_04 year)" \
                "begin end;"
                );

  OK_SIMPLE_STMT(Stmt, "create function procedure_columns_test4_func(re_paramF int) returns varchar(32) deterministic "\
                "begin return CONCAT('abc', paramF); end;"
                );

  OK_SIMPLE_STMT(Stmt, "create function procedure_columns_test4_func_noparam() returns varchar(32) deterministic "\
                "begin return 'abc'; end;"
                );

  OK_SIMPLE_STMT(Stmt, "create procedure procedure_columns_test5(IN re_param_set_01 SET('', \"one\", 'two', 'three'),"\
                "OUT re_param_enum_02 ENUM('', \"one\", 'tw)o', 'three m'))" \
                "begin end;"
                );

  CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, NULL, 0, NULL, 0,
                                     "procedure_columns_test%", SQL_NTS, 
                                     "re_%", SQL_NTS));

  while(SQLFetch(Stmt) == SQL_SUCCESS)
  {
    SQLCHAR buff[255] = {0}, *param_cat, *param_name;
    SQLUINTEGER col_size, buf_len, octet_len;

    param_cat= (SQLCHAR*)my_fetch_str(Stmt, buff, 1);
    IS_STR(param_cat, data_to_check[iter].c01_procedure_cat, 
           strlen(data_to_check[iter].c01_procedure_cat) + 1);

    IS_STR(my_fetch_str(Stmt, buff, 3), 
           data_to_check[iter].c03_procedure_name, 
           strlen(data_to_check[iter].c03_procedure_name) + 1);

    param_name= (SQLCHAR*)my_fetch_str(Stmt, buff, 4);
    diag("%s.%s", param_cat, param_name);
    IS_STR(param_name, data_to_check[iter].c04_column_name, 
           strlen(data_to_check[iter].c04_column_name) + 1);

    is_num(my_fetch_int(Stmt, 5), data_to_check[iter].c05_column_type);

    is_num(my_fetch_int(Stmt, 6), data_to_check[iter].c06_data_type);

    IS_STR(my_fetch_str(Stmt, buff, 7), 
           data_to_check[iter].c07_type_name, 
           strlen(data_to_check[iter].c07_type_name) + 1);

    col_size= my_fetch_int(Stmt, 8);
    is_num(col_size, data_to_check[iter].c08_column_size);

    buf_len= my_fetch_int(Stmt, 9);
    is_num(buf_len, data_to_check[iter].c09_buffer_length);

    diag("Iter: %d", iter);
    is_num(my_fetch_int(Stmt, 10), data_to_check[iter].c10_decimal_digits);
    
    is_num(my_fetch_int(Stmt, 11), data_to_check[iter].c11_num_prec_radix);

    is_num(my_fetch_int(Stmt, 15), data_to_check[iter].c15_sql_data_type);

    is_num(my_fetch_int(Stmt, 16), data_to_check[iter].c16_sql_datetime_sub);

    octet_len= my_fetch_int(Stmt, 17);
    is_num(octet_len, data_to_check[iter].c17_char_octet_length);

    is_num(my_fetch_int(Stmt, 18), data_to_check[iter].c18_ordinal_position);

    IS_STR(my_fetch_str(Stmt, buff, 19), 
           data_to_check[iter].c19_is_nullable, 
           strlen(data_to_check[iter].c19_is_nullable + 1));

    ++iter;
  }

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test1");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test2");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test2_noparam");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test3");
  OK_SIMPLE_STMT(Stmt, "drop function if exists procedure_columns_test4_func");
  OK_SIMPLE_STMT(Stmt, "drop function if exists procedure_columns_test4_func_noparam");
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists procedure_columns_test5");
  

  return OK;
}


ODBC_TEST(t_bug57182)
{
  SQLLEN nRowCount;
  SQLCHAR buff[24];

  OK_SIMPLE_STMT(Stmt, "drop procedure if exists bug57182");
  OK_SIMPLE_STMT(Stmt, "CREATE DEFINER=`adb`@`%` PROCEDURE `bug57182`(in id int, in name varchar(20)) "
    "BEGIN"
    "  insert into simp values (id, name);"
    "END");

  CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, my_schema, SQL_NTS, NULL, 0,
    "bug57182", SQL_NTS, 
    NULL, 0));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &nRowCount));
  is_num(2, nRowCount);
  
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  
  IS_STR(my_fetch_str(Stmt, buff, 3), "bug57182", 9);
  IS_STR(my_fetch_str(Stmt, buff, 4), "id", 3);
  IS_STR(my_fetch_str(Stmt, buff, 7), "int", 4);

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  IS_STR(my_fetch_str(Stmt, buff, 3), "bug57182", 9);
  IS_STR(my_fetch_str(Stmt, buff, 4), "name", 5);
  IS_STR(my_fetch_str(Stmt, buff, 7), "varchar", 8);
  is_num(my_fetch_int(Stmt, 8), 20);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA, "No data expected");
  
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* Almost the same thing but with column specified */
  CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, my_schema, SQL_NTS, NULL, 0,
    "bug57182", SQL_NTS, 
    "id", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &nRowCount));
  is_num(1, nRowCount);

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  IS_STR(my_fetch_str(Stmt, buff, 3), "bug57182", 9);
  IS_STR(my_fetch_str(Stmt, buff, 4), "id", 3);
  IS_STR(my_fetch_str(Stmt, buff, 7), "int", 4);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA, "No data expected");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* And testing impossible column condition - expecting to get no rows */
  CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, my_schema, SQL_NTS, NULL, 0,
    "bug57182", SQL_NTS, 
    "non_existing_column%", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &nRowCount));
  is_num(0, nRowCount);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA, "No data expected");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "drop procedure if exists bug57182");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  return OK;
}


/* SQLRowCount() doesn't work with SQLTables and other functions
   Testing of that with SQLTables, SQLColumn is incorporated in other testcases
*/
ODBC_TEST(t_bug55870)
{
  SQLLEN  rowCount;
  SQLCHAR noI_SconnStr[512], query[256];
  SQLHDBC    hdbc1;
  SQLHSTMT   hstmt1;

  OK_SIMPLE_STMT(Stmt, "drop table if exists bug55870r");
  OK_SIMPLE_STMT(Stmt, "drop table if exists bug55870_2");
  OK_SIMPLE_STMT(Stmt, "drop table if exists bug55870");
  OK_SIMPLE_STMT(Stmt, "create table bug55870(a int not null primary key, "
    "b varchar(20) not null, c varchar(100) not null, INDEX(b)) ENGINE=InnoDB");

  /* There should be no problems with I_S version of SQLTablePrivileges. Thus need connection
     not using I_S. SQlStatistics doesn't have I_S version, but it ma change at certain point.
     Thus let's test it on NO_I_S connection too */
  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc1));

  sprintf((char *)noI_SconnStr, "DSN=%s;UID=%s;PWD=%s; NO_I_S=1", my_dsn, my_uid, my_pwd);

  sprintf(query, "grant Insert, Select on bug55870 to %s", my_uid);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, query, SQL_NTS));
  sprintf(query, "grant Insert (c), Select (c), Update (c) on bug55870 to %s", my_uid);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, query, SQL_NTS));

  CHECK_DBC_RC(hdbc1, SQLDriverConnect(hdbc1, NULL, noI_SconnStr, sizeof(noI_SconnStr), NULL,
                                0, NULL, SQL_DRIVER_NOPROMPT));

  CHECK_DBC_RC(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  CHECK_STMT_RC(hstmt1, SQLStatistics(hstmt1, NULL, 0, NULL, 0,
                                   "bug55870", SQL_NTS,
                                   SQL_INDEX_UNIQUE, SQL_QUICK));
  CHECK_STMT_RC(hstmt1, SQLRowCount(hstmt1, &rowCount));
  is_num(rowCount, 1);

  CHECK_STMT_RC(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  CHECK_STMT_RC(hstmt1, SQLTablePrivileges(hstmt1, my_schema, SQL_NTS, 0, 0, "bug55870",
                                    SQL_NTS));

  CHECK_STMT_RC(hstmt1, SQLRowCount(hstmt1, &rowCount));
  is_num(rowCount, my_print_non_format_result(hstmt1));

  CHECK_STMT_RC(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  CHECK_STMT_RC(hstmt1, SQLColumnPrivileges(hstmt1, my_schema, SQL_NTS, 0, 0, "bug55870",
                                      SQL_NTS, "c", SQL_NTS));

  CHECK_STMT_RC(hstmt1, SQLRowCount(hstmt1, &rowCount));
  is_num(rowCount, my_print_non_format_result(hstmt1));

  CHECK_STMT_RC(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "create table bug55870_2 (id int not null primary key, value "
                "varchar(255) not null) ENGINE=InnoDB");
  OK_SIMPLE_STMT(Stmt, "create table bug55870r (id int unsigned not null primary key,"
                "refid int not null, refid2 int not null,"
                "somevalue varchar(20) not null,  foreign key b55870fk1 (refid) "
                "references bug55870 (a), foreign key b55870fk2 (refid2) "
                "references bug55870_2 (id)) ENGINE=InnoDB");

  /* actually... looks like no-i_s version of SQLForeignKeys is broken on latest
     server versions. comment in "show table status..." contains nothing */
  CHECK_STMT_RC(hstmt1, SQLForeignKeys(hstmt1, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
    NULL, 0, (SQLCHAR *)"bug55870r", SQL_NTS));

  CHECK_STMT_RC(hstmt1, SQLRowCount(hstmt1, &rowCount));
  is_num(rowCount, my_print_non_format_result(hstmt1));

  /** surprise-surprise - just removing table is not enough to remove related
      records from tables_priv and columns_priv
  */
  sprintf(query, "revoke select,insert on bug55870 from %s", my_uid);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, query, SQL_NTS));

  sprintf(query, "revoke select (c),insert (c),update (c) on bug55870 from %s", my_uid);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, query, SQL_NTS));

  /*
  OK_SIMPLE_STMT(Stmt, "drop table if exists bug55870r");
    OK_SIMPLE_STMT(Stmt, "drop table if exists bug55870_2");
    OK_SIMPLE_STMT(Stmt, "drop table if exists bug55870");*/
  

  CHECK_STMT_RC(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  CHECK_DBC_RC(hdbc1, SQLDisconnect(hdbc1));
  CHECK_DBC_RC(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));

  return OK;
}


/**
 Bug#31067 test. Testing only part of the patch, as the report itself
 is about Access and can't be tested automatically. The test checks
 if SQLColumns returns correct default value if table's catalog specified.
*/
ODBC_TEST(t_bug31067)
{
  SQLCHAR    buff[512];

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS bug31067");
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE bug31067");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE bug31067.a (a varchar(10) not null default 'bug31067')");

  /* Get the info from just one table.  */
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, (SQLCHAR *)"bug31067", SQL_NTS, NULL, SQL_NTS,
                             (SQLCHAR *)"a", SQL_NTS, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  IS_STR(my_fetch_str(Stmt, buff, 13), "'bug31067'", 11);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE bug31067");

  return OK;
}


/* Some catalog functions can return only one row of result if previous prepare
   statement pre-execution has failed */
ODBC_TEST(bug12824839)
{
  SQLLEN      row_count;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS b12824839");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS b12824839a");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE b12824839 "
                "(id int primary key, vc_col varchar(32), int_col int)");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE b12824839a "
                "(id int, vc_col varchar(32) UNIQUE, int_col int,"
                "primary key(id,int_col))");

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, my_schema, SQL_NTS, NULL, 0, "b12824839",
                            SQL_NTS, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &row_count));
  is_num(3, row_count);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLPrimaryKeys(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
                                "b12824839a", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &row_count));
  is_num(2, row_count);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* SQLColumns was not completely fixed */
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, my_schema, SQL_NTS, NULL, 0, NULL,
                            SQL_NTS, NULL, 0));
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &row_count));

  /* There should be records at least for those 2 tables we've created */
  FAIL_IF(row_count < 6, "expected >= 6 rows");

  return OK;
}


/* If no default database is selected for the connection, call of SQLColumns
   will cause error "Unknown database 'null'" */
ODBC_TEST(sqlcolumns_nodbselected)
{
  SQLHDBC hdbc1;
  SQLHSTMT hstmt1;
  SQLCHAR conn_in[512];

  /* Just to make sure we have at least one table in our test db */
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS sqlcolumns_nodbselected");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE sqlcolumns_nodbselected (id int)");

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc1));

  /* Connecting not specifying default db */
  sprintf((char *)conn_in, "DRIVER=%s;SERVER=%s;UID=%s;PWD=%s;PORT=%d", my_drivername,
                              my_servername, my_uid, my_pwd, my_port);

  

  CHECK_DBC_RC(hdbc1, SQLDriverConnect(hdbc1, NULL, conn_in, sizeof(conn_in), NULL,
                                 0, NULL,
                                 SQL_DRIVER_NOPROMPT));



  CHECK_DBC_RC(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  CHECK_DBC_RC(hdbc1, SQLGetInfo(hdbc1, SQL_DATABASE_NAME,
            (SQLPOINTER) conn_in, sizeof(conn_in), NULL));
  IS_STR("null", conn_in, 5);

  CHECK_STMT_RC(hstmt1, SQLColumns(hstmt1, my_schema, SQL_NTS, NULL, 0, NULL,
                            0, NULL, 0));

  CHECK_DBC_RC(hdbc1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  CHECK_DBC_RC(hdbc1, SQLDisconnect(hdbc1));
  CHECK_DBC_RC(hdbc1, SQLFreeConnect(hdbc1));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS sqlcolumns_nodbselected");

  return OK;
}


/**
 Bug#14085211 test. LONG TABLE NAMES CRASH OBDC DRIVER
 We will try creating databases, tables and columns with the
 maximum allowed length of 64 symbols and also try to give
 the driver very long (>1024 symbols) names to make it crash.
*/
ODBC_TEST(t_bug14085211_part1)
{
  SQLCHAR  buff[8192];
  SQLCHAR  db_64_name[65]  = "database_64_symbols_long_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  SQLCHAR  tab_64_name[65] = "table____64_symbols_long_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  SQLCHAR  col_64_name[65] = "column___64_symbols_long_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  SQLCHAR  tab_1024_name[1025] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
                             "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  sprintf(buff, "DROP DATABASE IF EXISTS %s", db_64_name);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, buff, SQL_NTS));

  sprintf(buff, "CREATE DATABASE %s", db_64_name);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, buff, SQL_NTS));

  sprintf(buff, "CREATE TABLE %s.%s(%s varchar(10))", db_64_name, tab_64_name, col_64_name);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, buff, SQL_NTS));

  /* Lets check if SQLTables can get these long names  */
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, (SQLCHAR *)db_64_name, SQL_NTS, NULL, SQL_NTS,
                                  (SQLCHAR *)tab_64_name, SQL_NTS, 
                                  "BASE TABLE" /*,VIEW" */, SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  /* check the database name */
  IS_STR(my_fetch_str(Stmt, buff, 1), db_64_name, 64);

  /* check the table name */
  IS_STR(my_fetch_str(Stmt, buff, 3), tab_64_name, 64);
  
  /* only one db/table match, so nothing should be in the results */
  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* Lets check if SQLTables can ignore 1024-characters for table name */
  FAIL_IF(SQLTables(Stmt, (SQLCHAR *)tab_1024_name, SQL_NTS, NULL, SQL_NTS,
                                  (SQLCHAR *)tab_1024_name, SQL_NTS, 
                                  "TABLE,VIEW", SQL_NTS) !=  SQL_ERROR, "Error expected");
  is_num(check_sqlstate(Stmt, IS_ODBC3() ? "HY090" : "S1009"), OK);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  sprintf(buff, "DROP DATABASE IF EXISTS %s", db_64_name);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, buff, SQL_NTS));

  return OK;
}


ODBC_TEST(t_bug14085211_part2)
{
  /* 
    TODO: test all catalog functions for extreme lengths of
          database, table and column names     
  */
  return OK;
}


/* Bug#14338051 SQLCOLUMNS WORKS INCORRECTLY IF CALLED AFTER A STATEMENT
                RETURNING RESULT
   I expect that some other catalog function can be vulnerable, too */
ODBC_TEST(t_sqlcolumns_after_select)
{
  OK_SIMPLE_STMT(Stmt, "DROP TABLE if exists b14338051");

  OK_SIMPLE_STMT(Stmt,"CREATE TABLE b14338051("
               "blob_field BLOB, text_field TEXT )");

  OK_SIMPLE_STMT(Stmt, "insert into b14338051 "
                "set blob_field= 'blob', text_field= 'text'; ");

  OK_SIMPLE_STMT(Stmt, "SELECT 'blob', 'text'");


  while (SQL_SUCCEEDED(SQLFetch(Stmt)))
  {
  }

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
                          (SQLCHAR *)"b14338051",
                          (SQLSMALLINT)strlen("b14338051"), NULL, 0));

  is_num(myrowcount(Stmt), 2);

  return OK;
}

/* Bug #14555713 USING ADO, ODBC DRIVER RETURNS WRONG TYPE AND VALUE FOR BIT(>1)
                 FIELD.
   Parameters datatypes returned for SP bit(n) parameters are inconsistent with
   those retruned for corresponding column types.
 */
ODBC_TEST(t_bug14555713)
{
  OK_SIMPLE_STMT(Stmt, "drop procedure if exists b14555713");

  OK_SIMPLE_STMT(Stmt, "create procedure b14555713(OUT p1 bit(1), OUT p2 bit(9)) \
                begin\
                 set p1= 1;\
                 set p2= b'100100001';\
                end");

  CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, NULL, 0, NULL, 0,
                                     "b14555713", SQL_NTS, 
                                     "p%", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 6), SQL_BIT);
  is_num(my_fetch_int(Stmt, 8), 1);

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 8), 2);
  is_num(my_fetch_int(Stmt, 6), SQL_BINARY);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "drop procedure if exists b14555713");
  return OK;
}

/* Bug ODBC-38. It's impossible in MS Access to insert/update record in table containing date, time or timestamp field.
   The reason is that MS Access is ODBCv2 application, and SQLColumns returned for such fields SQL type for ODBCv3
 */
ODBC_TEST(t_odbc38)
{
  unsigned int i;
  SQLSMALLINT  expected[][4]= {{SQL_DATE,      SQL_TIME,      SQL_TIMESTAMP     , SQL_TIMESTAMP},       /* ODBCv2 */
                               {SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP}}; /* ODBCv3 */
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_odbc38");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_odbc38 (d date not null, t time not null, dt datetime not null, ts timestamp not null)");

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
    (SQLCHAR *)"t_odbc38", SQL_NTS, NULL, 0));

  for (i= 0; i < sizeof(expected[0])/sizeof(SQLSMALLINT); ++i)
  {
    diag("ODBCv%d, row#%u", OdbcVer, i);
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(my_fetch_int(Stmt, DATA_TYPE), expected[OdbcVer - SQL_OV_ODBC2][i]);
  }
  EXPECT_STMT(Stmt, SQLFetch(Stmt), SQL_NO_DATA);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE t_odbc38");

  return OK;
}


/*
Bug ODBC-51 - SQLTables failed if string fields were bound as WCHAR, and of them(usually REMARKS) contained empty string
In fact the bug is not SQLTables(or aby other catalog function) specific.
*/
ODBC_TEST(odbc51)
{
  SQLRETURN rc;
  SQLWCHAR  remark[100];
  SQLLEN    len;

  /* Making sure there is at least one table with empty remarks */
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS aaa_odbc51");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE aaa_odbc51 (a int not null)");
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, (SQLCHAR *)my_schema, strlen(my_schema), NULL, 0, NULL, 0, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_WCHAR, remark, sizeof(remark), &len));

  do {
    rc = SQLFetch(Stmt);
    if (!SQL_SUCCEEDED(rc) && rc != SQL_NO_DATA)
    {
      /* Just to get diagnostics printed */
      CHECK_STMT_RC(Stmt, rc);
    }
  }
  while (rc != SQL_NO_DATA);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE aaa_odbc51");

  return OK;
}


MA_ODBC_TESTS my_tests[]=
{
  {t_bug37621, "t_bug37621", NORMAL},
  {t_bug34272, "t_bug34272", NORMAL},
  {t_bug49660, "t_bug49660", NORMAL},
  {t_bug51422, "t_bug51422", NORMAL},
  {t_bug36441, "t_bug36441", NORMAL},
  {t_bug53235, "t_bug53235", NORMAL},
  {t_bug50195, "t_bug50195", NORMAL},
  {t_sqlprocedurecolumns, "t_sqlprocedurecolumns", KNOWN_FAILURE},
  {t_bug57182, "t_bug57182", NORMAL},
  {t_bug55870, "t_bug55870", NORMAL},
  {t_bug31067, "t_bug31067", NORMAL},
  {bug12824839,               "bug12824839",               NORMAL},
  {sqlcolumns_nodbselected,   "sqlcolumns_nodbselected",   NORMAL},
  {t_bug14085211_part1,       "t_bug14085211_part1",       NORMAL},
  {t_sqlcolumns_after_select, "t_sqlcolumns_after_select", NORMAL},
  {t_bug14555713,             "t_bug14555713",             NORMAL},
  {t_odbc38,                  "odbc2_odbc3_data_types",    NORMAL},
  { odbc51,    "odbc51_wchar_emptystring", NORMAL },
  {NULL, NULL}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  int result;

  get_options(argc, argv);
  plan(tests);

  result= run_tests(my_tests);
  /* Now one more run as ODBC2 app */
  OdbcVer= SQL_OV_ODBC2;

  return run_tests(my_tests) || result;
}
