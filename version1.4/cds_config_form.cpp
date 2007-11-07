/*
    This file is part of coindesigner.

    coindesigner is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    coindesigner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with coindesigner; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#include "cds_config.h"
#include <cds_config_form.h>
#include <qfiledialog.h>
#include <qprocess.h>
#include <qstring.h>
#include <iostream>

///Objeto para almacenar la configuración de coindesigner
QSettings settings;

/*
 *  Constructs a cds_config_form as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
cds_config_form::cds_config_form( QWidget* parent, const char* name, bool modal, WFlags fl )
: QDialog( parent, name, modal, fl )
{
	if ( !name )
		setName( "cds_config_form" );
	setSizeGripEnabled( true );
	setModal( TRUE );
	cds_config_form_layout = new QVBoxLayout( this, 11, 6, "cds_config_form_layout"); 

	//Leemos los setting sobre el QStringList
    QStringList keys = settings.entryList( "/coindesigner" );

	//Creamos una tabla del tamaño adecuado
	table = new QTable(keys.size(),2, this);

	//Rellenamos la tabla con los settings encontrados
	QHeader *verticalHeader = table->verticalHeader();
	int i=0;
	for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) 
	{
		//Rellenamos el nombre del setting
		verticalHeader->setLabel(i, *it);

		//Rellenamos el valor actual del setting
		QString key("/coindesigner/");
		key.append(*it);
		table->setText(i,0,settings.readEntry(key));

		//Introducimos un helper
		table->setPixmap(i,1, QPixmap::fromMimeSource( "filefind.png" ));
		table->adjustRow(i);

		//Siguiente fila
		i++;
	}

	//Ajustamos el ancho de la columna
	table->horizontalHeader()->setLabel( 0,tr("Valor") );
	table->horizontalHeader()->setLabel( 1,"");
	table->verticalHeader()->adjustHeaderSize();
	table->adjustColumn(0);
	table->setColumnWidth(1,table->rowHeight(0));
	table->setColumnReadOnly(1,true);

	//Añadimos la tabla al layout del formulario
	cds_config_form_layout->addWidget( table );

	//Callback para detectar pinchazos en el helper
	connect(table, SIGNAL( clicked(int,int,int,const QPoint&) ), this, SLOT( table_clicked(int,int) ) );

	//Layout para la botonera inferior (OK, Cancel)
	layout10 = new QHBoxLayout( 0, 0, 6, "layout10"); 

	buttonOk = new QPushButton( this, "buttonOk" );
	buttonOk->setAutoDefault( TRUE );
	buttonOk->setDefault( TRUE );
	layout10->addWidget( buttonOk );

	spacer2 = new QSpacerItem( 60, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	layout10->addItem( spacer2 );

	buttonReset = new QPushButton( this, "buttonReset" );
	layout10->addWidget( buttonReset );

	layout10->addItem( new QSpacerItem( 60, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

	buttonCancel = new QPushButton( this, "buttonCancel" );
	buttonCancel->setAutoDefault( TRUE );
	layout10->addWidget( buttonCancel );

	cds_config_form_layout->addLayout( layout10 );

	languageChange();
	resize( QSize(table->contentsWidth()+table->verticalHeader()->width()+28, 400).expandedTo(minimumSizeHint()) );
	clearWState( WState_Polished );

	// signals and slots connections
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( buttonOk_clicked() ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonReset, SIGNAL( clicked() ), this, SLOT( buttonReset_clicked() ) );
	connect( buttonReset, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

/*
*  Destroys the object and frees any allocated resources
*/
cds_config_form::~cds_config_form()
{
	// no need to delete child widgets, Qt does it all for us
}


/*
*  Sets the strings of the subwidgets using the current
*  language.
*/
void cds_config_form::languageChange()
{
	setCaption( tr( "Opciones de coindesigner" ) );

	buttonOk->setText( tr( "&OK" ) );
	buttonOk->setAccel( QKeySequence( QString::null ) );
	buttonReset->setText( tr( "&Reset" ) );
	buttonReset->setAccel( QKeySequence( QString::null ) );
	buttonCancel->setText( tr( "&Cancelar" ) );
	buttonCancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
}


void cds_config_form::buttonOk_clicked ()
{
	//printf("Has pinchado OK\n");

	//Salvamos todos los settings
	QHeader *verticalHeader = table->verticalHeader();
	for ( int i=0; i < table->numRows(); i++)
	{

		//Rellenamos el valor actual del setting
		QString key("/coindesigner/");
		key.append(verticalHeader->label(i));

		//Leemos el valor y miramos si es nulo
		QString value = table->text(i,0);
		if (value)
			settings.writeEntry(key, value);
		else
			settings.removeEntry(key);

		//printf ("%d: %s=%s\n", i, key.ascii(), value.ascii() );
	}

}

void cds_config_form::buttonReset_clicked ()
{
	//Borramos todos los settings
	cds_removeSettings();
	cds_defaultSettings();
}

void cds_config_form::table_clicked (int fila, int columna)
{
	//printf("Has pinchado %d %d\n", fila, columna);
	if (columna == 1)
	{
		//Miramos si buscamos un nombre de directorio o de fichero
		if (table->verticalHeader()->label(fila).endsWith("_dir", false))
		{
			QString value = QFileDialog::getExistingDirectory (table->text(fila,0));
			if (value)
			{
				table->setText(fila, 0, QString("file://")+value);
				table->adjustColumn(0);
			}
		}
		else //if (verticalHeader->label(fila).endsWith("_app")
		{
			QString value = QFileDialog::getOpenFileName (table->text(fila,0));

			if (value)
			{
				table->setText(fila, 0, value);
				table->adjustColumn(0);
			}
		}

		//Colocamos el focus en la celda editada
		table->setCurrentCell(fila,0);
	}
}


