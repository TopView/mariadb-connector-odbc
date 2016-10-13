/************************************************************************************
   Copyright (C) 2013,2016 MariaDB Corporation AB
   
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
#include <ma_odbc.h>

extern MARIADB_CHARSET_INFO*  utf16;

char *MADB_GetTableName(MADB_Stmt *Stmt)
{
  char *TableName= NULL;
 unsigned  int i= 0;
  if (Stmt->TableName && Stmt->TableName[0])
    return Stmt->TableName;
  if (!mysql_stmt_field_count(Stmt->stmt))
    return NULL;

  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    if (Stmt->stmt->fields[i].org_table)
    {
      if (!TableName)
        TableName= Stmt->stmt->fields[i].org_table;
      if (strcmp(TableName, Stmt->stmt->fields[i].org_table))
      {
        MADB_SetError(&Stmt->Error, MADB_ERR_HY000, "Couldn't identify unique table name", 0);
        return NULL;
      }
    }
  }
  if (TableName)
    Stmt->TableName= _strdup(TableName);
  return TableName;
}

char *MADB_GetCatalogName(MADB_Stmt *Stmt)
{
  char *CatalogName= NULL;
  unsigned int i= 0;
  if (Stmt->CatalogName && Stmt->CatalogName[0])
    return Stmt->CatalogName;
  if (!mysql_stmt_field_count(Stmt->stmt))
    return NULL;

  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    if (Stmt->stmt->fields[i].org_table)
    {
      if (!CatalogName)
        CatalogName= Stmt->stmt->fields[i].db;
      if (strcmp(CatalogName, Stmt->stmt->fields[i].db))
      {
        MADB_SetError(&Stmt->Error, MADB_ERR_HY000, "Couldn't identify unique catalog name", 0);
        return NULL;
      }
    }
  }
  if (CatalogName)
    Stmt->CatalogName= _strdup(CatalogName);
  return CatalogName;
}

my_bool MADB_DynStrAppendQuoted(DYNAMIC_STRING *DynString, char *String)
{
  if (ma_dynstr_append(DynString, "`") ||
      ma_dynstr_append(DynString, String) ||
      ma_dynstr_append(DynString, "`"))
    return TRUE;
  return FALSE;
}

my_bool MADB_DynStrUpdateSet(MADB_Stmt *Stmt, DYNAMIC_STRING *DynString)
{
  unsigned int i;
  int  IgnoredColumns= 0, Count= 0;
  MADB_DescRecord *Record;
  if (ma_dynstr_append(DynString, " SET "))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
    return TRUE;
  }
  // ???? memcpy(&Stmt->Da->Apd->Header, &Stmt->Ard->Header, sizeof(MADB_Header));
  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    SQLINTEGER *IndicatorPtr= NULL;
    Record= MADB_DescGetInternalRecord(Stmt->Ard, i, MADB_DESC_READ);
    if (Record->IndicatorPtr)
      IndicatorPtr= (SQLINTEGER *)GetBindOffset(Stmt->Ard, Record, Record->IndicatorPtr, MAX(0, Stmt->DaeRowNumber-1), Record->OctetLength);
 /*   if ((IndicatorPtr && *IndicatorPtr == SQL_COLUMN_IGNORE) || !Record->inUse)
    {
      IgnoredColumns++;
      continue;
    } */
    
    if ((i - IgnoredColumns) && ma_dynstr_append(DynString, ","))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
      return TRUE;
    }
    if (MADB_DynStrAppendQuoted(DynString, Stmt->stmt->fields[i].org_name))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
      return TRUE;
    }
    if (ma_dynstr_append(DynString, "=?"))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
      return TRUE;
    }
  }
  if (IgnoredColumns == mysql_stmt_field_count(Stmt->stmt))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_21S02, NULL, 0);
    return TRUE;
  }
  return FALSE;
}

