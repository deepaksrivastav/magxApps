#include "MenuSelectDlg.h"

MenuSelectDlg::MenuSelectDlg(QString str1,QString str2,QString str3):ZPopup()
{
    setTitle(str1);
    listBox = new ZListBox("%I%M",this);
    
    // insert menu items with images from SysRegistry
    ZConfig config ( "/usr/data_resource/preloadapps/SysRegistry", false );
    QStringList grouplist;
    config.getGroupsKeyList ( grouplist );
    int appType = -1;
    
    for ( QStringList::Iterator it = grouplist.begin(); it != grouplist.end(); ++it ) {
        appType = config.readNumEntry(*it, "ObjectType", 0);
        // int javaFolder = config.readNumEntry(*it, "JavaEnabledFolder", 0);
        if(appType == 1 && isItemInMenuTree(*it)){
            MgxMenuItem *menuItem = new MgxMenuItem(listBox);
            menuItem->setName(config.readEntry ( *it, "Name", "" ));
            menuItem->setUUID(*it);
            QString icon = config.readEntry ( *it, "BigIcon", "" );
            QImage pm_icon_img;
            QPixmap pm_icon;
            if (QFile::exists( icon )) 
            {
                pm_icon_img.load(icon);
                QImage scaledImage = pm_icon_img.smoothScale(49,49);
                pm_icon = scaledImage;
            } else {
                if(NULL != icon && icon !="")
                    pm_icon = iconReader.getIcon(icon);
                else 
                    pm_icon = iconReader.getIcon("certificate_mgr_std.g");    
            }
            // pm_icon.resize(49,49);
            
            listBox->insertItem(menuItem);
            menuItem->setPixmap(0, pm_icon);
            menuItem->appendSubItem(1,menuItem->getName());
        }
    }
    
    // read folders from InstalledDB
    ZConfig installedDBConfig ( "/ezxlocal/download/appwrite/am/InstalledDB", false );
    QStringList insDBgrouplist;
    installedDBConfig.getGroupsKeyList ( insDBgrouplist );
    for ( QStringList::Iterator it = insDBgrouplist.begin(); it != insDBgrouplist.end(); ++it ) {
        appType = installedDBConfig.readNumEntry(*it, "ObjectType", 0);
        if(appType == 1 && isItemInMenuTree(*it)){
            MgxMenuItem *menuItem = new MgxMenuItem(listBox);
            menuItem->setName(installedDBConfig.readEntry ( *it, "Name", "" ));
            menuItem->setUUID(*it);
            QString icon = installedDBConfig.readEntry ( *it, "BigIcon", "" );
            QImage pm_icon_img;
            QPixmap pm_icon;
            if (QFile::exists( icon )) 
            {
                pm_icon_img.load(icon);
                QImage scaledImage = pm_icon_img.smoothScale(49,49);
                pm_icon = scaledImage;
            } else {
                if(NULL != icon && icon !="")
                    pm_icon = iconReader.getIcon(icon);
                else 
                    pm_icon = iconReader.getIcon("certificate_mgr_std.g");    
            }
            // pm_icon.resize(49,49);
            
            listBox->insertItem(menuItem);
            menuItem->setPixmap(0, pm_icon);
            menuItem->appendSubItem(1,menuItem->getName());
        }
    }
    
    insertChild(listBox);
    setSoftkeyText(str2,str3);
    connect(this,SIGNAL(leftSoftkeyClicked()),this,SLOT(slotSelected()));
    connect(listBox,SIGNAL(returnPressed(ZSettingItem *)),this,SLOT(slotItemClicked(ZSettingItem *)));
    
}

bool MenuSelectDlg::isItemInMenuTree(QString uuid)
{
    ZConfig config ( "/ezxlocal/download/appwrite/am/UserMenuTree", false );
    QStringList grouplist;
    config.getGroupsKeyList ( grouplist );
    for ( QStringList::Iterator it = grouplist.begin(); it != grouplist.end(); ++it ) {
        if(*it == uuid) {
            return true;
        }
    }
    return false;
}


MenuSelectDlg::~MenuSelectDlg()
{
}

void MenuSelectDlg::slotItemClicked(ZSettingItem *item)
{
    MgxMenuItem *citem = (MgxMenuItem *)item;
    uuid = citem->getUUID();
    accept();
}

void MenuSelectDlg::slotSelected()
{
    MgxMenuItem *item = (MgxMenuItem *)listBox->item(listBox->currentItem());
    uuid = item->getUUID();
}

