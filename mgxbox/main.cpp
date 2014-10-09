#include "MgxBox.h"

#include <sqlite3.h>

#include <ZApplication.h>
#include "zBaseGUI.h"

#include <iostream>
using namespace std;


ZConfig* cLng = NULL;
QString appName = QString::null;
QString appPath = QString::null;

QString FullPath_7z = QString::null;
QString appMenuUUID = "21596a0c-ab4e-ea34-5f66-4aede0f4855b";

QMap<QString, AMRegEntry* > appRegList;

bool sdExists = false;
bool NeedRefreshMenu = false;

extern sqlite3* AppDb;

extern bool APPDB_Open();
extern bool APPDB_Close();
extern bool APPDB_isAppRegisteredInDB(const char* aPath,QUuid* appUUID);
extern bool APPDB_isAppMenuRegisteredInDB(const char* aPath,QUuid* appUUID);
extern void APPDB_InsertAppToDB(QUuid* appUUID,const char* aName,const char* aPath, int loc);
extern bool APPDB_SetMenuRegisteredAppInDB(const char* aPath,QUuid* appUUID);
extern bool APPDB_getUUIDAppRegisteredInDB(const char* aPath,char* ptrUUID);
extern bool APPDB_SetMenuUnregisteredAppInDB(const char* aPath,QUuid* appUUID);
extern QString lc(const char *key);

extern bool ConfirmAction(QString title, QString text);
extern bool sys_exec_cmd(QString cmd,bool inMultiOper = false);

bool checkParameters(int argc, char** argv);
bool detectLanguageFile(QString srcPath);
bool LoadCustomizationFile(QString srcPath);
void checkDirectories();

void LoadAllRegisteredApp(bool ForceReload = false);

void ProcessAppRegist();

int main(int argc, char** argv)
{
    QString fPath = QString(argv[0]);
    appName = fPath.mid( fPath.findRev("/") +1 );
    appPath = fPath.left( fPath.findRev("/") );

    DBG_OUT("App Name : %s", appName.ascii() );
    DBG_OUT("App Path : %s", appPath.ascii() );

    checkDirectories();
    APPDB_Open();

	ZApplication* app = new ZApplication(argc, argv);

    FullPath_7z = APP_FULLPATH_7Z;

    if ( !detectLanguageFile(appPath) )
    {
        detectLanguageFile( APP_SYSFILES_PATH );
    }

    if ( !LoadCustomizationFile(appPath) )
    {
        LoadCustomizationFile( APP_SYSFILES_PATH );
    }

    srand( (int)getpid() );

    if (argc>1)
    {
        if ( checkParameters(argc,argv) )
        {
            delete app;
            return 0;
        }
    }

    // LoadAllRegisteredApp();

    DBG_OUT("Register Load DONE");

	zBaseGUI *gui = new zBaseGUI(NULL);
//    DBG_OUT("GUI Done (gui = %Xl , app = %Xl )", (uint)gui, (uint)app);
	app->setMainWidget(gui);
//	DBG_OUT("Set Main");
        gui->show();

	int ret=app->exec();
//int ret=0;
	delete gui;
	delete app;

    APPDB_Close();
	return ret;
}

bool LoadCustomizationFile(QString srcPath)
{
    QString fn_Custom = QString("%1/%2.custom").arg(srcPath).arg(appName);
    DBG_OUT("Checking for Customization file: %s", fn_Custom.ascii() );

	if (QFile::exists(fn_Custom))
	{
        DBG_OUT("Customization file found. Loading custom parameters");
        ZConfig* cstm_File = new ZConfig(fn_Custom);

        if (cstm_File->groupExists("UNPACK_TOOLS"))
        {

            FullPath_7z = cstm_File->readEntry("UNPACK_TOOLS","BIN_FULLPATH",APP_FULLPATH_7Z);
            DBG_OUT("UNPACK_TOOLS::BIN_FULLPATH : '%s'", FullPath_7z.ascii() );
        }

        if (cstm_File->groupExists("MENU_CFGS"))
        {
            appMenuUUID = cstm_File->readEntry("MENU_CFGS","APP_MENU_UUID",APP_FULLPATH_7Z);
            DBG_OUT("MENU_CFGS::APP_MENU_UUID : '%s'", appMenuUUID.ascii() );
        }
        delete cstm_File;
        return true;
    }
	return false;
}

