/**
 * \file mysql_client.h
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 * \brief Class for MySQL connections
 */

#ifndef MYSQL_CLIENT_H
#define MYSQL_CLIENT_H

#include <mysql.h>
#include <my_global.h>

class mysql_client {
	private:
		MYSQL* mysql_conn;

	public:
		mysql_client(char* db_name, char* hostname, char* username, char* password);
		~mysql_client();
		///* do Query get result_set
		MYSQL_RES* doQuery(char* query);
		///* get row from result_set
		MYSQL_ROW getRow(MYSQL_RES* res_set);
		///* free result_set
		void freeResult(MYSQL_RES* res_set);
		///* get Row from query
		MYSQL_ROW getRowQuery(char* query);
};
#endif //ndef MYSQL_H
