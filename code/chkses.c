
#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid.h>

char udb[37];


int check_valid_user(char *string)
{
	char query[1000];
	int retval=-1;
	sqlite3 * userdb;
	udb[0]=0;
	sprintf(query,"select dbd from upass where encid like '%s';",string);
	uint32_t userdb_status = sqlite3_open_v2("db/userdb",&userdb,SQLITE_OPEN_READONLY,NULL);
	if( userdb_status == SQLITE_OK )
	{
		sqlite3_stmt *st;
		int sret=sqlite3_prepare_v2(userdb,query,-1,&st,NULL);
		int stret = sqlite3_step(st);
		if( sret == SQLITE_OK && stret == SQLITE_ROW)
		{
			memcpy(udb,sqlite3_column_text(st,0),37);
			retval = 1;
			sqlite3_finalize(st);
			sqlite3_close(userdb);
		}

	}
	return retval;
}

int main()
{
	sqlite3 *curdb;
	int ret_status_code = -1;
	char * len = getenv("CONTENT_LENGTH");
	long ll=0;
	if( len != NULL && (ll = atoi(len))>0 )
	{

		int i=0;
		char  userstring[1024]; 
		while( i<ll && i<1024 )
		{
			userstring[i] = getchar();
			i++;
		}
		userstring[i]='\0';
		char userID[100];
		int ui=0;
		while( userstring[ui] != ',' && ui<ll )
		{
			userID[ui]=userstring[ui];
			ui++;

		}
		userID[ui]=0;
		ui++;
		if( i==ll && ui<ll &&  check_valid_user(userID) == 1 && udb[0]!=0 )
		{
			char db_name[200];
			sprintf(db_name,"db/%s.db",udb);
			// 1 in active means not active 0 means acitve;
			uint32_t db_status = sqlite3_open_v2(db_name,&curdb,SQLITE_OPEN_READWRITE,NULL);
			if( db_status == SQLITE_OK )
			{
				//set all previous sessions to inactive.
				char del_query[150];
				sprintf(del_query,"select sestkn from session where sestkn like '%s' and active=1 and (select UNIXEPOCH())-time<=60*15;",&userstring[ui]);
				sqlite3_stmt *st;
				int sret=sqlite3_prepare_v2(curdb,del_query,-1,&st,NULL);
				int stret = sqlite3_step(st);
				if( sret == SQLITE_OK && stret == SQLITE_ROW && strcmp(&userstring[ui],sqlite3_column_text(st,0))==0 )
				{
					memcpy(udb,sqlite3_column_text(st,0),37);
					sqlite3_finalize(st);
					puts("Status: 200 OK\r");
					puts("Content-Type: text/html\r");
					puts("\r");
					puts("valid");
					ret_status_code=1;
				}
				sqlite3_close(curdb);
			}
		}	
		if( ret_status_code==-1)
		{
			puts("Status: 503 ERROR\r");
			puts("Content-Type: text/html\r");
			puts("\r");
			puts("invalid");
		}
	}
}
	/*
	   sqlite3 *mydb;
	   int sqlRet=sqlite3_open_v2("test.db",&mydb,SQLITE_OPEN_READWRITE,NULL);
	   if( sqlRet == SQLITE_OK )
	   {
	   char stmt[] = "insert into sample values('i',1);";
	   printf("connection established succesfully\n");
	   sqlite3_close(mydb);
	   }
	   printf("sqlRet:%d\n",sqlRet);
	 */
