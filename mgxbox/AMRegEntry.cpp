#include "AMRegEntry.h"

AM_AppRegistry*  AMRegEntry::reg = NULL;

AMRegEntry::AMRegEntry(QString appUID)
{
    valid = false;
    // Get Registry Object
    const AM_RegistryObject* re_obj = AMRegEntry::getReg()->getRegistryObject(appUID);
    if (re_obj==NULL)
        return;
    UID = appUID;
    Name = re_obj->getName();
    eType = re_obj->getType();
    SimpleIcon = re_obj->getSimpleIcon();
    SmallIcon = re_obj->getSmallIcon();
    SVGIcon = re_obj->getSVGIcon();
    if (eType == 2)
    {
        ExecFullPath = re_obj->getSVGIcon();
    }
    valid = true;
}

AMRegEntry::~AMRegEntry()
{
}

QString AMRegEntry::getIconName()
{
    if (!SimpleIcon.isEmpty() )
        return SimpleIcon;
    if (!SVGIcon.isEmpty() )
        return SVGIcon;
    return QString::null;
}
