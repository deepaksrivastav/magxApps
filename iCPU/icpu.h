#ifndef __ICPU_H__
#define __ICPU_H__
#define QT_NO_DRAGANDDROP
#define QT_NO_PROPERTIES
#include <ZKbMainWidget.h>
#include <stdio.h>
#include <stdlib.h>
#include <qfile.h>
#include <qtextcodec.h>
#include <qlayout.h>
#include <string.h>
#include <ZConfig.h>

class MyZKbMain : public ZKbMainWidget
{	
    Q_OBJECT
	public:
		// the constructor
		MyZKbMain();
		~MyZKbMain();
	
	private slots:
		virtual void slotReveivePhoneMsg(const QCString& msg, const QByteArray& data);
};

#endif