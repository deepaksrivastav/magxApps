#include "MgxBox.h"

#include <sqlite3.h>

/*  Qt includes */
#include <qfile.h>
#include <qstring.h>
#include <quuid.h>

char sqlInitScript[] =  "BEGIN TRANSACTION;"
                        "CREATE TABLE AppReg ("
                        "   id INTEGER PRIMARY KEY,"
                        "   UUID CHAR(38),"
                        "   Name VARCHAR(64),"
                        "   PathName VARCHAR(255),"
                        "   Location INT,"
                        "   RegDate DATE,"
                        "   menuUUID CHAR(38)"
                        ");"
                        "CREATE TABLE App_Menu"
                        "("
                        "   app_id	INTENGER NOT NULL"
                        "       CONSTRAINT fk_app_id REFERENCES AppReg(id) ON DELETE CASCADE"
                        ");"
                        "COMMIT;";

sqlite3* AppDb = NULL;

bool APPDB_Open(char* sqlInitScript);
bool APPDB_Close();
bool APPDB_CheckAppInDB(char* aPath,QUuid* appUUID);
void APPDB_InsertAppToDB(char* aName, char* aPath, int loc);


static int abort_callback(void *NotUsed, int argc, char **argv, char **azColName){
    return 1;
}
bool APPDB_Open()
{
    int rc;
    char sqlCheckEmpty[] = "SELECT * FROM sqlite_master\0";

    rc = sqlite3_open(APP_DB_FULLPATH, &AppDb);
    if( rc )
    {
        DBG_OUT("Can't open Applications database: %s", sqlite3_errmsg(AppDb) );

        fprintf(stderr, "Can't open Applications database: %s\n", sqlite3_errmsg(AppDb) );
        sqlite3_close(AppDb);
        return false;
    }

    rc = sqlite3_exec(AppDb, sqlCheckEmpty, abort_callback, NULL, NULL);
    if ( rc != SQLITE_ABORT )
    {
        DBG_OUT("Creating Application Database");
        if ( sqlite3_exec(AppDb, sqlInitScript, (sqlite3_callback)NULL, NULL, NULL) )
        {
            DBG_OUT("Error executing Init Script");
        }
    }
    printf("Application Database Opened\n");
    return true;
}

bool APPDB_Close()
{
    if (AppDb!=NULL)
    {
        if (sqlite3_close(AppDb)!=SQLITE_OK)
        {
            DBG_OUT("Error while closing Application Database");

            printf("Error while closing Application Database\n");
            return false;
        }
    }
    printf("Application Database Closed\n");
    return true;
}

