#include "magxOffice.h"

MagxOffice::MagxOffice(QWidget *parent):ZKbMainWidget(ZHeader::MAINDISPLAY_HEADER,parent,0,0)
{
	//load config
	QString configFile = QString("%1%2").arg(getProgramDir()).arg("magxOffice.ini");
	config = new ZConfig(configFile,true);
	sFilter = "*.pdf *.docx *odt";
	lastDir = config->readEntry("MagxOfficeConfig","LastDir");
	
	pdfViewerPath = config->readEntry("MagxOfficeConfig","PDFViewerPath",QString("%1%2").arg(getProgramDir()).arg("pdf"));
	docViewerPath = config->readEntry("MagxOfficeConfig","DocViewerPath",QString("%1%2").arg(getProgramDir()).arg("docViewer"));
	
	//initialize UI
	setMainWidgetTitle("Magx Office");
	listBox = new ZListBox("%I%M",this,0);
	setContentWidget(listBox);
	softKey = new ZSoftKey("CST_2",this,0);
	softKey->setText(ZSoftKey::LEFT,"Options");
	softKey->setText(ZSoftKey::RIGHT,"Quit");
	setCSTWidget(softKey);
	QRect rect=ZGlobal::getContentR();
	menu=new ZOptionsMenu(rect, this,0,0,ZSkinService::WidgetClsID(55));
	softKey->setOptMenu(ZSoftKey::LEFT, menu);
	menu->insertItem("About", NULL, this, SLOT(slotAbout()), true, false, false, 0);
	
	//load FileList
	enterDirectory(lastDir);
	
	//connect signal/slots
	connect(listBox,SIGNAL(selected(int)),this,SLOT(slotSelected(int)));
	softKey->setClickedSlot(ZSoftKey::RIGHT,this,SLOT(slotQuit()));
}

MagxOffice::~MagxOffice()
{
	delete listBox;
	delete softKey;
}

void MagxOffice::slotQuit()
{
    config->writeEntry("FILESELECT","LASTDIR",sBasePath);
    qApp->quit();
}

void MagxOffice::enterDirectory(const QString &sPath)
{
	RES_ICON_Reader iconReader;
	bool isBack = true;
	QString iconName = "";
	listBox->clear();
	QDir dir ( sPath, sFilter );
    dir.setMatchAllDirs ( true );
    dir.setFilter ( QDir::Dirs | QDir::Hidden );
    if ( !dir.isReadable() )
      return;
    QStringList entries = dir.entryList();
    entries.sort();
    QStringList::ConstIterator it = entries.begin();
    QPixmap pixmap;
    int i=1;
    if(it==entries.end())
	{
		isBack = isBack && false;
		pixmap = QPixmap(QString("%1%2").arg(getProgramDir()).arg("ico.png"));
		ZListBoxItem* Item = new ZListBoxItem ( listBox, QString ( "%I%M" ) );
        Item->setPixmap ( 0, pixmap  );
        Item->appendSubItem ( 1, "..", false, NULL );
        listBox->insertItem ( Item,-1,true );
	}
	while ( it != entries.end() )
	{
		if(*it != "."&&*it!=".."&&i==1&&dir.canonicalPath()!="/")
		{
			isBack = isBack && false;  
			pixmap = iconReader.getIcon("gen_back_to_arrw_small.bmp",true);
			ZListBoxItem* Item = new ZListBoxItem ( listBox, QString ( "%I%M" ) );
			Item->setPixmap ( 0, pixmap  );
			Item->appendSubItem ( 1, "..", false, NULL );
			listBox->insertItem ( Item,-1,true );
			i++;
			continue;
		}
		if ( *it != "." )
		{
			if (*it != "..")
			{ 
				isBack = isBack && true; 
				pixmap = iconReader.getIcon("fm_folder_small.bmp",true);
			} 
			else  
			{
				isBack = isBack && false;
				pixmap = iconReader.getIcon("gen_back_to_arrw_small.bmp",true);
			}
			ZListBoxItem* Item = new ZListBoxItem ( listBox, QString ( "%I%M" ) );
			Item->setPixmap ( 0, pixmap  );
			Item->appendSubItem ( 1, *it, false, NULL );
			listBox->insertItem ( Item,-1,true );
		}
		i++;
		++it;
	}
	dir.setFilter ( QDir::Files | QDir::Hidden );
	entries = dir.entryList();
	entries.sort();
	it = entries.begin();
	while ( it != entries.end() )
	{
		if ( *it != "." )
		{
			ZListBoxItem* Item;
			if (*it != "..")  
			{
				isBack = isBack && true; 
		        pixmap = QPixmap(QString("%1%2").arg(getProgramDir()).arg("ico.png"));
		        Item = new ZListBoxItem ( listBox, QString ( "%I%M" ) );
			}  
			else
			{
				isBack = isBack && false;
				pixmap = iconReader.getIcon("gen_back_to_arrw_small.bmp",true);
				Item = new ZListBoxItem ( listBox, QString ( "%I%M" ) );
			}
			Item->setPixmap ( 0, pixmap);
			Item->appendSubItem ( 1, *it, false, NULL );
			listBox->insertItem ( Item,-1,true );
        }
        ++it;
    }
    sBasePath = dir.canonicalPath();
    if ((isBack) && (sBasePath != "/") && (sBasePath.length() > 1)) 
    {
        ZListBoxItem* Item = new ZListBoxItem ( listBox, QString ( "%I%M" ) );
	    QPixmap pixmap = iconReader.getIcon("gen_back_to_arrw_small.bmp",true);
        Item->setPixmap ( 0, pixmap  );
        Item->appendSubItem ( 1, QString(".."), false, NULL );
        listBox->insertItem ( Item, 0,true );
        isBack = true;
    }  
    listBox->setCurrentItem(0);
}

QString MagxOffice::getProgramDir()
{
	QString m_sProgramDir = QString ( qApp->argv() [0] ) ;
	int i = m_sProgramDir.findRev ( "/" );
	m_sProgramDir.remove ( i+1, m_sProgramDir.length() - i );
	return m_sProgramDir;
}

void MagxOffice::slotSelected(int index)
{
    QString a = "";
    ZListBoxItem* listitem = (ZListBoxItem *)listBox->item(index);
    a = listitem->getSubItemText(1, 0, false);
    QString sPath = sBasePath + "/" + a;  
    if (QFileInfo(sPath).isDir())
    {
        listBox->clear();
        enterDirectory(sPath);
    }     
    else
    {
        openFile(sPath);
    } 
}

void MagxOffice::openFile(QString file)
{
    QString extension = QFileInfo(file).extension();
    if(extension.upper() == "PDF")
    {
        QString command = QString("%1 -d %2").arg(pdfViewerPath).arg(file);
        system(command.utf8());     
    }
    
    if(extension.upper() == "DOCX" || extension.upper() == "ODT")
    {
        QString command = QString("%1 -d %2").arg(docViewerPath).arg(file);
        system(command.utf8());
    }
}
    
void MagxOffice::slotAbout()
{
    ZMessageDlg *dlg = new ZMessageDlg("About","MagxOffice v0.1b \nAuthor:Deepak Srivastav\nThanks to Motocakerteam, Tulanix",ZMessageDlg::TypeOK,0,this);
    dlg->exec();
    delete dlg;
}

