#include <stdio.h>
#include <mysql.h>
#include <string>

class FFError
{
public:
    std::string Label;

    FFError( ) { Label = (char *)"Generic Error"; }
    FFError( char *message ) { Label = message; }
    ~FFError() { }
    inline const char*   GetMessage  ( void )   { return Label.c_str(); }
};

using namespace std;

main()
{
    // --------------------------------------------------------------------
    // Connect to the database
    
    MYSQL      *MySQLConRet;
    MYSQL      *MySQLConnection = NULL;

    string hostName = "localhost";
    string userId   = "root";
    string password = "lolita1168";
    string DB       = "cgroups";

    MySQLConnection = mysql_init( NULL );

    try
    {
        MySQLConRet = mysql_real_connect( MySQLConnection,
                                          hostName.c_str(), 
                                          userId.c_str(), 
                                          password.c_str(), 
                                          DB.c_str(), 
                                          0, 
                                          NULL,
                                          0 );

        if ( MySQLConRet == NULL )
            throw FFError( (char*) mysql_error(MySQLConnection) );
   
        printf("MySQL Connection Info: %s \n", mysql_get_host_info(MySQLConnection));
        printf("MySQL Client Info: %s \n", mysql_get_client_info());
        printf("MySQL Server Info: %s \n", mysql_get_server_info(MySQLConnection));

    }
    catch ( FFError e )
    {
        printf("%s\n",e.Label.c_str());
        return 1;
    }
      
    int             mysqlStatus = 0;
    MYSQL_RES      *mysqlResult = NULL;

    // --------------------------------------------------------------------
    // This block of code would be performed if this insert were in a loop 
    // with changing data. Of course it is not necessary in this example.

    if(mysqlResult)
    {
        mysql_free_result(mysqlResult);
        mysqlResult = NULL;
    }

    // --------------------------------------------------------------------
    // Perform a SQL table create

    try
    {
        string sqlInsStatement = "CREATE TABLE cgroupsContiki(id INT primary key, \
						name VARCHAR(25),\
						ipv4 VARCHAR(25),\
						port VARCHAR(25),\
						mac VARCHAR(25))";
        mysqlStatus = mysql_query( MySQLConnection, sqlInsStatement.c_str() );
        if (mysqlStatus)
        {
            throw FFError( (char*)mysql_error(MySQLConnection) );
        }
    }
    catch ( FFError e )
    {
        printf("%s\n",e.Label.c_str());
        mysql_close(MySQLConnection);
        return 1;
    }

    if(mysqlResult)
    {
        mysql_free_result(mysqlResult);
        mysqlResult = NULL;
    }
    
    // --------------------------------------------------------------------
    // Perform a SQL SELECT and retrieve data

    MYSQL_ROW       mysqlRow;
    MYSQL_FIELD    *mysqlFields;
    my_ulonglong    numRows;
    unsigned int    numFields;
    
    try
    {
        string sqlSelStatement = "SELECT * FROM cgroupsContiki";
        mysqlStatus = mysql_query( MySQLConnection, sqlSelStatement.c_str() );

        if (mysqlStatus)
            throw FFError( (char*)mysql_error(MySQLConnection) );
        else
            mysqlResult = mysql_store_result(MySQLConnection); // Get the Result Set

        if (mysqlResult)  // there are rows
        {
            // # of rows in the result set
            numRows = mysql_num_rows(mysqlResult);

            // # of Columns (mFields) in the latest results set
            numFields = mysql_field_count(MySQLConnection);

            // Returns the number of columns in a result set specified
            numFields = mysql_num_fields(mysqlResult);

            printf("Number of rows=%llu  Number of fields=%u \n",numRows,numFields);
        }
        else
        {
           printf("Result set is empty");
        }
    
        // Print column headers

        mysqlFields = mysql_fetch_fields(mysqlResult);

        for(int jj=0; jj < numFields; jj++)
        {
            printf("%s\t",mysqlFields[jj].name);
        }
        printf("\n");
     
        // print query results

        while(mysqlRow = mysql_fetch_row(mysqlResult)) // row pointer in the result set
        {
            for(int ii=0; ii < numFields; ii++)
            {
                printf("%s\t", mysqlRow[ii] ? mysqlRow[ii] : "NULL");  // Not NULL then print
            }
            printf("\n");
        }

        if(mysqlResult)
        {
            mysql_free_result(mysqlResult);
            mysqlResult = NULL;
        }
    }
    catch ( FFError e )
    {
        printf("%s\n",e.Label.c_str());
        mysql_close(MySQLConnection);
        return 1;
    }

    // --------------------------------------------------------------------
    // Close datbase connection

    mysql_close(MySQLConnection);

    return 0;
}

