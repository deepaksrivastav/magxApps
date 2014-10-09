#ifndef MGXBOX_H_INCLUDED
#define MGXBOX_H_INCLUDED

#define APP_TITLE "MgxBox"

#ifndef NO_DEBUG
    #define DBG_OUT(fmt, arg...) printf("DEBUG:: "fmt"\n", ##arg)
#else 
    #define DBG_OUT(fmt, arg...) 
#endif

#define APP_SYSFILES_PATH "/ezxlocal/mgxbox"
#define APP_DB_FULLPATH APP_SYSFILES_PATH"/AppMan.db"

#define APP_FULLPATH_7Z "/usr/local/p7zip/7z"

#define APP_FULLPATH_PH_SKIN "/ezxlocal/download/mystuff/skins"
#define APP_FULLPATH_SD_SKIN "/mmc/ext3/skins"

#define APP_FULLPATH_ROOT_PH "/mmc/movinand1"
#define APP_FULLPATH_PH "/mmc/movinand1/mpkgbox"
#define APP_FULLPATH_ROOT_SD "/mmc/mmca1"
#define APP_FULLPATH_SD "/mmc/mmca1/mpkgbox"

#define EZX_SYSTEM_CFG "/ezx_user/download/appwrite/setup/ezx_system.cfg"


#endif // MGXBOX_H_INCLUDED
