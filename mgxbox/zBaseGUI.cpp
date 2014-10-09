#include "MgxBox.h"

#include "zBaseGUI.h"

#include <iostream>
using namespace std;
extern bool NeedRefreshMenu;
extern bool sdExists;
extern QString appName;
extern QString appPath;
extern QString FullPath_7z;
extern QString appMenuUUID;
extern ZConfig* cLng;

extern void APPDB_AddApp(QUuid* appUUID,const char* aName,const char* aPath, int loc);
extern bool APPDB_RemoveApp(QUuid* appUUID);
extern bool APPDB_UpdateAppInfo(QUuid* appUUID,const char* aName,const char* aPath, int loc);

extern bool APPDB_isAppRegisteredInDB(const char* aPath,QUuid* appUUID);
extern bool APPDB_isAppMenuRegisteredInDB(const char* aPath,QUuid* appUUID);

extern bool APPDB_isUUIDusedDB(QUuid* appUUID);
extern bool APPDB_RegisterAppInMenu(const char* aPath,QUuid* appUUID, QString menuUUID);
extern bool APPDB_UnregisterAppFromMenu(QUuid* appUUID);

extern bool APPDB_getAppUUID(const char* aPath,char* ptrUUID);
extern bool APPDB_getMenuUUIDFromAppId(QUuid* appUUID,char* menuUUID);

QString lc(const char *key);

MGXList mgxlist;

/*  BEGIN OF MGX_App Code */

class MGX_App
{
public:
    MGX_App(){};
    ~MGX_App(){};

    QString Name;
    QString Author;
    QString Version;
    QString Exec;
    QString Icon;
    QString Path;
    QString Comment;
    QString menuUUID;

    QString getIconPath()
    {
        return Path + "/" + Icon;
    };

    QString getKeyName()
    {
        return Name + ( (location==LOCATION_PH)?"_PH":"_SD" );
    }
    QString getDirName()
    {
        return Path.mid( Path.findRev("/") + 1 );
    }
    QString getUUID_Striped()
    {
        return UUID->toString().replace("{","").replace("}","");
    }
    enum MGXAPP_LOCATION {
        LOCATION_PH,
        LOCATION_SD
    };

    int lenght;
    bool Registered;
    MGXAPP_LOCATION location;
    QUuid* UUID;
};

/*  END OF MGX_App Code */

char* fileReadAll(const char* name,uint* ptrSize = NULL)
{
    FILE* pFile = NULL;
    char* buffer = NULL;
    uint lSize = 0;

    pFile = fopen( name ,"r");
    if (pFile!=NULL)
    {
        // obtain file size
        fseek (pFile , 0 , SEEK_END);
        lSize = ftell (pFile);
        rewind (pFile);

        // allocate memory to contain the whole file
        buffer = (char*) malloc(sizeof(char)*lSize);
        if (buffer == NULL)
        {
            DBG_OUT("Error while allocating memory for file");
        }else{
            // copy the file into the buffer:
            uint result = fread (buffer,1,lSize,pFile);
            if (result != lSize)
            {
                DBG_OUT("Error while reading file (%s)", name );
            }
        }
        fclose (pFile);
    }else{
        DBG_OUT("Error while opening file (%s)", name );
    }
    if (ptrSize)
        *ptrSize = lSize;

    return buffer;
}

/*  Foreign Code for reading/writing cfg files     */
void readconfig(const char* buf, const char *key, char *dest)
{
	char *mystr=strdup(buf);
	memset(dest, 0, sizeof(dest));
	char *p=strtok(mystr, "\n");
	while (p!=NULL) {
		if (strncmp(p, key, strlen(key))==0) {
			strcpy(dest, p+strlen(key));
			break;
		}
		memset(p, 0, sizeof(p));
		p=strtok(NULL, "\n");
	}
	free(mystr);
}

void setconfig(const char *buf, const char *key, const char *value, char *dest)
{
	char *mystr=strdup(buf);
	char temp[512];
	memset(dest, 0, sizeof(dest));
	char *p=strtok(mystr, "\n");
	while (p!=NULL) {
		sprintf(temp, "%s\n", p);
		if (strncmp(temp, key, strlen(key))==0) {
			sprintf(temp, "%s = %s\n", key, value);
		}
		strcat(dest, temp);
		memset(p, 0, sizeof(p));
		p=strtok(NULL, "\n");
	}
	free(mystr);
}

/*  End of Foreign Code    */