my_bool MADB_DynStrInsertSet(MADB_Stmt *Stmt, DYNAMIC_STRING *DynString)
{
  DYNAMIC_STRING ColVals;
  unsigned int i;
  int IgnoredColumns= 0, Count= 0;
  MADB_DescRecord *Record;

  ma_init_dynamic_string(&ColVals, "VALUES (", 32, 32);
  if (ma_dynstr_append(DynString, " (") )
  {
    goto dynerror;
    
    return TRUE;
  }
  // ???? memcpy(&Stmt->Da->Apd->Header, &Stmt->Ard->Header, sizeof(MADB_Header));
  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    SQLINTEGER *IndicatorPtr= NULL;
    Record= MADB_DescGetInternalRecord(Stmt->Ard, i, MADB_DESC_READ);
    if (Record->IndicatorPtr)
      IndicatorPtr= (SQLINTEGER *)GetBindOffset(Stmt->Ard, Record, Record->IndicatorPtr, MAX(0, Stmt->DaeRowNumber-1), Record->OctetLength);
    
    if ((i) && 
        (ma_dynstr_append(DynString, ",") || ma_dynstr_append(&ColVals, ",")))
      goto dynerror;

    if (MADB_DynStrAppendQuoted(DynString, Stmt->stmt->fields[i].org_name) ||
        ma_dynstr_append(&ColVals, "?"))
       goto dynerror;
  }
  if (ma_dynstr_append(DynString, ") " ) ||
      ma_dynstr_append(&ColVals, ")") ||
      ma_dynstr_append(DynString, ColVals.str))
    goto dynerror;
  ma_dynstr_free(&ColVals);
  return FALSE;
dynerror:
  MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
  ma_dynstr_free(&ColVals);
  return TRUE;
}

my_bool MADB_DynStrGetColumns(MADB_Stmt *Stmt, DYNAMIC_STRING *DynString)
{
  unsigned int i;
  if (ma_dynstr_append(DynString, " ("))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
    return TRUE;
  }
  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    if (i && ma_dynstr_append(DynString, ", "))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
      return TRUE;
    }
    if (MADB_DynStrAppendQuoted(DynString, Stmt->stmt->fields[i].org_name))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
      return TRUE;
    }
  }
  if (ma_dynstr_append(DynString, " )"))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
    return TRUE;
  }
  return FALSE;
}

my_bool MADB_DynStrGetWhere(MADB_Stmt *Stmt, DYNAMIC_STRING *DynString, char *TableName, my_bool ParameterMarkers)
{
  int UniqueCount=0, PrimaryCount= 0;
  unsigned int i;
  int Flag= 0;
  char *Column= NULL;
  SQLLEN StrLength;

  for (i=0; i < mysql_stmt_field_count(Stmt->stmt);i++)
  {
    if (Stmt->stmt->fields[i].flags & PRI_KEY_FLAG)
      PrimaryCount++;
    if (Stmt->stmt->fields[i].flags & UNIQUE_KEY_FLAG)
      UniqueCount++;
  }
  /* We need to use all columns, otherwise it will be difficult to map fields for Positioned Update */
  if (PrimaryCount && PrimaryCount != MADB_KeyTypeCount(Stmt->Connection, TableName, PRI_KEY_FLAG))
    PrimaryCount= 0;
  if (UniqueCount && UniqueCount != MADB_KeyTypeCount(Stmt->Connection, TableName, UNIQUE_KEY_FLAG))
    UniqueCount= 0;
  
  /* if no primary or unique key is in the cursorm the cursor must contain all
     columns from table in TableName */
  if (!PrimaryCount && !UniqueCount)
  {
    char      StmtStr[256];
    MADB_Stmt *CountStmt;
    int       FieldCount= 0;

    MA_SQLAllocHandle(SQL_HANDLE_STMT, Stmt->Connection, (SQLHANDLE*)&CountStmt);
    snprintf(StmtStr, 256, "SELECT * FROM `%s` LIMIT 0", TableName);
    CountStmt->Methods->ExecDirect(CountStmt, (SQLCHAR *)StmtStr, SQL_NTS);
    FieldCount= mysql_stmt_field_count(((MADB_Stmt *)CountStmt)->stmt);
    CountStmt->Methods->StmtFree(CountStmt, SQL_DROP);

    if (FieldCount != mysql_stmt_field_count(Stmt->stmt))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_S1000, "Can't build index for update/delete", 0);
      return TRUE;
    }
  }
  if (ma_dynstr_append(DynString, " WHERE 1"))
    goto memerror;
  for (i=0; i < mysql_stmt_field_count(Stmt->stmt);i++)
  {
    if (Stmt->stmt->fields[i].flags & Flag || !Flag)
    {
      if (ma_dynstr_append(DynString, " AND ") ||
          MADB_DynStrAppendQuoted(DynString, Stmt->stmt->fields[i].org_name))
          goto memerror;
      if (ParameterMarkers)
      {
        if (ma_dynstr_append(DynString, "=?"))
          goto memerror;
      }
      else
      { 
        if (!SQL_SUCCEEDED(Stmt->Methods->GetData(Stmt, i+1, SQL_C_CHAR, NULL, 0, &StrLength)))
        {
          MADB_FREE(Column);
          return TRUE;
        }
        if (StrLength < 0)
        {
           if (ma_dynstr_append(DynString, " IS NULL"))
             goto memerror;
        }
        else
        {
          Column= MADB_CALLOC(StrLength + 1);
          Stmt->Methods->GetData(Stmt,i+1, SQL_C_CHAR, Column, StrLength + 1, NULL);
          if (ma_dynstr_append(DynString, "= '") ||
                 ma_dynstr_append(DynString, Column) ||
                 ma_dynstr_append(DynString, "'"))
          goto memerror;
          MADB_FREE(Column);
          Column= NULL;
        }
      }
    }
  }
  if (ma_dynstr_append(DynString, " LIMIT 1"))
    goto memerror;
  MADB_FREE(Column);
  return FALSE;
