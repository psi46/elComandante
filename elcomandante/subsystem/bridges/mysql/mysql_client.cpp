/**
 * \file mysql_client.cpp
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#include "mysql_client.h"

/**
 * constructor
 */
mysql_client::mysql_client(char* db_name, char* hostname, char* username, char* password){
	/* Verbindungs-Handle initialisieren */
	mysql_conn = mysql_init (NULL);
	if(mysql_conn == NULL){
		printf("mysql_init() failed (probably out of memory)\n");
		exit (1);
	}
	/* Serververbindung aufbauen */
	if (mysql_real_connect(mysql_conn, hostname, username, password, db_name, 0, NULL, 0) == NULL){
		fprintf(stderr,"mysql_real_connect() failed:\nError %u (%s)\n",
			mysql_errno (mysql_conn), mysql_error (mysql_conn));
		mysql_close(mysql_conn);
		exit (1);
	}
}
mysql_client::~mysql_client(){
	mysql_close(mysql_conn);
}
/**
 * perform query and get complete result_set
 */
MYSQL_RES* mysql_client::doQuery(char* query){
	MYSQL_RES *res_set;
	if (mysql_query (mysql_conn, query) != 0){
        	printf ("Could not execute query");
        	return NULL;
    	}else{
        	res_set = mysql_store_result (mysql_conn);
        	return res_set;
    	}
}
/**
 * free result set
 */
void mysql_client::freeResult(MYSQL_RES* res_set){
	if(res_set){ mysql_free_result(res_set); }
}
/**
 * extract row from result set
 */
MYSQL_ROW mysql_client::getRow(MYSQL_RES* res_set){
	MYSQL_ROW row;
	if(res_set){
		row = mysql_fetch_row(res_set);
		return row;
	}
	return NULL;
}
/**
 * Get row from result. Use this function to get one result line or insert values in a database.
 * \param query query-string to perform.
 * \return first row from result. use row[i] to get content of column i.
 */
MYSQL_ROW mysql_client::getRowQuery(char* query){
	MYSQL_RES* res_set = doQuery(query);
	MYSQL_ROW row = getRow(res_set);
	freeResult(res_set);
	return row;
}
