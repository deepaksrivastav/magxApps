/****************************************************************************
** MyZApplication meta object code from reading C++ file 'myapp.h'
**
** Created: Mon Dec 14 06:47:34 2009
**      by: The Qt MOC ($Id: qt/src/moc/moc.y   2.3.8   edited 2004-08-05 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "myapp.h"
#include <qmetaobject.h>
#include <qapplication.h>
#ifdef QWS
#include <qobjectdict.h>
#endif



const char *MyZApplication::className() const
{
    return "MyZApplication";
}

QMetaObject *MyZApplication::metaObj = 0;

#ifdef QWS
static class MyZApplication_metaObj_Unloader {
public:
    ~MyZApplication_metaObj_Unloader()
    {
         if ( objectDict )
             objectDict->remove( "MyZApplication" );
    }
} MyZApplication_metaObj_unloader;
#endif

void MyZApplication::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(ZApplication::className(), "ZApplication") != 0 )
	badSuperclassWarning("MyZApplication","ZApplication");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString MyZApplication::tr(const char* s)
{
    return qApp->translate( "MyZApplication", s, 0 );
}

QString MyZApplication::tr(const char* s, const char * c)
{
    return qApp->translate( "MyZApplication", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* MyZApplication::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) ZApplication::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (MyZApplication::*m1_t0)();
    typedef void (QObject::*om1_t0)();
    typedef void (MyZApplication::*m1_t1)();
    typedef void (QObject::*om1_t1)();
    typedef void (MyZApplication::*m1_t2)();
    typedef void (QObject::*om1_t2)();
    m1_t0 v1_0 = &MyZApplication::slotShutdown;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    m1_t1 v1_1 = &MyZApplication::slotQuickQuit;
    om1_t1 ov1_1 = (om1_t1)v1_1;
    m1_t2 v1_2 = &MyZApplication::slotRaise;
    om1_t2 ov1_2 = (om1_t2)v1_2;
    QMetaData *slot_tbl = QMetaObject::new_metadata(3);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(3);
    slot_tbl[0].name = "slotShutdown()";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl_access[0] = QMetaData::Protected;
    slot_tbl[1].name = "slotQuickQuit()";
    slot_tbl[1].ptr = (QMember)ov1_1;
    slot_tbl_access[1] = QMetaData::Protected;
    slot_tbl[2].name = "slotRaise()";
    slot_tbl[2].ptr = (QMember)ov1_2;
    slot_tbl_access[2] = QMetaData::Protected;
    metaObj = QMetaObject::new_metaobject(
	"MyZApplication", "ZApplication",
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
