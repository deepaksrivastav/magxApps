/****************************************************************************
** MyZKbMain meta object code from reading C++ file 'icpu.h'
**
** Created: Mon Dec 14 06:47:33 2009
**      by: The Qt MOC ($Id: qt/src/moc/moc.y   2.3.8   edited 2004-08-05 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "icpu.h"
#include <qmetaobject.h>
#include <qapplication.h>
#ifdef QWS
#include <qobjectdict.h>
#endif



const char *MyZKbMain::className() const
{
    return "MyZKbMain";
}

QMetaObject *MyZKbMain::metaObj = 0;

#ifdef QWS
static class MyZKbMain_metaObj_Unloader {
public:
    ~MyZKbMain_metaObj_Unloader()
    {
         if ( objectDict )
             objectDict->remove( "MyZKbMain" );
    }
} MyZKbMain_metaObj_unloader;
#endif

void MyZKbMain::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(ZKbMainWidget::className(), "ZKbMainWidget") != 0 )
	badSuperclassWarning("MyZKbMain","ZKbMainWidget");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString MyZKbMain::tr(const char* s)
{
    return qApp->translate( "MyZKbMain", s, 0 );
}

QString MyZKbMain::tr(const char* s, const char * c)
{
    return qApp->translate( "MyZKbMain", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* MyZKbMain::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) ZKbMainWidget::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void (MyZKbMain::*m1_t0)(const QCString&,const QByteArray&);
    typedef void (QObject::*om1_t0)(const QCString&,const QByteArray&);
    m1_t0 v1_0 = &MyZKbMain::slotReveivePhoneMsg;
    om1_t0 ov1_0 = (om1_t0)v1_0;
    QMetaData *slot_tbl = QMetaObject::new_metadata(1);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(1);
    slot_tbl[0].name = "slotReveivePhoneMsg(const QCString&,const QByteArray&)";
    slot_tbl[0].ptr = (QMember)ov1_0;
    slot_tbl_access[0] = QMetaData::Private;
    metaObj = QMetaObject::new_metaobject(
	"MyZKbMain", "ZKbMainWidget",
	slot_tbl, 1,
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
