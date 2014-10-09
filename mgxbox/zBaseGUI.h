#ifndef ZBASEGUI_H_INCLUDED
#define ZBASEGUI_H_INCLUDED

/* ZWidgets includes    */
#include <ZApplication.h>
#include <ZConfig.h>
#include <ZFormContainer.h>
#include <ZKbInputField.h>
#include <ZKbMainWidget.h>
#include <ZLabel.h>
#include <ZLineEdit.h>
#include <ZListBox.h>
#include <ZMessageDlg.h>
#include <ZNavTabWidget.h>
#include <ZOptionsMenu.h>
#include <ZSingleCaptureDlg.h>
#include <ZSingleSelectDlg.h>
#include <ZSoftKey.h>
#include <ZNoticeDlg.h>

#include <RES_ICON_Reader.h>

#include <AM_AppRegistry.h>
#include <AM_AppObject.h>
#include <AM_Folder.h>
#include <AM_RegistryObject.h>

/*  Qt includes */
#include <qfile.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <quuid.h>

/*  C++ includes    */
#include <stdio.h>
#include <stdlib.h>

/*  OS includes */
#include <mntent.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

/*  My Includes */
#include "AMRegEntry.h"
#include "MenuSelectDlg.h"

class MGX_App;
typedef QMap<QString, MGX_App *> MGXList;

class zBaseGUI: public ZKbMainWidget
{
    Q_OBJECT
public:
    zBaseGUI(QWidget* parent);
    ~zBaseGUI();

    static QString ctr(const char*NonANSI_Str)
                    {
                        if (gbkstr == NULL)
                            gbkstr = QTextCodec::codecForName("UTF-8");
                        return gbkstr->toUnicode(NonANSI_Str);
                    };

    void FillMountPointsList();
    void GetApplicationsMenu();

    enum SAVE_LOCATION
    {
        SAVE_TO_PH,
        SAVE_TO_SD,
        SAVE_TO_FM
    };

protected slots:
    void quit();
    void firstRefresh();

    void tabSelected( const QString& par);

    void app_selected( int index );

    void mgx_highlighted( int index );
    void mgx_selected( int index );

    void mgxOptM_Install();
    void mgxOptM_GoToSD();
    void mgxOptM_GoToPH();
    void mgxOptM_ShowAll();

    void mainOptM_Move();
    void mainOptM_Rename();
    void mainOptM_Delete();

    void mainOptMMS_Move();
    void mainOptMMS_Delete();
    void mainOptMMS_SelectAll();
    void mainOptMMS_UnselectAll();

    void mainOptM_Properties();
    void mainOptM_RegisterAppMenu();

    void mainSubOptM_SaveAs_PH();
    void mainSubOptM_SaveAs_SD();
    void mainSubOptM_SaveAs_FM();

    void mainSubOptM_ViewSDPH();
    void mainSubOptM_ViewRegistered();
    void mainOptM_MultiSelect();
    void mainOptM_Refresh();
    void OptM_About();

    void cmd_InstallMGX();
    void cmd_Move(MGX_App* mgx_obj,int index = -1);
    void cmd_MultiMove();
    void cmd_Delete(MGX_App* mgx_obj);
    void cmd_MultiDelete();
    void cmd_SaveAs(MGX_App* mgx_obj, SAVE_LOCATION loc);

protected:
    void CreateControls(QWidget* parent);
    bool FillMgxExplorer(QString cdir = QString::null );
    bool FillMgxManager(bool Reload = false);

    bool exec_cmd(QString cmd);

    void showWaitDlg(bool show);

    MGX_App* getAppByListIndex(int index);
    MGX_App* getSelectedApp();
    ZSoftKey* getSoftKey();
    ZOptionsMenu* getOptionsMenu();
    ZOptionsMenu* getMainMultiSelectOptionsMenu();
    ZOptionsMenu* getMainOptionsMenu();
    ZOptionsMenu* getMainSaveAsMGXSubOptionsMenu();
    ZOptionsMenu* getMainViewSubOptionsMenu();
    ZOptionsMenu* getMgxOptionsMenu();


private:
/*  Internal Use Objects    */
    static QTextCodec* gbkstr;
	RES_ICON_Reader iconReader;

/*  Widget Control Objects  */
    ZNavTabWidget* znvtw;
    ZSoftKey* softkey;
    ZOptionsMenu* optm_Main;
    ZOptionsMenu* optm_MainMultiSelect;
    ZOptionsMenu* optm_MainView;
    ZOptionsMenu* optm_MainSaveAsMGX;
    ZOptionsMenu* optm_Mgx;
    ZOptionsMenu* optm_Fav;

    ZListBox* zlb_sdph;
    ZListBox* zlb_mgx;
    ZListBox* zlb_fav;
    ZPopup* waitDlg;
    bool sdphMultiSelectMode;
};

#endif // ZBASEGUI_H_INCLUDED