memerror:
  MADB_FREE(Column);
  MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
  return TRUE;
}

my_bool MADB_DynStrGetValues(MADB_Stmt *Stmt, DYNAMIC_STRING *DynString)
{
  unsigned int i;
  if (ma_dynstr_append(DynString, " VALUES("))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
    return TRUE;
  }
  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    if (ma_dynstr_append(DynString, (i) ? ",?" : "?"))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
      return TRUE;
    }
  }
  if (ma_dynstr_append(DynString, ")"))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
    return TRUE;
  }
  return FALSE;
}

char *MADB_GetInsertStatement(MADB_Stmt *Stmt)
{
  char *StmtStr;
  size_t Length= 1024;
  char *p;
  char *TableName;
  unsigned int i;

  if (!(StmtStr= MADB_CALLOC(1024)))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_HY013, NULL, 0);
    return NULL;
  }
  if (!(TableName= MADB_GetTableName(Stmt)))
    goto error;
  p= StmtStr;
  
  p+= snprintf(StmtStr, 1024, "INSERT INTO `%s` (", TableName);
  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    if (strlen(StmtStr) > Length - NAME_LEN - 4/* comma + 2 ticks + terminating NULL */)
    {
      Length+= 1024;
      if (!(StmtStr= MADB_REALLOC(StmtStr, Length)))
      {
        MADB_SetError(&Stmt->Error, MADB_ERR_HY013, NULL, 0);
        goto error;
      }
    }
    p+= snprintf(p, Length - strlen(StmtStr), "%s`%s`", (i==0) ? "" : ",", Stmt->stmt->fields[i].org_name);
  }
  p+= snprintf(p, Length - strlen(StmtStr), ") VALUES (");

  if (strlen(StmtStr) > Length - mysql_stmt_field_count(Stmt->stmt)*2 - 1)/* , and ? for each column  + (- 1 comma for 1st column + closing ')')
                                                                            + terminating NULL */
  {
    Length= strlen(StmtStr) + mysql_stmt_field_count(Stmt->stmt)*2 + 1;
    if (!(StmtStr= MADB_REALLOC(StmtStr, Length)))
    {
      MADB_SetError(&Stmt->Error, MADB_ERR_HY013, NULL, 0);
      goto error;
    }
  }

  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    p+= snprintf(p, Length - strlen(StmtStr), "%s?", (i==0) ? "" : ",");
  }
  p+= snprintf(p, Length - strlen(StmtStr), ")");
  return StmtStr;

error:
  if (StmtStr)
    MADB_FREE(StmtStr);
  return NULL;
}