void APPDB_AddApp(QUuid* appUUID,const char* aName,const char* aPath, int loc)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[39] = {0};

    char sqlInsert[] = "INSERT INTO AppReg(UUID,Name,pathName,Location) VALUES(?, ?, ?, ?);";

	if (sqlite3_prepare(AppDb, sqlInsert, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return;

    sprintf(cUUID,"%s", appUUID->toString().ascii() );

    sqlite3_bind_text(hStmt, 1, cUUID, -1, NULL);
	sqlite3_bind_text(hStmt, 2, (const char*)aName, -1, NULL);
	sqlite3_bind_text(hStmt, 3, (const char*)aPath, -1, NULL);
	sqlite3_bind_int(hStmt, 4, loc);

	sqlite3_step(hStmt);
	sqlite3_finalize(hStmt);
}

bool APPDB_isAppMenuRegisteredInDB(const char* aPath,QUuid* appUUID)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[39] = {0};
    int appCount = 0;

    char sqlSelect[] = "SELECT COUNT(*) FROM AppReg WHERE id IN ( SELECT app_id FROM App_Menu) AND ( PathName=? OR UUID = ? );";

	if (sqlite3_prepare(AppDb, sqlSelect, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    if (appUUID!=NULL)
        sprintf(cUUID,"%s", appUUID->toString().ascii() );

    sqlite3_bind_text(hStmt, 1, aPath, -1, NULL);
    sqlite3_bind_text(hStmt, 2, cUUID, -1, NULL);

    sqlite3_step(hStmt);
    appCount = sqlite3_column_int(hStmt,0);
    DBG_OUT("REGISTERED Count for PathName ('%s') = %d",aPath,appCount);
    if (appCount == 0 )
	{
        sqlite3_finalize(hStmt);
        return false;
	}
	sqlite3_finalize(hStmt);
    return true;
}

bool APPDB_isAppRegisteredInDB(const char* aPath,QUuid* appUUID)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[39] = {0};
    int appCount = 0;

    char sqlCheckApp[] = "SELECT COUNT(*) FROM AppReg WHERE PathName=? OR UUID = ?;";

	if (sqlite3_prepare(AppDb, sqlCheckApp, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    if (appUUID!=NULL)
        sprintf(cUUID,"%s", appUUID->toString().ascii() );
    sqlite3_bind_text(hStmt, 1, aPath, -1, NULL);
    sqlite3_bind_text(hStmt, 2, cUUID, -1, NULL);

    sqlite3_step(hStmt);
    appCount = sqlite3_column_int(hStmt,0);
    DBG_OUT("Count for PathName ('%s') = %d",aPath,appCount);
    if (appCount == 0 )
	{
        sqlite3_finalize(hStmt);
        return false;
	}
	sqlite3_finalize(hStmt);
    return true;
}

bool APPDB_isUUIDusedDB(QUuid* appUUID)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[39] = {0};
    int appCount = 0;

    if (appUUID==NULL)
        return true;

    char sqlCheckApp[] = "SELECT COUNT(*) FROM AppReg WHERE UUID = ?;";

	if (sqlite3_prepare(AppDb, sqlCheckApp, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    sprintf(cUUID,"%s", appUUID->toString().ascii() );
    sqlite3_bind_text(hStmt, 1, cUUID, -1, NULL);

    sqlite3_step(hStmt);
    appCount = sqlite3_column_int(hStmt,0);
    DBG_OUT("Count for PathName ('%s') = %d",cUUID,appCount);
    if (appCount == 0 )
	{
        sqlite3_finalize(hStmt);
        return false;
	}
	sqlite3_finalize(hStmt);
    return true;
}

bool APPDB_UpdateAppInfoMenuUID(QUuid* appUUID,QString menuUID)
{
    printf("Menu UUID : %s\n",menuUID.latin1()); 
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[38] = {0};

    char sqlCmd[] = "UPDATE AppReg SET menuUUID = ? WHERE UUID = ?;";

	if (sqlite3_prepare(AppDb, sqlCmd, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    if (appUUID!=NULL)
        sprintf(cUUID,"%s", appUUID->toString().ascii() );

    sqlite3_bind_text(hStmt, 1, menuUID.latin1(), -1, NULL);
    sqlite3_bind_text(hStmt, 2, cUUID, -1, NULL);

    if ( sqlite3_step(hStmt)!=SQLITE_OK)
    {
        sqlite3_finalize(hStmt);
        return false;
	}
	sqlite3_finalize(hStmt);
    return true;
}

// select UUID from app_menu where app_id in (select id from appreg where pathname = '/mmc/mmca1/mgxbox/mgxbox' )
bool APPDB_RegisterAppInMenu(const char* aPath,QUuid* appUUID, QString menuUUID)
{
    printf("Inserting into SQL DB\n");
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[39] = {0};

    char sqlInsertMenuRegApp[] = "INSERT INTO App_Menu(app_id) SELECT id from AppReg WHERE PathName = ? OR UUID = ?;";

	if (sqlite3_prepare(AppDb, sqlInsertMenuRegApp, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    if (appUUID!=NULL)
        sprintf(cUUID,"%s", appUUID->toString().ascii() );
    

    sqlite3_bind_text(hStmt, 1, aPath, -1, NULL);
    sqlite3_bind_text(hStmt, 2, cUUID , -1, NULL);

    DBG_OUT("Registered App in Path ('%s')", aPath);

    if ( sqlite3_step(hStmt)!=SQLITE_OK)
    {
        sqlite3_finalize(hStmt);
        APPDB_UpdateAppInfoMenuUID(appUUID,menuUUID);
        return false;
	}
	sqlite3_finalize(hStmt);
	
	
	
	return true;
}

bool APPDB_UnregisterAppFromMenu(QUuid* appUUID)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[39] = {0};

    char sqlInsertMenuRegApp[] = "DELETE FROM App_Menu WHERE app_id IN ( SELECT id from AppReg WHERE UUID = ? ) ;";
    if (appUUID==NULL)
        return false;

	if (sqlite3_prepare(AppDb, sqlInsertMenuRegApp, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    sprintf(cUUID,"%s", appUUID->toString().ascii() );
    sqlite3_bind_text(hStmt, 1, cUUID , -1, NULL);

    if ( sqlite3_step(hStmt)!=SQLITE_OK)
    {
        sqlite3_finalize(hStmt);
        return false;
	}
	sqlite3_finalize(hStmt);
    return true;
}

bool APPDB_getAppUUID(const char* aPath,char* ptrUUID)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;

    char* cUUID = NULL;

    char sqlGetUUIDAppInDB[] = "SELECT UUID FROM AppReg WHERE PathName = ? ;";

	if (sqlite3_prepare(AppDb, sqlGetUUIDAppInDB, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;
    sqlite3_bind_text(hStmt, 1, aPath, -1, NULL);
    if ( sqlite3_step(hStmt)!=SQLITE_ROW)
    {
        sqlite3_finalize(hStmt);
        return false;
	}
	cUUID  = (char*)sqlite3_column_text(hStmt, 0);
	strcpy(ptrUUID, cUUID);
	sqlite3_finalize(hStmt);
    return true;
}

bool APPDB_getMenuUUIDFromAppId(QUuid* appUUID,char* menuUUID)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;

    char cUUID[38] = {0};
    char *mUUID = NULL;
    
    if (appUUID!=NULL)
        sprintf(cUUID,"%s", appUUID->toString().ascii() );

    char sqlGetUUIDAppInDB[] = "SELECT menuUUID FROM AppReg WHERE UUID = ? ;";

	if (sqlite3_prepare(AppDb, sqlGetUUIDAppInDB, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;
    sqlite3_bind_text(hStmt, 1, cUUID, -1, NULL);
    if ( sqlite3_step(hStmt)!=SQLITE_ROW)
    {
        sqlite3_finalize(hStmt);
        return false;
	}
	mUUID  = (char*)sqlite3_column_text(hStmt, 0);
	strcpy(menuUUID, mUUID);
	sqlite3_finalize(hStmt);
    return true;
}

bool APPDB_UpdateAppInfo(QUuid* appUUID,const char* aName,const char* aPath, int loc)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[38] = {0};

    char sqlCmd[] = "UPDATE AppReg SET Name = ?, PathName = ?, Location = ? WHERE UUID = ?;";

	if (sqlite3_prepare(AppDb, sqlCmd, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    if (appUUID!=NULL)
        sprintf(cUUID,"%s", appUUID->toString().ascii() );

    sqlite3_bind_text(hStmt, 1, aName, -1, NULL);
    sqlite3_bind_text(hStmt, 2, aPath, -1, NULL);
    sqlite3_bind_int(hStmt, 3, loc);
    sqlite3_bind_text(hStmt, 4, cUUID, -1, NULL);

    if ( sqlite3_step(hStmt)!=SQLITE_OK)
    {
        sqlite3_finalize(hStmt);
        return false;
	}
	sqlite3_finalize(hStmt);
    return true;
}

bool APPDB_RemoveApp(QUuid* appUUID)
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;
    char cUUID[39] = {0};

    char sqlCmd[] = "DELETE FROM AppReg WHERE UUID = ?;";
    if (appUUID==NULL)
        return false;

	if (sqlite3_prepare(AppDb, sqlCmd, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return false;

    sprintf(cUUID,"%s", appUUID->toString().ascii() );
    sqlite3_bind_text(hStmt, 1, cUUID, -1, NULL);

    if ( sqlite3_step(hStmt)!=SQLITE_OK)
    {
        sqlite3_finalize(hStmt);
        return false;
	}
	sqlite3_finalize(hStmt);
    return true;
}