QStringList* AM_GetUIDFromFile(QString pathFile,QStringList* sl = NULL)
{
    if (sl==NULL)
        sl = new QStringList();
    QFile file( pathFile );

    if (file.open(IO_ReadOnly))
    {
        QTextStream in(&file);
        while ( !in.atEnd() )
        {
            QString tmp = in.readLine();
            if ( tmp.startsWith("[") && tmp.length() == 38 )
            {
                tmp = tmp.replace("[", "");
                tmp = tmp.replace("]", "");
                sl->append(tmp);
                DBG_OUT("UID : %s",tmp.ascii() );
            }
        }
    }
    file.close();
    return sl;
}
void LoadAllRegisteredApp(bool ForceReload)
{
    if (!appRegList.isEmpty() && !ForceReload)
        return;

    appRegList.clear();
    QString appUID;
    QStringList* appUID_List = new QStringList();

    AM_AppRegistry* reg = new AM_AppRegistry();

    if (reg->init()!=0)
    {
        delete reg;
        DBG_OUT("Error Initializing AppRegistry Instance");
        return;
    }

    QValueList<const AM_AppObject* > list;

    //appUID_List = AM_GetUIDFromFile("/usr/data_resource/preloadapps/SysRegistry", appUID_List);
    //appUID_List = AM_GetUIDFromFile("/usr/SYSqtapp/am/SysRegistry" ,appUID_List);
    appUID_List = AM_GetUIDFromFile("/ezxlocal/download/appwrite/am/InstalledDB" , appUID_List);

    reg->getAllAppObjects(list);

    for ( uint i = 0; i<list.count(); ++i )
    {
        appUID = (list[i]->getAppId()).toString();
            appUID.replace("{","");
            appUID.replace("}","");
        if (!appUID_List->contains(appUID) )
            appUID_List->append(appUID);
    }

    AMRegEntry* re = NULL;

    for(QStringList::Iterator it = appUID_List->begin(); it!= appUID_List->end(); it++)
    {
        appUID = (*it);
        re = new AMRegEntry(appUID);
        if (re->valid)
            appRegList[appUID] = re;
        else
            delete re;
    }
    delete appUID_List;
}

void ListRegisteredAppsInMenu()
{
    LoadAllRegisteredApp();

    DBG_OUT("List Applications registered in Menu");
    DBG_OUT("[List START]");

    ZConfig* umt = new ZConfig("/ezxlocal/download/appwrite/am/UserMenuTree");
    QString appUID = QString::null;
    QString items = umt->readEntry(appMenuUUID,"Items","");
    QStringList itemsList = QStringList::split(";", items,false);
    delete umt;

    for (uint i = 0; i<itemsList.count();i++)
    {
        appUID = itemsList[i];
        if (appRegList.contains( appUID ) )
        {
            printf("\tAPP OBJECT{ Name: %s, Type: %d, Icon: %s}\n",
                appRegList[appUID]->Name.ascii(),
                appRegList[appUID]->eType,
                appRegList[appUID]->SimpleIcon.ascii() );
        }
    }
    DBG_OUT("[List END]");
}