QStringList getFiles(bool OnlyDirs, bool OnlyFiles, bool AllFiles, bool Sort)
{
    QStringList result;

    DIR           *d;
    struct dirent *dir;

    d = opendir( "." );
    if( d == NULL ) {
        return result;
    }

    while( ( dir = readdir( d ) ) )
    {
        if( strcmp( dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 )
        {
            continue;
        }

        if (OnlyDirs)
        {
            if ( dir->d_type == DT_DIR )
                result.append( dir->d_name );
        }else{
            if (strstr(dir->d_name,".mgx")!=NULL || AllFiles && (OnlyFiles && dir->d_type!= DT_DIR))
                result.append( dir->d_name );
        }
    }
    if (Sort)
        result.sort();
    closedir( d );
    return result;
}
bool ConfirmAction(QString title, QString text)
{
	ZMessageDlg *confirmDlg= new ZMessageDlg(title, text, ZMessageDlg::TypeConfirm );
	int ret = confirmDlg->exec();
	delete confirmDlg;

	if (ret!=1) return false;
	return true;
}

bool sys_exec_cmd(QString cmd,bool inMultiOper)
{
    bool cmd_ok = false;
    if (cmd.isEmpty() || cmd.isNull() )
        return false;

	DBG_OUT("Exec command ( %s )", cmd.ascii() );
	cmd.append(" > /tmp/mgxbox_exec.log 2>&1");

	int ret = system(cmd.ascii() );
	ret>>=8;
	DBG_OUT("Command executed returned %d", ret );

	if ( WIFEXITED(ret) )
	{
	    if (inMultiOper )
            return true;

		ZMessageDlg *finishDlg = new ZMessageDlg( lc("POP_OK_TITLE"), lc("POP_OK_TEXT"), ZMessageDlg::TypeOK);
		finishDlg->exec();
		delete finishDlg;
		cmd_ok = true;
    }else{
        QString logString;
        char* buffer=NULL;
        uint lSize;
        size_t result;
        FILE* pFile=NULL;

        pFile = fopen( "/tmp/mgxbox_exec.log" ,"r");
        if (pFile!=NULL)
        {
            // obtain file size:
            fseek (pFile , 0 , SEEK_END);
            lSize = ftell (pFile);
            rewind (pFile);

            // allocate memory to contain the whole file:
            buffer = (char*) malloc(sizeof(char)*lSize);
            if (buffer == NULL)
            {
                DBG_OUT("Error while allocating memory for log file");
            }else{
                // copy the file into the buffer:
                result = fread (buffer,1,lSize,pFile);
                if (result != (size_t)lSize)
                {
                    DBG_OUT("Error while reading log file" );
                }else{
                    logString = buffer;
                }
                free(buffer);
            }
        }

		ZMessageDlg *aboutDlg = new ZMessageDlg(lc("POP_ERROR_TITLE"), logString, (ZMessageDlg::MessageDlgType)2);
            aboutDlg->setTitleIcon("error_pop.bmp");
            aboutDlg->exec();
		delete aboutDlg;
	}
	system("cat /tmp/mgxbox_exec.log");
	QFile::remove("/tmp/mgxbox_exec.log");

    return cmd_ok ;
}

//  /usr/local/Reloaded/p7zip
MGX_App* getMGXFromDir(QString path)
{
    QString cfgName;
    MGX_App* mgx_obj = NULL;
    char* buffer;

    char prev_path[MAXPATHLEN] = {0};
    char value[300] = {0};

    if (path.isEmpty())
        return (MGX_App*)NULL;

    getcwd( prev_path, MAXPATHLEN );
    
    DBG_OUT("prev dir : %s",prev_path);
    DBG_OUT("path dir : %s",path.latin1());
    chdir( path.ascii() );
    cfgName = path;
    cfgName.append(".cfg");

    buffer = fileReadAll( cfgName.ascii() );
    if (buffer!=NULL)
    {
        mgx_obj = new MGX_App();
        mgx_obj->Path = QString(prev_path) + "/" + path;

        readconfig(buffer,"Name = ",value);
        mgx_obj->Name = zBaseGUI::ctr(value);

        readconfig(buffer,"Author = ",value);
        mgx_obj->Author = zBaseGUI::ctr(value);

        readconfig(buffer,"Version = ",value);
        mgx_obj->Version = zBaseGUI::ctr(value);

        readconfig(buffer,"Exec = ",value);
        mgx_obj->Exec = zBaseGUI::ctr(value);

        readconfig(buffer,"Icon = ",value);
        mgx_obj->Icon = zBaseGUI::ctr(value);

        readconfig(buffer,"Comment = ",value);
        mgx_obj->Comment = zBaseGUI::ctr(value);

        mgx_obj->Registered = false;

        free (buffer);
    }
    chdir( prev_path );
    return mgx_obj;
}

void MGXCheckDBStatus(MGX_App* mgx_obj)
{
    QUuid* appUUID = NULL;
    char cUUID[39] = {0};

    if(!APPDB_isAppRegisteredInDB(mgx_obj->Path.ascii(), NULL))
    {
        do{
            appUUID = new  QUuid( 0x8e10aded, 0xf8ee, rand(), rand(), rand(), rand(), rand(),rand(), rand(), rand(),rand() );
        }while( APPDB_isUUIDusedDB(appUUID) );

        sprintf(cUUID,"%s", appUUID->toString().ascii() );
        DBG_OUT("Adding APP to AppDB ('%s')\n\tApp UUID is %s",mgx_obj->Name.ascii(), cUUID );
        APPDB_AddApp( appUUID, mgx_obj->Name.ascii(), mgx_obj->Path.ascii(), mgx_obj->location);
    }else{
        APPDB_getAppUUID(mgx_obj->Path.ascii(), cUUID);
        DBG_OUT("Application '%s' is already in DB.\n\tApp UUID is %s",mgx_obj->Name.ascii(), cUUID);
        appUUID = new QUuid(cUUID);
    }
    mgx_obj->UUID = appUUID;

    if (APPDB_isAppMenuRegisteredInDB(mgx_obj->Path.ascii(), NULL))
        mgx_obj->Registered = true;

    return;
}
void UpdateMGXList()
{
    DIR* d;
    char prev_path[MAXPATHLEN] = {0};
    MGX_App* mgx_obj = NULL;

    mgxlist.clear();

    // Save Working Directory
    getcwd( prev_path, MAXPATHLEN );

    if (sdExists)
    {
        d = opendir( APP_FULLPATH_SD );
        if (d!=NULL)
        {
            closedir(d);
            chdir( APP_FULLPATH_SD );
            int index = 0;
            QStringList dirs = getFiles(true,false,false,true);
            for ( QStringList::Iterator it = dirs.begin(); it!= dirs.end(); it++)
            {
    //            cout<<" Dir : "<<*it<<endl;
                mgx_obj = getMGXFromDir(*it);
                if (mgx_obj==NULL)
                    continue;
                mgx_obj->location = MGX_App::LOCATION_SD;
                MGXCheckDBStatus( mgx_obj );
                mgxlist[ mgx_obj->getKeyName() ] = mgx_obj;
                index++;
            }
            DBG_OUT("File in mem card is : %d\n",index);
        }
    }

    d = opendir( APP_FULLPATH_PH );
    if (d!=NULL)
    {
        closedir(d);

        chdir( APP_FULLPATH_PH );
        int index = 0;
        QStringList dirs = getFiles(true,false,false,true);
        for ( QStringList::Iterator it = dirs.begin(); it!= dirs.end(); it++)
        {
//            cout<<" Dir : "<<*it<<endl;
            mgx_obj = getMGXFromDir(*it);
            if (mgx_obj==NULL)
                continue;
            mgx_obj->location = MGX_App::LOCATION_PH;
            MGXCheckDBStatus(mgx_obj);
            mgxlist[ mgx_obj->getKeyName() ] = mgx_obj;
            index++;
        }
        DBG_OUT("File in phone memory is : %d\n",index);

    }
    // Restore Working Directory
    chdir( prev_path );
    DBG_OUT("Restored working directory\n");
    return;
}

QString lc(const char *key)
{
    if (cLng==NULL)
        return QString(key);
    return cLng->readEntry("WORDS",key,key);
}

bool RegisterAppInMenu(MGX_App* mgx_obj)
{
    printf("Adding to main menu\n");
    MenuSelectDlg *dlg = new MenuSelectDlg("Select Folder","Select","Cancel");
    if(dlg->exec())
    {
        QString uuid = dlg->getUUID();
        mgx_obj->menuUUID = uuid;
        printf("Selected folder is : %s\n",uuid.latin1());  
        
        QString tUUID = mgx_obj->getUUID_Striped();

        /*  Add Section in InstalledDB */
        ZConfig* insDB = new ZConfig("/ezxlocal/download/appwrite/am/InstalledDB");
            insDB->writeEntry(tUUID,"AppID","{" + tUUID + "}" );
            insDB->writeEntry(tUUID,"AppType",0);
            insDB->writeEntry(tUUID,"ObjectType",2);
            insDB->writeEntry(tUUID,"Attribute",1);
            insDB->writeEntry(tUUID,"Args","");
            insDB->writeEntry(tUUID,"Daemon",0);
            insDB->writeEntry(tUUID,"Directory",mgx_obj->Path);
            insDB->writeEntry(tUUID,"ExecId",mgx_obj->Exec);
            insDB->writeEntry(tUUID,"Name",mgx_obj->Name);
            insDB->writeEntry(tUUID,"LockEnabled ",1);
            insDB->writeEntry(tUUID,"AniIcon",mgx_obj->getIconPath());
            insDB->writeEntry(tUUID,"BigIcon",mgx_obj->getIconPath());
            insDB->writeEntry(tUUID,"Icon",mgx_obj->getIconPath());
            insDB->writeEntry(tUUID,"SVGIcon",mgx_obj->getIconPath());
            insDB->writeEntry(tUUID,"GroupID","root");
            insDB->writeEntry(tUUID,"UserID","root");
            insDB->writeEntry(tUUID,"Vendor",mgx_obj->Author);
            insDB->flush();
        delete insDB;

        /*  Register in userMenuTree    */
        ZConfig* umt = new ZConfig("/ezxlocal/download/appwrite/am/UserMenuTree");
        QString items = umt->readEntry(uuid,"Items","");
        unsigned int colonIdx = items.findRev(";",-1,false);
        if(colonIdx  == items.length() -1){
            printf("Nofix ;\n");
            umt->writeEntry(uuid,"Items", items + tUUID + ";");
        } else {
            printf("Prefixing ;\n");
            umt->writeEntry(uuid,"Items",QString(items + ";" + tUUID + ";") );
        }
        umt->flush();
        delete umt;

        mgx_obj->Registered=true;

        /*  Register in DB  */
        APPDB_RegisterAppInMenu(mgx_obj->Path.ascii(), mgx_obj->UUID, mgx_obj->menuUUID );

        NeedRefreshMenu = true;         
        delete dlg;
    } else {
        return false;
    }
    
    return true;
}
bool UnregisterAppFromMenu(MGX_App* mgx_obj)
{
    DBG_OUT("Unregister App ('%s') from Menu\n\tApp UUID is '%s'", mgx_obj->Path.ascii(), mgx_obj->UUID->toString().ascii() );

    QString tUUID = mgx_obj->getUUID_Striped();

    /*  Remove Section in InstalledDB */
    ZConfig* insDB = new ZConfig("/ezxlocal/download/appwrite/am/InstalledDB");
        insDB->clearGroup(tUUID);
        insDB->flush();
    delete insDB;
    
    char cUUID[39]= {0};
    APPDB_getMenuUUIDFromAppId(mgx_obj->UUID,cUUID);
    printf("Menu UUID for App %s\n",cUUID);
    printf("Menu UUID for App %s\n",tUUID.latin1());

    /*  Remove Link from UserMenuTree */
    ZConfig* umt = new ZConfig("/ezxlocal/download/appwrite/am/UserMenuTree");
    QString items = umt->readEntry(QString(cUUID),"Items","");
    items = items.replace(QString(tUUID+";"),"",false);
        umt->writeEntry(cUUID,"Items", items);
        umt->flush();
    delete umt;

    mgx_obj->Registered=false;

    /*  Unregister from DB */
    APPDB_UnregisterAppFromMenu( mgx_obj->UUID );

    NeedRefreshMenu = true;
    return true;
}
bool UpdateAppInfoInMenu(MGX_App* mgx_obj)
{
    QString tUUID = mgx_obj->getUUID_Striped();
    DBG_OUT("Update App Info ('%s') in Menu\n\tApp UUID is '%s'", mgx_obj->Path.ascii(), mgx_obj->UUID->toString().ascii() );
    /*  Add Section in InstalledDB */
    ZConfig* insDB = new ZConfig("/ezxlocal/download/appwrite/am/InstalledDB");
        insDB->writeEntry(tUUID,"Directory",mgx_obj->Path);
        insDB->writeEntry(tUUID,"Exec",mgx_obj->Exec);
        insDB->writeEntry(tUUID,"Name",mgx_obj->Name);
        insDB->writeEntry(tUUID,"AniIcon",mgx_obj->getIconPath());
        insDB->writeEntry(tUUID,"BigIcon",mgx_obj->getIconPath());
        insDB->writeEntry(tUUID,"Icon",mgx_obj->getIconPath());
        insDB->writeEntry(tUUID,"SVGIcon",mgx_obj->getIconPath());
        insDB->flush();
    delete insDB;

    NeedRefreshMenu = true;

    return true;
}
QTextCodec* zBaseGUI::gbkstr = NULL;

QStringList* SplitTextToLines(QString strText,int MaxWidth, QFont fnt)
{
    QString txt = strText;
    QStringList* strList_R = new QStringList();

    QStringList strList;
	QFontMetrics fm( fnt );

    while(fm.width(txt)>MaxWidth)
    {
        strList = strList.split(" ",txt);
        txt = "";
//        DBG_OUT("Cantidad de Subcadenas : %d", strList.count() );

        while( ( fm.width( txt + " " + strList.first() )<MaxWidth ) && (strList.count() >0) )
        {
//            DBG_OUT("Probando con cadena : %s", strList.first().ascii() );
            txt+=" " + strList.first() ;
            strList.remove( strList.first() );
        };

        strList_R->append(txt);

        txt = strList.join(" ");
    };
    strList_R->append(txt);
    return strList_R;
}

zBaseGUI::zBaseGUI(QWidget* parent)
{
    Q_UNUSED(parent);
    DBG_OUT("zBaseGUI Contructor");
    // Create QTextCodec objecto for encoding non-ANSI strings
    gbkstr = QTextCodec::codecForName("UTF-8");

    // Initialize all internal object references to NULL
    softkey = NULL;
    optm_Main = NULL;
    optm_MainMultiSelect = NULL;
    optm_MainSaveAsMGX = NULL;
    optm_MainView = NULL;
    optm_Mgx = NULL;
    optm_Fav = NULL;

    sdphMultiSelectMode = false;

	waitDlg = new ZPopup(ZPopup::NOTICE);
        waitDlg->setTitleIcon( "recur_enable_pop.bmp" );
	ZLabel* lbl = new ZLabel(lc("POP_PLEASEWAIT"), waitDlg, 0, 0, (ZSkinService::WidgetClsID)1);
        lbl->setAlignment(Qt::AlignHCenter);
        waitDlg->insertChild(lbl);
        waitDlg->setAutoDismissTime(1);
        waitDlg->hide();

    CreateControls(parent);

    QTimer::singleShot(10,this,SLOT( firstRefresh() ) );

    DBG_OUT("zBaseGUI Contructor - End");
    return;
}
zBaseGUI::~zBaseGUI()
{
    DBG_OUT("zBaseGUI Destructor");
    delete gbkstr;
}
void zBaseGUI::firstRefresh()
{
    if (sdExists)
        FillMgxExplorer(APP_FULLPATH_ROOT_SD);
    else
        FillMgxExplorer(APP_FULLPATH_ROOT_PH);

    FillMgxManager(true);
}

void zBaseGUI::CreateControls(QWidget* parent)
{
    DBG_OUT("Creating controls");
    Q_UNUSED(parent);
    
    DBG_OUT("Set title");
    setMainWidgetTitle(ctr(APP_TITLE));
    //setMainWidgetTitleIcon( iconReader.getIcon("certificate_mgr_std.g") );

    //QVBoxLayout* layout = getVBoxLayout();

	znvtw = new ZNavTabWidget(this, 0, (ZSkinService::WidgetClsID)63);

	ZSetLayout(znvtw, ZGlobal::getContentR());

	znvtw->stopNextWhenKeyRepeat(FALSE);

    //layout->addWidget(znvtw);

	zlb_sdph = new ZListBox("%I%M", znvtw);
	zlb_sdph->setAutoExpandInNestMode(true);

	znvtw->addTab(zlb_sdph, iconReader.getIcon("fm_small.bmp"), tr("all"));

    zlb_mgx = new ZListBox("%I%M", znvtw);
	zlb_mgx->setAutoExpandInNestMode(true);

	znvtw->addTab(zlb_mgx, iconReader.getIcon("fm_folder_small.bmp"), tr("mgx"));
/*
    zlb_fav = new ZListBox("%R%M170%I", znvtw);
	zlb_fav->setAutoExpandInNestMode(true);

	znvtw->addTab(zlb_fav, iconReader.getIcon("fm_memory_card_tab.bmp"), tr("favorites"));
*/

    connect(znvtw, SIGNAL(selected( const QString& ) ), this, SLOT(tabSelected( const QString& ) ) );

    connect(zlb_sdph, SIGNAL(selected( int ) ), this, SLOT(app_selected( int ) ) );
    
    connect(zlb_mgx, SIGNAL(highlighted( int ) ), this, SLOT(mgx_highlighted( int ) ) );
    connect(zlb_mgx, SIGNAL(selected( int ) ), this, SLOT(mgx_selected( int ) ) );

//    FillMountPointsList();

	setSoftKey( getSoftKey() );

    DBG_OUT("Creating controls -END");
    return;
}

ZSoftKey* zBaseGUI::getSoftKey()
{
    if (softkey!=NULL)
        return softkey;

	softkey = new ZSoftKey(tr("CST_2A"), this, this);
	ZSetLayout(softkey, ZGlobal::getCstR());

	softkey->setText(ZSoftKey::LEFT, lc("SK_OPT"), ZSoftKey::TEXT_PRIORITY(0));
	softkey->setText(ZSoftKey::RIGHT, lc("SK_EXIT"), ZSoftKey::TEXT_PRIORITY(0));

	softkey->setTextForOptMenuHide(lc("SK_OPT"));
	softkey->setTextForOptMenuShow(lc("SK_SEL"), lc("SK_CANCEL"));

    softkey->setOptMenu(ZSoftKey::LEFT, getOptionsMenu() );

	softkey->setClickedSlot(ZSoftKey::RIGHT, this, SLOT(quit()));

    return softkey;
}

void zBaseGUI::quit()
{
    DBG_OUT("SoftKey Quit");
    if (sdphMultiSelectMode)
    {
        mainOptM_MultiSelect();
    }else{
        if (NeedRefreshMenu)
        {
            if( ConfirmAction(lc("POP_CONFIRM_TITLE"), lc("POP_CONFIRM_REFRESH_MENU") ) )
            {
                system("busybox killall am flowplayer messaging");
            }
        }
        qApp->quit();
    }
}

ZOptionsMenu* zBaseGUI::getOptionsMenu()
{
//    cout<<"DEBUG:: getOptionsMenu() Tabindex : "<<znvtw->currentPageIndex()<<endl;
    if (znvtw->currentPageIndex()==1)
        return getMgxOptionsMenu();
    return getMainOptionsMenu();
}

ZOptionsMenu* zBaseGUI::getMainOptionsMenu()
{
    if (optm_Main==NULL)
    {
        QRect rect;
        optm_Main = new ZOptionsMenu(rect, getSoftKey() , 0,0, ZSkinService::WidgetClsID(55));
        optm_Main->insertItem( lc("APP_MOVE"), NULL, NULL, true, 0, 0 );
        optm_Main->insertItem( lc("APP_RENAME"), NULL, NULL, true, 1, 1 );
        optm_Main->insertItem( lc("APP_DELETE"), NULL, NULL, true, 2, 2 );
        optm_Main->insertItem( lc("APP_PROPERTIES"), NULL, NULL, true, 3, 3 );
        optm_Main->insertItem( lc("APP_UN_REGISTER_APP_MENU"), NULL, NULL, true, 4, 4 );
        optm_Main->insertItem( lc("APP_SAVE_AS_MGX_TO"), getMainSaveAsMGXSubOptionsMenu(), NULL, true, 5, 5 );
        optm_Main->insertItem( lc("APP_VIEW"), getMainViewSubOptionsMenu(), NULL, true, 6, 6 );
        optm_Main->insertItem( lc("APP_MULTI_SELECT"), NULL, NULL, true, 7, 7 );
        optm_Main->insertItem( lc("APP_REFRESH"), NULL, NULL, true, 8, 8 );
        optm_Main->insertItem( lc("APP_ABOUT"), NULL, NULL, true, 9, 9 );

        optm_Main->connectItem(0, this, SLOT( mainOptM_Move() ) );
        optm_Main->connectItem(1, this, SLOT( mainOptM_Rename() ) );
        optm_Main->connectItem(2, this, SLOT( mainOptM_Delete() ) );
        optm_Main->connectItem(3, this, SLOT( mainOptM_Properties() ) );
        optm_Main->connectItem(4, this, SLOT( mainOptM_RegisterAppMenu() ) );

        optm_Main->connectItem(7, this, SLOT( mainOptM_MultiSelect() ) );
        optm_Main->connectItem(8, this, SLOT( mainOptM_Refresh() ) );
        optm_Main->connectItem(9, this, SLOT( OptM_About() ) );

    }
    return optm_Main;
}

ZOptionsMenu* zBaseGUI::getMainMultiSelectOptionsMenu()
{
    if (optm_MainMultiSelect==NULL)
    {
        QRect rect;
        optm_MainMultiSelect = new ZOptionsMenu(rect, this, 0,0, ZSkinService::WidgetClsID(55));
        optm_MainMultiSelect->insertItem( lc("APP_MOVE"), NULL, NULL, sdExists, 0, 0 ); // Only Move if SD exists
        optm_MainMultiSelect->insertItem( lc("APP_DELETE"), NULL, NULL, true, 1, 1 );
        optm_MainMultiSelect->insertItem( lc("APP_UN_REGISTER_APP_MENU"), NULL, NULL, false, 2, 2 );
        optm_MainMultiSelect->insertItem( lc("APP_SELECT_ALL"), NULL, NULL, true, 3, 3 );
        optm_MainMultiSelect->insertItem( lc("APP_UNSELECT_ALL"), NULL, NULL, true, 4, 4 );
        optm_MainMultiSelect->insertItem( lc("APP_CANCEL"), NULL, NULL, true, 5, 5 );

        //optm_MainMultiSelect->insertItem( lc("APP_SAVE_IN_FM_LOC"), NULL, NULL, true, 2, 2 );

        optm_MainMultiSelect->connectItem(0, this, SLOT( mainOptMMS_Move() ) );
        optm_MainMultiSelect->connectItem(1, this, SLOT( mainOptMMS_Delete() ) );
        optm_MainMultiSelect->connectItem(3, this, SLOT( mainOptMMS_SelectAll() ) );
        optm_MainMultiSelect->connectItem(4, this, SLOT( mainOptMMS_UnselectAll() ) );
        optm_MainMultiSelect->connectItem(5, this, SLOT( mainOptM_MultiSelect() ) );
    }
    return optm_MainMultiSelect;
}

ZOptionsMenu* zBaseGUI::getMainSaveAsMGXSubOptionsMenu()
{
    if (optm_MainSaveAsMGX==NULL)
    {
        QRect rect;
        optm_MainSaveAsMGX = new ZOptionsMenu(rect, this,  0,0, ZSkinService::WidgetClsID(55));
        optm_MainSaveAsMGX->insertItem( lc("APP_SAVE_IN_SD"), NULL, NULL, sdExists, 0, 0 );
        optm_MainSaveAsMGX->insertItem( lc("APP_SAVE_IN_PH"), NULL, NULL, true, 1, 1 );
        optm_MainSaveAsMGX->insertItem( lc("APP_SAVE_IN_FM_LOC"), NULL, NULL, true, 2, 2 );

        optm_MainSaveAsMGX->connectItem(0, this, SLOT( mainSubOptM_SaveAs_SD() ) );
        optm_MainSaveAsMGX->connectItem(1, this, SLOT( mainSubOptM_SaveAs_PH() ) );
        optm_MainSaveAsMGX->connectItem(2, this, SLOT( mainSubOptM_SaveAs_FM() ) );

    }
    return optm_MainSaveAsMGX;
}
ZOptionsMenu* zBaseGUI::getMainViewSubOptionsMenu()
{
    if (optm_MainView==NULL)
    {
        QRect rect;
        optm_MainView = new ZOptionsMenu(rect, this,  0,0, ZSkinService::WidgetClsID(55));

        optm_MainView->insertItem( lc("APP_APPS_IN_SD"), NULL, NULL, NULL, sdExists, true, true, 0, 0 );
        optm_MainView->insertItem( lc("APP_APPS_IN_PH"), NULL, NULL, NULL, true, true, true, 1, 1 );
        //optm_MainView->insertItem( lc("APP_ONLY_REGISTERED_APPS"), NULL, NULL, NULL, true, true, false, 2, 2 );

        optm_MainView->connectItem(0, this, SLOT( mainSubOptM_ViewSDPH() ) );
        optm_MainView->connectItem(1, this, SLOT( mainSubOptM_ViewSDPH() ) );
        //optm_MainView->connectItem(2, this, SLOT( mainSubOptM_ViewRegistered() ) );

//        optm_MainView->connectItem(7, this, SLOT( OptM_About() ) );
    }
    return optm_MainView;
}
ZOptionsMenu* zBaseGUI::getMgxOptionsMenu()
{
    if (optm_Mgx==NULL)
    {
        QRect rect;
        optm_Mgx = new ZOptionsMenu(rect, getSoftKey() ,  0,0, ZSkinService::WidgetClsID(55));
        optm_Mgx->insertItem( lc("MGX_INSTALL"), NULL, NULL, true, 0, 0 );
        optm_Mgx->insertItem( lc("APP_SHOW_ALL"), NULL, NULL, NULL, true, true, false, 1, 1 );
        optm_Mgx->insertItem( lc("APP_GO_MEMORY_CARD"), NULL, NULL, sdExists, 2, 2 );
        optm_Mgx->insertItem( lc("APP_GO_PHONE_MEMORY"), NULL, NULL, true, 3, 3 );
        optm_Mgx->insertItem( lc("APP_ABOUT"), NULL, NULL, true, 4, 4 );

        optm_Mgx->connectItem(0, this, SLOT( mgxOptM_Install() ) );
        optm_Mgx->connectItem(1, this, SLOT( mgxOptM_ShowAll() ) );
        optm_Mgx->connectItem(2, this, SLOT( mgxOptM_GoToSD() ) );
        optm_Mgx->connectItem(3, this, SLOT( mgxOptM_GoToPH() ) );
        optm_Mgx->connectItem(4, this, SLOT( OptM_About() ) );

        //optm_Main->insertItem( lc("Menu 5"), NULL, NULL, NULL, true, true, false, 4, 4 );
    }
    return optm_Mgx;
}

void zBaseGUI::showWaitDlg(bool visible)
{
    if (visible)
    {
        waitDlg->exec();
        waitDlg->show();
    }else{
        waitDlg->hide();
    }
}

bool zBaseGUI::FillMgxExplorer(QString cdir)
{
    struct stat filestatus;
    char result[MAXPATHLEN] = {0};

    if (!cdir.isEmpty())
        chdir( cdir.ascii() );

    zlb_mgx->clear();
    setMainWidgetTitle( lc("TXT_LOADING") );

    QStringList dirs = getFiles(true,false,false,true);
    QStringList files = getFiles(false,true, getMgxOptionsMenu()->isItemChecked(1) ,true);

    setMainWidgetTitle( lc("TXT_REFRESHING") );

    getcwd( result, MAXPATHLEN );

    ZListBoxItem* zlbi = new ZListBoxItem(zlb_mgx);
        zlbi->appendSubItem(1, ctr(result) );
        zlbi->setEnabled(false);
    zlb_mgx->insertItem(zlbi,0,true);

    QPixmap pm = iconReader.getIcon("gen_back_to_arrw_small.bmp");

    zlbi = new ZListBoxItem(zlb_mgx);
        zlbi->appendSubItem(1, ".." , false, &pm);
    zlb_mgx->insertItem(zlbi,1,true);

    for ( QStringList::Iterator it = dirs.begin(); it!= dirs.end(); it++)
    {
        stat( *it, &filestatus );

        ZListBoxItem* zlbi = new ZListBoxItem( zlb_mgx );
            zlbi->appendSubItem(1,ctr(*it));
            zlbi->setPixmap(0,iconReader.getIcon("fm_folder_small.bmp"));
/*
        if (filestatus.st_size<1024)
            sprintf(result,"%ld bytes",filestatus.st_size);
        else
            sprintf(result,"%.2f Kbytes",(double)filestatus.st_size/1024.0);
        zlbi->appendSubItem(1,result);
*/
        zlbi->setPixmap(0,iconReader.getIcon("fm_folder_small.bmp"));
        zlb_mgx->insertItem(zlbi,-1,true);
    }

    for ( QStringList::Iterator it = files.begin(); it!= files.end(); it++)
    {
        stat( *it, &filestatus );

        ZListBoxItem* zlbi = new ZListBoxItem( zlb_mgx );
            zlbi->appendSubItem(1,ctr(*it));
            zlbi->setPixmap(0,iconReader.getIcon("fm_folder_small.bmp"));

        if (filestatus.st_size<1024)
            sprintf(result,"%ld bytes",filestatus.st_size);
        else
            sprintf(result,"%.2f Kbytes",(double)filestatus.st_size/1024.0);
        zlbi->appendSubItem(1,result);

        if (strstr( QString(*it).ascii() ,".mgx")!=NULL)
            zlbi->setPixmap(0,iconReader.getIcon("certificate_mgr_std.g"));
        else
            zlbi->setPixmap(0,iconReader.getIcon("fm_unknown_thb.bmp"));

        zlb_mgx->insertItem(zlbi,-1,true);
    }

    setMainWidgetTitle(APP_TITLE);

    zlb_mgx->setSelected(1,true);

    if (znvtw->currentPageIndex()==1)
        zlb_mgx->setFocus();

    return true;
}

bool zBaseGUI::FillMgxManager(bool Reload)
{
    MGX_App* mgx_obj;
    ZListBoxItem* zlbi;
    QPixmap pm_sd = iconReader.getIcon("fm_memory_card_tab.bmp");
    QPixmap pm_ph = iconReader.getIcon("ctact_mobile_tab.bmp");
    QPixmap pm_reg = iconReader.getIcon("ptt_simple.g");
    QImage pm_icon_img;
    QPixmap pm_icon;

    int base_index_subitem = (sdphMultiSelectMode?1:0);

    bool showSD = getMainViewSubOptionsMenu()->isItemChecked(0);
    bool showPH = getMainViewSubOptionsMenu()->isItemChecked(1);
    bool showOnlyRegistered = getMainViewSubOptionsMenu()->isItemChecked(2);

//    DBG_OUT("ZOptionsMenu Size: %d \n",sizeof( getMainViewSubOptionsMenu() ) );

    if (Reload)
    {
        setMainWidgetTitle( lc("TXT_LOADING") );
        UpdateMGXList();
    }

    setMainWidgetTitle( lc("TXT_REFRESHING") );

    zlb_sdph->clear();

    for(MGXList::Iterator it = mgxlist.begin(); it!= mgxlist.end(); it++)
    {
        mgx_obj = (MGX_App*)*it;

        if (!showSD && mgx_obj->location == MGX_App::LOCATION_SD)
            continue;
        if (!showPH && mgx_obj->location == MGX_App::LOCATION_PH)
            continue;
        if (showOnlyRegistered && !mgx_obj->Registered)
            continue;

        zlbi = new ZListBoxItem( zlb_sdph );

        QString iconPath = mgx_obj->getIconPath();
        if (QFile::exists( iconPath )) 
        {
            pm_icon_img.load(iconPath);
            QImage scaledImage = pm_icon_img.smoothScale(49,49);
            pm_icon = scaledImage;
        }
        
        else
            pm_icon = iconReader.getIcon("yes_no_pop.bmp");
        
        pm_icon.resize(49,49);
        zlbi->setPixmap(base_index_subitem ,pm_icon);

        if (mgx_obj->location == MGX_App::LOCATION_PH)
            zlbi->appendSubItem(base_index_subitem+1, mgx_obj->Name,QString::null, &pm_ph );
        else
            zlbi->appendSubItem(base_index_subitem+1, mgx_obj->Name,QString::null, &pm_sd );

        if (mgx_obj->Registered)
            zlbi->appendSubItem(base_index_subitem+1, QString::null, mgx_obj->Author, &pm_reg );
        else
            zlbi->appendSubItem(base_index_subitem+1, QString::null, mgx_obj->Author, NULL );

        zlb_sdph->insertItem(zlbi,-1,true);
    }

    bool eEnable = false;
    if ( zlb_sdph->count() != 0 )
        eEnable = true;

    for (int i = 1;i<7;i++)
        getMainOptionsMenu()->setItemEnabled(i,eEnable);

    // For Move Operation
    getMainOptionsMenu()->setItemEnabled(0,eEnable & sdExists);
    // For Register Operation
    //getMainOptionsMenu()->setItemEnabled(4,false);
    // For MultiSelect Operation
    getMainOptionsMenu()->setItemEnabled(7,eEnable);

    if ( eEnable )
        zlb_sdph->setSelected(0,true);

    eEnable = false;
    if ( mgxlist.count() != 0 )
            eEnable = true;
    getMainOptionsMenu()->setItemEnabled(6,eEnable);

    if (znvtw->currentPageIndex()==0)
        zlb_sdph->setFocus();

    setMainWidgetTitle(APP_TITLE);

    return true;
}
MGX_App* zBaseGUI::getAppByListIndex(int index)
{
    MGX_App* mgx_obj=NULL;
    bool showSD = getMainViewSubOptionsMenu()->isItemChecked(0);
    bool showPH = getMainViewSubOptionsMenu()->isItemChecked(1);
    bool showOnlyRegistered = getMainViewSubOptionsMenu()->isItemChecked(2);

    int k=0;

    for(MGXList::Iterator it = mgxlist.begin(); it!= mgxlist.end(); it++)
    {
        mgx_obj = (MGX_App*)*it;
        if ( ( mgx_obj->location==MGX_App::LOCATION_SD && !showSD ) || ( mgx_obj->location==MGX_App::LOCATION_PH && !showPH ) )
            continue;
        if ( !mgx_obj->Registered && showOnlyRegistered )
            continue;
        if (k == index)
            break;
        k++;
    }
    return mgx_obj;
}
MGX_App* zBaseGUI::getSelectedApp()
{
    return getAppByListIndex( zlb_sdph->currentItem() );
}

void zBaseGUI::cmd_Move(MGX_App* mgx_obj, int index)
{
    if (mgx_obj==NULL)
        return;

    bool showSD = getMainViewSubOptionsMenu()->isItemChecked(0);
    bool showPH = getMainViewSubOptionsMenu()->isItemChecked(1);

    if (!sdphMultiSelectMode)
        showWaitDlg(true);

    QString dst_path = APP_FULLPATH_SD;;
    if (mgx_obj->location==MGX_App::LOCATION_SD)
        dst_path = APP_FULLPATH_PH;

    dst_path += "/" + mgx_obj->getDirName();

    QString mv_cmd = QString("mv '%1' '%2/'").arg(mgx_obj->Path).arg( dst_path );

    if (index==-1 && !sdphMultiSelectMode)
        index = zlb_sdph->currentItem();

    if ( exec_cmd( mv_cmd ) )
    {
        DBG_OUT("MGX Moved succesfully");
        QPixmap icon;
        if (mgx_obj->location==MGX_App::LOCATION_SD)
        {
            icon = iconReader.getIcon("ctact_mobile_tab.bmp");
            mgx_obj->location = MGX_App::LOCATION_PH;
        }else{
            icon = iconReader.getIcon("fm_memory_card_tab.bmp");
            mgx_obj->location = MGX_App::LOCATION_SD;
        }
        /* if both Phone and SD apps are visible when moved just change
            location icon. If not, then remove item from the list
        */
        if (showSD & showPH & sdExists)
        {
            ZListBoxItem* lbItem = (ZListBoxItem*)zlb_sdph->item(index);
            lbItem->setSubItem(1, 0, mgx_obj->Name,QString::null, &icon  );
        }
            
        else
            zlb_sdph->removeItem( index );

        mgx_obj->Path = dst_path;
        /*  Update Info in DB   */
        APPDB_UpdateAppInfo( mgx_obj->UUID, mgx_obj->Name, mgx_obj->Path, mgx_obj->location);
        if (mgx_obj->Registered)
            UpdateAppInfoInMenu(mgx_obj);
    }else{
        DBG_OUT("Error while moving MGX");
    }

    if (!sdphMultiSelectMode)
        showWaitDlg(false);

}
void zBaseGUI::cmd_MultiMove()
{
    int app_count = zlb_sdph->count();
    int not_visible_app_count = 0;
    int app_index;
    if (!app_count)
        return;

    bool showSD = getMainViewSubOptionsMenu()->isItemChecked(0);
    bool showPH = getMainViewSubOptionsMenu()->isItemChecked(1);

    showWaitDlg(true);

    ZListBoxItem* item;
    MGX_App* mgx_obj = NULL;
    for (int i = 0; i<app_count;i++)
    {
        app_index = i - not_visible_app_count;

        item = (ZListBoxItem*)zlb_sdph->item( app_index );
        if ( !item->isChecked() )
            continue;
        mgx_obj = getAppByListIndex( app_index );

        if (mgx_obj==NULL)
            continue;
        DBG_OUT("Move APP (%s)", mgx_obj->Name.ascii() );

        cmd_Move( mgx_obj, app_index);

        if (! (showSD & showPH & sdExists) )
        {
            not_visible_app_count++;
        }
    }

    showWaitDlg(false);

    mainOptM_MultiSelect();

}
void zBaseGUI::cmd_InstallMGX()
{
    int index = zlb_mgx->currentItem();
    
    ZListBoxItem *pathItem = (ZListBoxItem*)zlb_mgx->item(0);
    ZListBoxItem *mgxItem = (ZListBoxItem*)zlb_mgx->item(index);
    
    QString mgx_path = pathItem->getSubItemText(1,0);
    QString mgx_name = mgxItem->getSubItemText(1,0);

    QStringList slLocations;
    slLocations+=lc("FT_PH_MEMORY");
    if ( sdExists )
        slLocations+=lc("FT_SD_MEMORY");

    ZSingleSelectDlg* pSSD = new ZSingleSelectDlg( lc("POP_INSTALL_MGX_TITLE"), lc("POP_INSTALL_MGX_LOCATION_TEXT"), NULL, "ZSingleSelectDlg", true, 0, 0);
        pSSD->addItemsList(slLocations);
    int iLoc = pSSD->exec();

    //system( QString("chmod +x %1").arg( APP_FULLPATH_7Z ) );

    QString sCmd = QString("%2 x -y '%3'").arg( FullPath_7z ).arg( mgx_path + "/" + mgx_name );

    if (iLoc==0) return;

    showWaitDlg(true);

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
    exec_cmd(sCmd);
    system( sCmd2.ascii() );
    showWaitDlg(false);
    FillMgxManager(true);
}

void zBaseGUI::cmd_Delete(MGX_App* mgx_obj)
{
    if (mgx_obj==NULL)
        return;

    if (!sdphMultiSelectMode)
        showWaitDlg(true);

    QString rm_cmd = QString("rm -rf '%1'").arg(mgx_obj->Path);
    if ( exec_cmd( rm_cmd ) )
    {
        if ( mgx_obj->Registered )
            UnregisterAppFromMenu( mgx_obj );

        APPDB_RemoveApp( mgx_obj->UUID );
        mgxlist.remove( mgx_obj->getKeyName() );

        DBG_OUT("MGX Deleted succesfully");
    }else{
        DBG_OUT("Error while deleting MGX");
    }

    if (!sdphMultiSelectMode)
        showWaitDlg(false);
}
void zBaseGUI::cmd_MultiDelete()
{
    int app_count = zlb_sdph->count();
    int deleted_app_count = 0;

    if (!app_count)
        return;

    showWaitDlg(true);

    ZListBoxItem* item;
    MGX_App* mgx_obj = NULL;
    for (int i = 0; i<app_count;i++)
    {
        item = (ZListBoxItem*)zlb_sdph->item(i);
        if ( !item->isChecked() )
            continue;
        mgx_obj = getAppByListIndex( i - deleted_app_count);

        if (mgx_obj==NULL)
            continue;
        DBG_OUT("Delete APP (%s)", mgx_obj->Name.ascii() );
        deleted_app_count++;
        cmd_Delete( mgx_obj );
    }

    showWaitDlg(false);

    mainOptM_MultiSelect();
}
void zBaseGUI::cmd_SaveAs(MGX_App* mgx_obj, SAVE_LOCATION loc)
{
    if (mgx_obj==NULL || (loc==zBaseGUI::SAVE_TO_SD && !sdExists) )
        return;

    DBG_OUT(" Save Location : %d ", loc);

    char prev_path[MAXPATHLEN] = {0};
    getcwd( prev_path, MAXPATHLEN );

    QString dst_path;
    QString saveas_cmd;
    switch (loc)
    {
        case SAVE_TO_PH:
            dst_path= APP_FULLPATH_ROOT_PH"/mgx_files";
            break;
        case SAVE_TO_SD:
            dst_path= APP_FULLPATH_ROOT_SD"/mgx_files";
            break;
        case SAVE_TO_FM:
            ZListBoxItem *dstPathItem = (ZListBoxItem*)zlb_mgx->item(0);
            dst_path = dstPathItem->getSubItemText(1,0);
            break;
        default:
            return;
    }
    if (mgx_obj->location==MGX_App::LOCATION_PH)
        saveas_cmd = QString("cd '%1';").arg(APP_FULLPATH_PH);
    else
        saveas_cmd = QString("cd '%1';").arg(APP_FULLPATH_SD);

    saveas_cmd.append(QString(" %1 a -y '%2/%3.mgx' '%4'")
                        .arg( FullPath_7z )
                        .arg(dst_path)
                        .arg(mgx_obj->getDirName() )
                        .arg(mgx_obj->getDirName() )
                    );

    showWaitDlg(true);

    QString rm_cmd = QString("rm -f '%1/%2.mgx'").arg(dst_path).arg(mgx_obj->getDirName() );
    system( rm_cmd.ascii() );

    exec_cmd( saveas_cmd );

    showWaitDlg(false);

    chdir(prev_path);
}

bool zBaseGUI::exec_cmd(QString cmd)
{
    return sys_exec_cmd(cmd, sdphMultiSelectMode);
}

void zBaseGUI::FillMountPointsList()
{
/*
	struct mntent *mnt;
	FILE *fp;

	fp = setmntent (MOUNTED, "r");
	if (fp == NULL)
	    return;

    ZListBoxItem* pItem;

	while ( mnt = getmntent (fp) )
	{
        if ( strstr(mnt->mnt_type,"vfat")!=NULL)
        {
            pItem = new ZSettingItem(zlb_ph);
                pItem->appendSubItem(1, ctr(mnt->mnt_dir));
                pItem->appendSubItem(1, ctr(mnt->mnt_fsname), true, SETTINGITEM_REGION_B);
                pItem->appendSubItem(1, ctr(mnt->mnt_type), true, SETTINGITEM_REGION_B);
            if ( strstr(mnt->mnt_dir,"/mmc/mmc")!=NULL)
                pItem->setPixmap(2, iconReader.getIcon("fm_memory_card_tab.bmp") );
            else
                pItem->setPixmap(2, iconReader.getIcon("ctact_mobile_tab.bmp") );
            zlb_ph->insertItem(pItem, 0, true);
        }
	}
	endmntent(fp);
	zlb_ph->setFocus();
*/
}

void zBaseGUI::GetApplicationsMenu()
{
/*
    AM_AppRegistry* reg = new AM_AppRegistry();

    if (reg->init()!=0)
    {
        delete reg;
        DBG_OUT("Error Initializing AppRegistry Instance");
        return;
    }

    QString appUID;
    QStringList appUID_List;

    QValueList<const AM_AppObject* > list;

    QFile file(tr("/usr/data_resource/preloadapps/SysRegistry"));

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
                appUID_List<<tmp;
            }
        }
    }
    file.close();

    QFile file2(tr("/usr/SYSqtapp/am/SysRegistry"));
    if (file2.open(IO_ReadOnly))
    {
        QTextStream in(&file2);
        while ( !in.atEnd() )
        {
            QString tmp = in.readLine();
            if ( tmp.startsWith("[") && tmp.length() == 38 )
            {
                tmp = tmp.replace("[", "");
                tmp = tmp.replace("]", "");
                if (!appUID_List.contains(tmp) )
                    appUID_List<<tmp;

            }
        }
    }
    file2.close();

    reg->getAllAppObjects(list);

    printf("\n\tApps Count : %d\n", list.count() );

    for ( uint i = 0; i<list.count(); ++i )
    {
        appUID = (list[i]->getAppId()).toString();
            appUID.replace("{","");
            appUID.replace("}","");
        if (!appUID_List.contains(appUID) )
            appUID_List<<appUID;
    }

    QStringList appName_List;

    QValueList<AMRegEntry* > RE_list;
    AMRegEntry* re = NULL;
    for ( uint i = 0; i<appUID_List.count(); ++i )
    {
        appUID = appUID_List[i];
        re = new AMRegEntry(appUID);
        if (re->valid)
            RE_list.append(re);
        else
            delete re;
    }

    for ( uint i = 0; i<RE_list.count(); ++i )
    {
        printf("\tAPP OBJECT{ Name: %s, Type: %d, Icon: %s}\n",
            RE_list[i]->Name.ascii(),
            RE_list[i]->eType,
            RE_list[i]->SimpleIcon.ascii() );
    }
*/
//    delete reg;
}

void zBaseGUI::tabSelected( const QString& par)
{
    Q_UNUSED(par);
    getSoftKey()->setOptMenu(ZSoftKey::LEFT, getOptionsMenu() );
}

void zBaseGUI::app_selected(int index)
{
    Q_UNUSED(index);
    if (sdphMultiSelectMode)
        return;

    MGX_App* mgx_obj = getSelectedApp();
    if (mgx_obj==NULL)
        return;

    QString exec_cmd = mgx_obj->Path + "/" + mgx_obj->Exec + " &";

    DBG_OUT( "%s", exec_cmd.ascii() );
    system( exec_cmd.ascii() );
}
void zBaseGUI::mgx_highlighted(int index)
{
    DBG_OUT("MGX Higilighted called %d", index);
    bool oEnable = true;
    ZListBoxItem* lbItem = (ZListBoxItem*)zlb_mgx->item(index);
    QString subItemText = lbItem->getSubItemText(1,0);
    if (index<2 || !subItemText.endsWith(".mgx",false) )
    {   
        DBG_OUT("Debug here");
        oEnable = false;
    }
    getMgxOptionsMenu()->setItemEnabled(0,oEnable);
    DBG_OUT("MGX Higilighted END");
}

void zBaseGUI::mgx_selected(int index)
{   
    DBG_OUT("MGX Selected called");
    struct stat filestatus;
    ZListBoxItem* lbItem = (ZListBoxItem*)zlb_mgx->item(index);
    QString lbText = lbItem->getSubItemText(1,0);
    
    if (index<2 & zlb_mgx->item(index)->enabled())
    {
        DBG_OUT("Index < 2");
        FillMgxExplorer( lbText );
        return;
    }else if ( index!=0 ){
        stat( lbText.ascii(), &filestatus );
        if (filestatus.st_mode & S_IFDIR)
        {
            FillMgxExplorer( lbText );
            return;
        }
    }
    if ( lbText.endsWith(".mgx",false) )
        mgxOptM_Install();

    DBG_OUT("MGX selected index : %d", index);
}

void zBaseGUI::mainOptM_Move()
{
    MGX_App* mgx_obj = getSelectedApp();
    if (mgx_obj==NULL)
        return;

    if (!ConfirmAction(lc("POP_CONFIRM_TITLE"), lc("POP_CONFIRM_MOVE_TEXT") + "\n" + mgx_obj->Name ) )
        return;

    cmd_Move( mgx_obj );

    DBG_OUT("Move APP");
}
void zBaseGUI::mainOptMMS_Move()
{
    int app_count = zlb_sdph->count();
    if (app_count==0)
        return;

    if ( !ConfirmAction(lc("POP_CONFIRM_TITLE"), lc("POP_CONFIRM_MOVE_MULTIPLE_TEXT")) )
        return;
    QTimer::singleShot(10, this, SLOT( cmd_MultiMove() ) );
    DBG_OUT("Move Multiple APP");
}
void zBaseGUI::mainOptM_Rename()
{

    MGX_App* mgx_obj = getSelectedApp();
    if (mgx_obj==NULL)
        return;

	ZSingleCaptureDlg* dlg = new ZSingleCaptureDlg(
	    lc("POP_RENAME_TITLE"), lc("POP_RENAME_TEXT")+ " " + mgx_obj->Name,
	    ZSingleCaptureDlg::normal, NULL, "ZSingleCaptureDlg", true, 0, 0);

    ZLineEdit *line = new ZLineEdit( mgx_obj->Name, dlg, 0,( ZSkinService::WidgetClsID)39);
        line->selectAll();

    dlg->setEditWidget(line);
	if (!dlg->exec())
        return;

    mgx_obj->Name = line->text();

    QPixmap icon;
    if (mgx_obj->location==MGX_App::LOCATION_PH)
        icon = iconReader.getIcon("ctact_mobile_tab.bmp");
    else
        icon = iconReader.getIcon("fm_memory_card_tab.bmp");

    ZListBoxItem* lbItem = (ZListBoxItem*)zlb_sdph->item( zlb_sdph->currentItem() );
    lbItem->setSubItem(1, 0, mgx_obj->Name,QString::null, &icon  );

    QString cfgname;
    cfgname = QString("%1/%2.cfg").arg(mgx_obj->Path).arg( mgx_obj->getDirName() );
    char* buffer = NULL;
    uint lSize = 0;

    buffer = fileReadAll( cfgname.ascii(), &lSize );
    if (buffer)
    {
        buffer = (char*)realloc(buffer,lSize+512);
        if (buffer)
        {
            buffer[lSize]='\0';
            setconfig(buffer, "Name", mgx_obj->Name.stripWhiteSpace().ascii(), buffer);
            FILE * pFile;
            pFile = fopen( cfgname.ascii(), "w");
            if (pFile != NULL)
            {
                int bytestowrite = strlen(buffer);
                int nWritten = fwrite(buffer, sizeof(char), bytestowrite, pFile);
                if (nWritten!=bytestowrite)
                {
                        DBG_OUT("Error writing cfg file (%s)", cfgname.ascii());
                }
                fclose(pFile);
            }
        }
        free(buffer);
    }

    /*  Update Info in DB   */
    APPDB_UpdateAppInfo( mgx_obj->UUID, mgx_obj->Name, mgx_obj->Path, mgx_obj->location);
    if (mgx_obj->Registered)
        UpdateAppInfoInMenu(mgx_obj);

    DBG_OUT("Rename APP to %s", mgx_obj->Name.ascii() );
}
void zBaseGUI::mainOptM_Delete()
{
    MGX_App* mgx_obj = getSelectedApp();
    if (mgx_obj==NULL)
        return;

    if (!ConfirmAction(lc("POP_CONFIRM_TITLE"), lc("POP_CONFIRM_DELETE_TEXT") + "\n" + mgx_obj->Name ) )
        return;

    cmd_Delete( mgx_obj );
    zlb_sdph->removeItem( zlb_sdph->currentItem() );
    DBG_OUT("Delete APP");
}
void zBaseGUI::mainOptMMS_Delete()
{
    int app_count = zlb_sdph->count();
    if (app_count==0)
        return;

    if ( !ConfirmAction(lc("POP_CONFIRM_TITLE"), lc("POP_CONFIRM_DELETE_MULTIPLE_TEXT")) )
        return;
    QTimer::singleShot(10, this, SLOT( cmd_MultiDelete() ) );
    DBG_OUT("Delete Multiple APP");
}
void zBaseGUI::mainOptMMS_SelectAll()
{
    int app_count = zlb_sdph->count();
    for (int i = 0; i<app_count;i++)
        zlb_sdph->item(i)->check(true);
}
void zBaseGUI::mainOptMMS_UnselectAll()
{
    int app_count = zlb_sdph->count();
    for (int i = 0; i<app_count;i++)
        zlb_sdph->item(i)->check(false);
}
void zBaseGUI::mainOptM_Properties()
{
    DBG_OUT("Show APP Properties");

    MGX_App* mgx_obj = getSelectedApp();
    if (mgx_obj==NULL)
        return;

    ZMessageDlg* propDlg = new ZMessageDlg(mgx_obj->Name, NULL, ZMessageDlg::TypeOK);
    propDlg->setTitleIcon( QPixmap(mgx_obj->getIconPath()) );

    QFont def_small = ZApplication::font();
    def_small.setPixelSize(12);
   
    QString props = lc("APP_AUTHOR") + ":";
    props = props + mgx_obj->Author + "\n\n";
    props = props + lc("APP_VERSION") + ":";
    props = props + mgx_obj->Version + "\n\n";
    props = props + lc("APP_COMMENT") + ":";
    props = props + mgx_obj->Version + "\n\n";
    QStringList* strList = SplitTextToLines(mgx_obj->Comment, 210, def_small);

    for ( QStringList::Iterator it = strList->begin(); it!= strList->end(); it++)
    {
        props = props + *it + "\n";
    }
  
    propDlg->setMessage(props);
   
    propDlg->exec();
}
void zBaseGUI::mainSubOptM_SaveAs_PH()
{
    cmd_SaveAs(getSelectedApp(), zBaseGUI::SAVE_TO_PH);
    DBG_OUT("Save APP to PH");
}
void zBaseGUI::mainSubOptM_SaveAs_SD()
{
    cmd_SaveAs(getSelectedApp(), zBaseGUI::SAVE_TO_SD);
    DBG_OUT("Save APP to SD");
}
void zBaseGUI::mainSubOptM_SaveAs_FM()
{
    cmd_SaveAs(getSelectedApp(), zBaseGUI::SAVE_TO_FM);
    FillMgxExplorer();
    DBG_OUT("Save APP to FM");
}
void zBaseGUI::mainOptM_MultiSelect()
{
    sdphMultiSelectMode = !sdphMultiSelectMode;

    zlb_sdph->clear();
    if (sdphMultiSelectMode)
    {
        zlb_sdph->setItemType("%C%I%M");
        getSoftKey()->setText(ZSoftKey::RIGHT, lc("SK_CANCEL"), ZSoftKey::TEXT_PRIORITY(0));
        getSoftKey()->setOptMenu(ZSoftKey::LEFT, getMainMultiSelectOptionsMenu() );
    }else{
        zlb_sdph->setItemType("%I%M");
        getSoftKey()->setText(ZSoftKey::RIGHT, lc("SK_EXIT"), ZSoftKey::TEXT_PRIORITY(0));
        getSoftKey()->setOptMenu(ZSoftKey::LEFT, getMainOptionsMenu() );
    }

    FillMgxManager();

    DBG_OUT("Select Multiple APPs");
}
void zBaseGUI::mainOptM_RegisterAppMenu()
{
    ZNoticeDlg *noticeDlg;

    MGX_App* mgx_obj = getSelectedApp();
    if (mgx_obj==NULL)
        return;
    if (mgx_obj->Registered)
    {
        if (!ConfirmAction(lc("POP_CONFIRM_TITLE"), lc("POP_CONFIRM_UNREGISTER_APP_MENU") + "\n" + mgx_obj->Name ) )
            return;

        UnregisterAppFromMenu(mgx_obj);

        noticeDlg = new ZNoticeDlg((ZNoticeDlg::Type)0, 4, "" ,  lc("POP_OK_TEXT"), NULL, "ZNoticeDlg", true, 0);  
        noticeDlg->setTitle(lc("POP_OK_TITLE"));
        noticeDlg->exec();
        delete noticeDlg;

        FillMgxManager();

        return;
    }

    if (!ConfirmAction(lc("POP_CONFIRM_TITLE"), lc("POP_CONFIRM_REGISTER_APP_MENU") + "\n" + mgx_obj->Name ) )
        return;

    bool flag = RegisterAppInMenu(mgx_obj);
    if(flag) {
        noticeDlg = new ZNoticeDlg((ZNoticeDlg::Type)0, 4, "" ,  lc("POP_OK_TEXT"), NULL, "ZNoticeDlg", true, 0);  
        noticeDlg->setTitle(lc("POP_OK_TITLE"));
        noticeDlg->exec();
        delete noticeDlg;
    }
   
    FillMgxManager();

    DBG_OUT("Register APP to Menu");
}
void zBaseGUI::mainSubOptM_ViewSDPH()
{
    FillMgxManager();
}
void zBaseGUI::mainSubOptM_ViewRegistered()
{
    FillMgxManager();
}
void zBaseGUI::mainOptM_Refresh()
{
    FillMgxManager(true);
}
void zBaseGUI::mgxOptM_Install()
{
    int index = zlb_mgx->currentItem();
    ZListBoxItem* pathItem = (ZListBoxItem*)zlb_mgx->item(0);
    QString mgx_path = pathItem->getSubItemText(1,0);    
    
    ZListBoxItem* mgxItem  = (ZListBoxItem*)zlb_mgx->item(index);
    QString mgx_name = mgxItem->getSubItemText(1,0);

    if (!ConfirmAction( lc("POP_INSTALL_MGX_TITLE"), lc("POP_INSTALL_MGX_CONFIRM_TEXT").arg ( mgx_name ) ) )
        return;

    QTimer::singleShot(10, this, SLOT( cmd_InstallMGX() ) );

    DBG_OUT("Install MGX");
}
void zBaseGUI::mgxOptM_ShowAll()
{
    FillMgxExplorer(QString::null);
    DBG_OUT("Show All Files");
}
void zBaseGUI::mgxOptM_GoToPH()
{
    FillMgxExplorer(APP_FULLPATH_ROOT_PH);
    DBG_OUT("Go to Phone Memory");
}
void zBaseGUI::mgxOptM_GoToSD()
{
    FillMgxExplorer(APP_FULLPATH_ROOT_SD);
    DBG_OUT("Go to Memory Card");
}
void zBaseGUI::OptM_About()
{
	ZMessageDlg *aboutDlg = new ZMessageDlg(lc("FT_ABOUT"),
                                "MgxBox v2.0 (14-Jan-2010) \nE8 by Deepak \nOriginal by Kidscracker\nkidscracker@gmail.com\nTeam MMUS\nE8 Port by Deepak\n\nLicensed under GNU GPL License\n\n",
                                ZMessageDlg::TypeOK);
    aboutDlg->setTitleIcon( "certificate_mgr_std.g" );
	aboutDlg->exec();
	delete aboutDlg;
    DBG_OUT("About MgxBox");
}
/*  PARA MANEJO DE ZSettingItem
        ZMultiSelectDlg* zmsdlg = new ZMultiSelectDlg(ctr("Selección Multiple"), ctr("Marque las Opciones deseadas"), NULL, NULL, false, 0, 0);

        ZListBox* mlb = zmsdlg->getListBox();

  l      ZSettingItem* pItem = new ZSettingItem(mlb);
            pItem->appendSubItem(1, ctr("TEXTO..."));
            pItem->appendSubItem(1, ctr("SUB_TEXT"));
        mlb->insertItem(pItem);

            pItem = new ZSettingItem(mlb);
            pItem->appendSubItem(1, ctr("TEXTO 2..."));
            pItem->appendSubItem(1, ctr("SUB_TEXT 2"));
        mlb->insertItem(pItem);

        zmsdlg->show();
        zmsdlg->exec();


*/
