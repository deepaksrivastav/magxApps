#ifndef __MENU_SELECT_DLG_H__
#define __MENU_SELECT_DLG_H__

#include <ZPopup.h>
#include <ZListBox.h>
#include <ZApplication.h>
#include <ZConfig.h>
#include <qstring.h>
#include <qstringlist.h>
#include <RES_ICON_Reader.h>


class MgxMenuItem : public ZSettingItem
{
    public:
    MgxMenuItem(ZListBox* container = 0, const QString itemType = NULL):ZSettingItem(container,itemType)
    {
    }
    
    inline void setName(QString menuName)
    {
        name = menuName;
    }
    
    inline QString getName()
    {
        return name;
    }
    
    inline void setUUID(QString menuUUID)
    {
        uuid = menuUUID;
    }
   
    
    inline QString getUUID()
    {
        return uuid;
    }
    
    private:
    QString name;
    QString uuid;
};

class MenuSelectDlg : public ZPopup
{
    Q_OBJECT
    public:
        MenuSelectDlg(QString str1,QString str2,QString str3);
        ~MenuSelectDlg();
        inline QString getUUID()
        {
            return uuid;
        }
    private slots:
        void slotSelected();
        void slotItemClicked(ZSettingItem *item);
    private:
        ZListBox *listBox;
        QString uuid;
        RES_ICON_Reader iconReader;
        bool isItemInMenuTree(QString uuid);
};

#endif