void StripComments(char *s) {
  char *a, *b,
       *ca= "/*",
       *cb="*/";
  size_t len = strlen(s) + 1;
 
  while ((a = strstr(s, ca)) != NULL)
  {
	  b = strstr(a+2, cb);
	  if (b == NULL)
	    break;
	  b += 2;
	  memmove(a, b, strlen(b)+1);
  }
}

my_bool MADB_IsStatementSupported(char *StmtStr, char *token1, char *token2)
{
  my_bool ret= TRUE;
  char *Copy= _strdup(StmtStr);
  char *p;
#ifndef _WIN32
  char *r;
#endif

  StripComments(Copy);
  p= strtok_r(Copy, " \t", &r);
  if (p && strcasecmp(p, token1) != 0)
    goto end;

  p= strtok_r(NULL, " \t=", &r);
  if (p && strcasecmp(p, token2) == 0)
    ret= FALSE;
end:
  MADB_FREE(Copy);
  return ret;
}

my_bool MADB_ValidateStmt(char *StmtStr)
{
  return MADB_IsStatementSupported(StmtStr, "SET", "NAMES");
}


int InitClientCharset(Client_Charset *cc, const char * name)
{
  cc->cs_info= mariadb_get_charset_by_name(name);

  if (cc->cs_info == NULL)
  {
    return 1;
  }

  cc->CodePage= cc->cs_info->codepage;/*madb_get_windows_cp(name);*/

  return 0;
}


Client_Charset* GetDefaultOsCharset(Client_Charset *cc)
{
  if (InitClientCharset(cc, madb_get_os_character_set()))
  {
    return NULL;
  }
  return cc;
}


void CloseClientCharset(Client_Charset *cc)
{
}


/* Hmmm... Length in characters is SQLLEN, octet length SQLINTEGER */
SQLLEN MbstrOctetLen(char *str, SQLLEN *CharLen, MARIADB_CHARSET_INFO *cs)
{
  SQLLEN result= 0, inChars= *CharLen;

  if (str)
  {
    if (cs->mb_charlen == NULL)
    {
      /* Charset uses no more than a byte per char. Result is strlen or umber of chars */
      if (*CharLen < 0)
      {
        result= (SQLLEN)strlen(str);
        *CharLen= result;
      }
      else
      {
        result= *CharLen;
      }
      return result;
    }
    else
    {
      while (inChars > 0 || inChars < 0 && *str)
      {
        result+= cs->mb_charlen(*str);
        --inChars;
        str+= cs->mb_charlen(*str);
      }
    }
  }

  if (*CharLen < 0)
  {
    *CharLen-= inChars;
  }
  return result;
}


/* Number of characters in given number of bytes */
SQLLEN MbstrCharLen(char *str, SQLINTEGER OctetLen, MARIADB_CHARSET_INFO *cs)
{
  SQLLEN       result= 0;
  char        *ptr= str;
  unsigned int charlen;

  if (str)
  {
    if (cs->mb_charlen == NULL || cs->char_maxlen == 1)
    {
      return OctetLen;
    }
    while (ptr < str + OctetLen)
    {
      charlen= cs->mb_charlen((unsigned char)*ptr);
      if (charlen == 0)
      {
        /* Dirty hack to avoid dead loop - Has to be the error! */
        charlen= 1;
      }
      ptr+= charlen;
      ++result;
    }
  }

  return result;
}


/* Length of NT SQLWCHAR string in characters */
SQLINTEGER SqlwcsCharLen(SQLWCHAR *str, SQLLEN octets)
{
  SQLINTEGER result= 0;
  SQLWCHAR   *end=   octets != (SQLLEN)-1 ? str + octets/sizeof(SQLWCHAR) : (SQLWCHAR*)octets /*for simplicity - the address to be always bigger */;

  if (str)
  {
    while (str < end && *str)
    {
      str+= (utf16->mb_charlen(*str))/sizeof(SQLWCHAR);

      if (str > end)
      {
        break;
      }
      ++result;
    }
  }
  return result;
}


/* Length in SQLWCHAR units*/
SQLINTEGER SqlwcsLen(SQLWCHAR *str)
{
  SQLINTEGER result= 0;

  if (str)
  {
    while (*str)
    {
      ++result;
      /* str+= (utf16->mb_charlen(*str))/sizeof(SQLWCHAR)); */
      ++str;
    }
  }
  return result;
}