///Elimina todos los settings de la aplicacion
void cds_removeSettings()
{
   QStringList keys = settings.entryList( "/coindesigner" );
   for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) 
   {
        QString key("/coindesigner/");
        key.append(*it);
        std::cerr << "Eliminando: " << key << std::endl;
        settings.removeEntry(key);
   }
}


///Almacena todos los settings por defecto de la aplicacion
void cds_defaultSettings()
{
   QString key, value;

   //Navegador por defecto
   key = "/coindesigner/helpViewer_app";
   value = settings.readEntry(key);
   if (!value)
   {
      #ifdef _WIN32
      value = "explorer.exe";
      #else
      value = "firefox";
      #endif
      std::cerr << "Setting: " << key << "=" << value << std::endl;
      settings.writeEntry(key, value);
   }

   //Directorio de la ayuda
   key = "/coindesigner/reference_dir";
   value = settings.readEntry(key);
   if (!value)
   {
	   QString reference_dir;
      //Ayuda local
      //value = "file:///home/jespa/PFC/CDS/coindesigner/reference/";

	  //Buscamos el directorio de ayuda en el directorio de coindesigner
	  reference_dir.sprintf("%s/reference", cds_dir);
#ifdef _WIN32
	  reference_dir.replace( QChar('/'), "\\" );
#endif

	  if (QFile::exists(reference_dir) && QFileInfo(reference_dir).isDir() )
	  {
		  value = QString("file://")+reference_dir;
	  }
	  else
	  {
		  //Ayuda en coindesigner.sf.net
		  value = "http://coindesigner.sf.net/reference/";
	  }

      std::cerr << "Setting: " << key << "=" << value << std::endl;
      settings.writeEntry(key, value);
   }

   //Directorio con los tutoriales
   key = "/coindesigner/tutorial_dir";
   value = settings.readEntry(key);
   if (!value)
   {
      //Ayuda local
      //value = "file:///home/jespa/PFC/CDS/coindesigner/tut/";
      //Ayuda en coindesigner.sf.net
      value = "http://coindesigner.sf.net/tutorials/";
      std::cerr << "Setting: " << key << "=" << value << std::endl;
      settings.writeEntry(key, value);
   }

   //Path a QSlim. Si no existe lo buscamos
   key = "/coindesigner/qslim_app";
   value = settings.readEntry(key);
   if (!value)
   {
	   QString qslim_app;

	   //Miramos si existe una variable de entorno adecuada
	   qslim_app = getenv("QSLIM");
   	   if (qslim_app && QFile::exists(qslim_app) && !QFileInfo(qslim_app).isDir() && QFileInfo(qslim_app).isExecutable())
	   {
			value = qslim_app;
	   }
	   else
	   {
		   //Buscamos qslim en el directorio de coindesigner
#ifdef _WIN32
		   qslim_app.sprintf("%s/qslim.exe", cds_dir);
		   qslim_app.replace( QChar('/'), "\\" );
#else
		   qslim_app.sprintf("%s/qslim", cds_dir);
#endif

		   if (QFile::exists(qslim_app) && !QFileInfo(qslim_app).isDir() && QFileInfo(qslim_app).isExecutable())
		   {
			   value = qslim_app;
		   }
		   else
		   {
#ifdef _WIN32
			   qslim_app = "QSlim.exe";
#else
			   qslim_app = "qslim";
#endif
			   //Buscamos qslim en el path
			   QProcess *proc = new QProcess( NULL );
			   proc->addArgument( qslim_app );
			   proc->addArgument( "-h" );
			   if (proc->start() ) 
			   {
				   value = qslim_app;
			   }
			   proc->tryTerminate();
			   delete proc;
		   }
	   }

	   if (value)
	   {
		   //Salvamos el path encontrado
		   std::cerr << "Setting: " << key << "=" << value << std::endl;
		   settings.writeEntry(key, value);
	   }

   }//if (!value)

   //Path a Tetgen. Si no existe lo buscamos
   key = "/coindesigner/tetgen_app";
   value = settings.readEntry(key);
   if (!value)
   {
	   QString tetgen_app;

	   //Miramos si existe una variable de entorno adecuada
	   tetgen_app = getenv("TETGEN");
#ifdef _WIN32
		   tetgen_app.replace( QChar('\\'), "\\" );
#endif
   	   if (tetgen_app && QFile::exists(tetgen_app) && !QFileInfo(tetgen_app).isDir() && QFileInfo(tetgen_app).isExecutable())
	   {
			value = tetgen_app;
	   }
	   else
	   {
		   //Buscamos tetgen en el directorio de coindesigner
#ifdef _WIN32
		   tetgen_app.sprintf("%s/tetgen.exe", cds_dir);
		   tetgen_app.replace( QChar('/'), "\\" );
#else
		   tetgen_app.sprintf("%s/tetgen", cds_dir);
#endif

		   if (QFile::exists(tetgen_app) && !QFileInfo(tetgen_app).isDir() && QFileInfo(tetgen_app).isExecutable())
		   {
			   value = tetgen_app;
		   }
		   else
		   {
#ifdef _WIN32
			   tetgen_app = "tetgen.exe";
#else
			   tetgen_app = "tetgen";
#endif
			   //Buscamos tetgen en el path
			   QProcess *proc = new QProcess( NULL );
			   proc->addArgument( tetgen_app );
			   proc->addArgument( "-h" );
			   if (proc->start() ) 
			   {
				   value = tetgen_app;
			   }
			   proc->tryTerminate();
			   delete proc;
		   }
	   }

	   if (value)
	   {
		   //Salvamos el path encontrado
		   std::cerr << "Setting: " << key << "=" << value << std::endl;
		   settings.writeEntry(key, value);
	   }

   }//if (!value)

}
