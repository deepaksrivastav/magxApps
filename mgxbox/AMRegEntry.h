#ifndef AMREGENTRY_H_INCLUDED
#define AMREGENTRY_H_INCLUDED

/* ZWidgets includes    */
#include <AM_AppRegistry.h>
#include <AM_AppObject.h>
#include <AM_RegistryObject.h>
#include <AM_Folder.h>

/*  Qt includes */
#include <qstring.h>

class AMRegEntry
{
public:
    AMRegEntry(QString objUID);
    ~AMRegEntry();

    QString getIconName();

/*  Data of the Entry   */
    QString UID;
    QString Name;
    int eType;
    QString SimpleIcon;
    QString SmallIcon;
    QString SVGIcon;
    QString ExecFullPath;

    bool valid;
    static AM_AppRegistry* reg;
    static AM_AppRegistry* getReg()
        {
            if (reg == NULL)
                reg = new AM_AppRegistry();
            return reg;
        }
    static void freeReg()
        {
            if (reg != NULL)
                delete reg;
            reg = NULL;
        }
};

#endif // AMREGENTRY_H_INCLUDED
