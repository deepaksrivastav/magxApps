/****************************************************************************
** MagxOffice meta object code from reading C++ file 'magxOffice.h'
**
** Created: Tue Nov 23 19:39:05 2010
**      by: The Qt MOC ($Id: qt/src/moc/moc.y   2.3.8   edited 2004-08-05 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "magxOffice.h"
#include <qmetaobject.h>
#include <qapplication.h>
#ifdef QWS
#include <qobjectdict.h>
#endif



const char *MagxOffice::className() const
{
    return "MagxOffice";
}

QMetaObject *MagxOffice::metaObj = 0;

#ifdef QWS
static class MagxOffice_metaObj_Unloader {
public:
    ~MagxOffice_metaObj_Unloader()
    {
         if ( objectDict )
             objectDict->remove( "MagxOffice" );
    }
} MagxOffice_metaObj_unloader;
#endif

void MagxOffice::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(ZKbMainWidget::className(), "ZKbMainWidget") != 0 )
	badSuperclassWarning("MagxOffice","ZKbMainWidget");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString MagxOffice::tr(const char* s)
{
    return qApp->translate( "MagxOffice", s, 0 );
}

QString MagxOffice::tr(const char* s, const char * c)
{
    return qApp->translate( "MagxOffice", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* MagxOffice::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) ZKbMainWidget::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (MagxOffice::*m1_t0)(int);
    typedef void (QObject::*om1_t0)(int);
    typedef void (MagxOffice::*m1_t1)();
    typedef void (QObject::*om1_t1)();
    typedef void (MagxOffice::*m1_t2)();
    typedef void (QObject::*om1_t2)();
    m1_t0 v1_0 = &MagxOffice::slotSelected;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    m1_t1 v1_1 = &MagxOffice::slotAbout;
    om1_t1 ov1_1 = (om1_t1)v1_1;
    m1_t2 v1_2 = &MagxOffice::slotQuit;
    om1_t2 ov1_2 = (om1_t2)v1_2;
    QMetaData *slot_tbl = QMetaObject::new_metadata(3);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(3);
    slot_tbl[0].name = "slotSelected(int)";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl_access[0] = QMetaData::Private;
    slot_tbl[1].name = "slotAbout()";
    slot_tbl[1].ptr = (QMember)ov1_1;
    slot_tbl_access[1] = QMetaData::Private;
    slot_tbl[2].name = "slotQuit()";
    slot_tbl[2].ptr = (QMember)ov1_2;
    slot_tbl_access[2] = QMetaData::Private;
    metaObj = QMetaObject::new_metaobject(
	"MagxOffice", "ZKbMainWidget",
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}
