#include <my_global.h>
#include <mysql.h>

int main(int argc, char **argv)
{
  	printf("MySQL client version: %s\n", mysql_get_client_info());

	MYSQL *conn;

  	conn = mysql_init(NULL);

  	if (conn == NULL) {
      		printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
      		exit(1);
  	}

  	if (mysql_real_connect(conn, "localhost", "root", 
          	"lolita1168", NULL, 0, NULL, 0) == NULL) {
      		printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
      		exit(1);
  	}

  	if (mysql_query(conn, "create database testdb")) {
      		printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
      		exit(1);
  	}


  	mysql_close(conn);
}
