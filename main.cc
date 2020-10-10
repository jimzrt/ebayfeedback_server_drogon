#include <drogon/drogon.h>
// #include <iostream>
// #include <string>
// #include <sqlite3.h>

using namespace std;
int main()
{

    // // Pointer to SQLite connection
    // sqlite3 *db;
    // // Save any error messages
    // char *zErrMsg = 0;
    // // Save the result of opening the file
    // int rc;
    // // Save any SQL
    // string sql;
    // // Save the result of opening the file
    // rc = sqlite3_open("../models/example.db", &db);
    // if (rc)
    // {
    //     // Show an error message
    //     cout << "DB Error: " << sqlite3_errmsg(db) << endl;
    //     // Close the connection
    //     sqlite3_close(db);
    //     // Return an error
    //     return (1);
    // }
    // // Save SQL to create a table
    // sql = "CREATE TABLE IF NOT EXISTS EBAY_RESPONSE ("
    //       "USERNAME TEXT NOT NULL UNIQUE,"
    //       "BODY           TEXT    NOT NULL,"
    //       "ACCESS_TIME TEXT DEFAULT CURRENT_TIMESTAMP NOT NULL);"
    //       "CREATE UNIQUE INDEX IF NOT EXISTS index_name on EBAY_RESPONSE (USERNAME);";
    // // Run the SQL (convert the string to a C-String with c_str() )
    // rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &zErrMsg);
    // // Close the SQL connection
    // sqlite3_close(db);

    //Set HTTP listener address and port
    //drogon::app().addListener("0.0.0.0", 80);
    //Load config file
    drogon::app().loadConfigFile("../config.json");
    //Run HTTP framework,the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
