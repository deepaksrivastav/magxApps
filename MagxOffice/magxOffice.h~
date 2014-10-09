
#ifndef __MAGX_OFFICE__
#define __MAGX_OFFICE__

#include <ZKbMainWidget.h>
#include <ZListBox.h>
#include <ZSoftKey.h>
#include <ZOptionsMenu.h>
#include <RES_ICON_Reader.h>
#include <ZScrollPanel.h>
#include <ZApplication.h>
#include <ZConfig.h>
#include <qdir.h>
#include <qcopchannel_qws.h>
#include <ZAppInfoArea.h>
#include <stdlib.h>
#include <ZMessageDlg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ZSingleSelectDlg.h>


class MagxOffice : public ZKbMainWidget  
{
    Q_OBJECT
    public:
	    MagxOffice(QWidget *parent = 0);
	    virtual ~MagxOffice();
	
    protected:
	    void enterDirectory(const QString &sPath);
	
    private slots:
        void slotSelected(int index);
	    void slotAbout();	
	    void slotQuit();
    private:
	    QString getProgramDir();
	    void openFile(QString file);
    
    private:
	    ZListBox *listBox;
	    ZScrollPanel *widget;
	    ZSoftKey *softKey;
	    ZOptionsMenu *menu;
	    QString sFilter;
	    QString sBasePath;
	    QString currentFile;
	    QString lastDir;
	    ZConfig *config;
	    QString pdfViewerPath;
	    QString docViewerPath;
};
#endif //__MAGX_OFFICE__