void ListAppsInDB()
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;

    char sqlInsert[] = "SELECT UUID,Name,pathName,Location FROM AppReg;";

	if (sqlite3_prepare(AppDb, sqlInsert, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return;

    DBG_OUT("List Applications in DataBase");
    DBG_OUT("[List START]");
    while( sqlite3_step(hStmt) == SQLITE_ROW)
    {
        DBG_OUT("==========================================");
        DBG_OUT("UUID    : %s", sqlite3_column_text(hStmt,0) );
        DBG_OUT("AppName : %s", sqlite3_column_text(hStmt,1) );
        DBG_OUT("PathName: %s", sqlite3_column_text(hStmt,2) );
        DBG_OUT("Location: %d", sqlite3_column_int(hStmt,3) );
    }
    DBG_OUT("[List END]");
    sqlite3_finalize(hStmt);
}
void ListMenuAppsInDB()
{
    sqlite3_stmt* hStmt = NULL;
    const char** ptrDummy = 0;

    char sqlSelect[] = "SELECT UUID,Name,pathName,Location FROM AppReg WHERE id IN ( SELECT app_id FROM App_Menu);";
	if (sqlite3_prepare(AppDb, sqlSelect, -1, &hStmt, ptrDummy)!=SQLITE_OK)
        return;

    DBG_OUT("List Menu Applications in DataBase");
    DBG_OUT("[List START]");
    while( sqlite3_step(hStmt) == SQLITE_ROW)
    {
        DBG_OUT("==========================================");
        DBG_OUT("UUID    : %s", sqlite3_column_text(hStmt,0) );
        DBG_OUT("AppName : %s", sqlite3_column_text(hStmt,1) );
        DBG_OUT("PathName: %s", sqlite3_column_text(hStmt,2) );
        DBG_OUT("Location: %d", sqlite3_column_int(hStmt,3) );
    }
    DBG_OUT("[List END]");
    sqlite3_finalize(hStmt);
}

bool checkParameters(int argc, char** argv)
{
    DBG_OUT("Checking Parameters");

    for (int i = 1;i<argc;i++)
    {
        cout<<"\tParametro ["<<i<<"] = "<<argv[i]<<endl;
        if ( strcmp(argv[i],"--install")==0 )
        {
            DBG_OUT("Installer Mode");
            return true;
        }else if ( strcmp(argv[i],"--list-apps-menu-registered")==0 ){
            ListRegisteredAppsInMenu();
            return true;
        }else if ( strcmp(argv[i],"--list-apps-in-db")==0 ){
            ListAppsInDB();
            return true;
        }else if ( strcmp(argv[i],"--list-apps-menu-registered-in-db")==0 ){
            ListMenuAppsInDB();
            return true;
        }
    }

    QString fPath = argv[2];
    if (QFile::exists( fPath ) )
    {
        ZPopup* waitDlg = new ZPopup(ZPopup::NOTICE);
            waitDlg->setTitleIcon( "recur_enable_pop.bmp" );
        ZLabel* lbl = new ZLabel(lc("POP_PLEASEWAIT"), waitDlg, 0, 0, (ZSkinService::WidgetClsID)1);
            lbl->setAlignment(Qt::AlignHCenter);
            waitDlg->insertChild(lbl);
            waitDlg->setAutoDismissTime(1);
            waitDlg->hide();

        if ( fPath.endsWith(".mgx", false) )
        {

            DBG_OUT("MGX Installer Mode from Console");
            QString mgx_name = fPath.mid( fPath.findRev("/") +1 );

            if (!ConfirmAction( lc("POP_INSTALL_MGX_TITLE"), lc("POP_INSTALL_MGX_CONFIRM_TEXT") + " " + mgx_name ) )
                return true;
            //mount -o bind /ezxlocal/download/mystuff/swap /usr/local/Symbiosis/swap
            DBG_OUT("Install MGX (%s)",argv[2]);

            QStringList slLocations;
            slLocations+=lc("FT_PH_MEMORY");
            if ( sdExists )
                slLocations+=lc("FT_SD_MEMORY");

            ZSingleSelectDlg* pSSD = new ZSingleSelectDlg( lc("POP_INSTALL_MGX_TITLE"), lc("POP_INSTALL_MGX_LOCATION_TEXT"), NULL, "ZSingleSelectDlg", true, 0, 0);
                pSSD->addItemsList(slLocations);
            int iLoc = pSSD->exec();

            QString sCmd = QString("%2 x -y '%3'").arg( FullPath_7z ).arg( argv[2] );

            if (iLoc==0) return true;
            waitDlg->exec();
            waitDlg->show();

            QString sCmd2 = "";
            iLoc = pSSD->getCheckedItemIndex();
            if (iLoc==0)
            {
                sCmd.append(" -o").append(APP_FULLPATH_PH);
                sCmd2.append( QString("chmod +x -R %1/ ;").arg( APP_FULLPATH_PH ) );
            }else{
                sCmd.append(" -o").append(APP_FULLPATH_SD);
                sCmd2.append( QString("chmod +x -R %1/ ;").arg( APP_FULLPATH_SD ) );
            }
            sys_exec_cmd(sCmd);
            system( sCmd2.ascii() );

            waitDlg->hide();
            delete waitDlg;

            return true;
        }else if ( fPath.endsWith(".mgs",false) ) {
            DBG_OUT("MGS Installer Mode from Console");

            QString mgs_name = fPath.mid( fPath.findRev("/") +1 );

            if (!ConfirmAction( lc("POP_INSTALL_MGS_TITLE"), lc("POP_INSTALL_MGS_CONFIRM_TEXT") + " " + mgs_name ) )
                return true;
            //mount -o bind /ezxlocal/download/mystuff/swap /usr/local/Symbiosis/swap
            DBG_OUT("Install MGS (%s)",argv[2]);

            QStringList slLocations;
            slLocations+=lc("FT_PH_MEMORY");
            if ( sdExists )
                slLocations+=lc("FT_SD_MEMORY");

            ZSingleSelectDlg* pSSD = new ZSingleSelectDlg( lc("POP_INSTALL_MGS_TITLE"), lc("POP_INSTALL_MGS_LOCATION_TEXT"), NULL, "ZSingleSelectDlg", true, 0, 0);
                pSSD->addItemsList(slLocations);
            int iLoc = pSSD->exec();

            QString sCmd = QString("%2 x -y '%3'").arg( FullPath_7z ).arg( argv[2] );

            if (iLoc==0) return true;

            waitDlg->exec();
            waitDlg->show();

            iLoc = pSSD->getCheckedItemIndex();
            if (iLoc==0)
            {
                sCmd.append(" -o").append(APP_FULLPATH_PH_SKIN);
            }else{
                sCmd.append(" -o").append(APP_FULLPATH_SD_SKIN);
            }
            sys_exec_cmd(sCmd);

            waitDlg->hide();
            delete waitDlg;

            return true;
        }
    }
    return false;
}

bool detectLanguageFile(QString srcPath)
{
    // Short Name of currente LanguageType
	ZConfig* ezx_system = new ZConfig(EZX_SYSTEM_CFG);
	QString lng_Short = ezx_system->readEntry("SYS_SYSTEM_SET","LanguageType","en");
	delete ezx_system;

	DBG_OUT("Current LanguageType : '%s'",lng_Short.ascii() );

	// Join parts to form language file full path
	QString lngPath = QString("%1/%2_%3.lng").arg(srcPath).arg(appName).arg(lng_Short);
	DBG_OUT("Checking for lng file : '%s'", lngPath.ascii() );

	// Check for file
	if (QFile::exists(lngPath))
	{
		cLng = new ZConfig(lngPath);
		return true;
	}else if (lng_Short.length() > 2) {
	    lngPath = QString("%1/%2_%3.lng").arg(srcPath).arg(appName).arg( lng_Short.left(2) );
	    DBG_OUT("Checking for lng file : '%s'", lngPath.ascii() );
        if (QFile::exists(lngPath))
        {
            cLng = new ZConfig(lngPath);
            return true;
        }
	}
	DBG_OUT("The lng file is missing. Trying to use English lng file as default.");
	lngPath = QString("%1/%2_en.lng").arg(srcPath).arg(appName);

	if (!QFile::exists(lngPath) & cLng !=NULL )
		return false;

	if (cLng!=NULL)
		delete cLng;
	cLng = new ZConfig(lngPath);
	return false;
}
bool dirCreateCheck(char* path)
{
    DIR* d;
    d = opendir( path );
    if (d==NULL)
    {
        DBG_OUT( "Dir '%s' doesn't exist. Try to create it", path );
        if ( mkdir( path, 777 )!= 0 )
        {
            DBG_OUT( "[ERROR] : Directory creation Failed." );
            return false;
        }
    }else{
        closedir(d);
    }
    return true;
}
void checkDirectories()
{
	struct mntent *mnt;
	FILE *fp;

	fp = setmntent (MOUNTED, "r");
	if (fp != NULL)
    {
        while ( mnt = getmntent (fp) )
        {
            if ( strstr(mnt->mnt_dir,"/mmc/mmca1")!=NULL)
            {
                DBG_OUT( "SD exists" );
                sdExists = true;
                break;
            }
        }
        if (!sdExists)
        {
            DBG_OUT( "SD doesn't exist" );
        }
        endmntent(fp);
    }else{
        DBG_OUT( "Cannot determine if SD is exists. Disabled by default" );
    }

    if ( sdExists )
    {
        if ( !dirCreateCheck( APP_FULLPATH_SD ) )
        {
            DBG_OUT( "SD Directory creation Failed" );
        }
        if ( !dirCreateCheck( APP_FULLPATH_SD_SKIN ) )
        {
            DBG_OUT( "SD Skins Directory creation Failed" );
        }
    }
    if ( !dirCreateCheck( APP_FULLPATH_PH ) )
    {
        DBG_OUT( "Phone Directory creation Failed" );
    }
    if ( !dirCreateCheck( APP_FULLPATH_PH_SKIN ) )
    {
        DBG_OUT( "Phone Skins Directory creation Failed" );
    }
    if ( !dirCreateCheck( APP_SYSFILES_PATH ) )
    {
        DBG_OUT( "Application Sys Files Directory creation Failed" );
    }
}

/*
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    NotUsed=0;
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i]: "NULL");
    }
    printf("\n");
    return 0;
}

int main(int argc, char **argv){
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    if( argc!=3 ){
        fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
        exit(1);
    }
    rc = sqlite3_open(argv[1], &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);

    if (zErrMsg)
        free(zErrMsg);
    }
    sqlite3_close(db);
    return 0;
}
*/
