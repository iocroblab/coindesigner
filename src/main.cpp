
#include <qapplication.h>
#include "qslim_options.h"
#include "cds_view.h"
#include "mainwindow.h"
#include <qsettings.h>
#include <qresource.h>
#include <QByteArray>

///Objeto para almacenar la configuración de coindesigner
QSettings *settings;

int main(int argc, char *argv[])
{
	//Inicializamos los resources (iconos, etc...)
	Q_INIT_RESOURCE(icons);
	Q_INIT_RESOURCE(demos);

	QApplication app(argc, argv);

	//Abrimos fichero de configuración
	settings = new QSettings("coindesigner.sf.net", "coindesigner");

	//TODO: Configuracion de verdad
	settings->setValue("tetgen_app", "tetgen.exe");
#ifdef _WIN32
	settings->setValue("helpViewer_app", "explorer.exe");
#else
	settings->setValue("helpViewer_app", "firefox");
#endif
	settings->setValue("reference_dir", "http://coindesigner.sf.net/reference");

	//Creamos la ventana principal (esto inicializa SoDB y SoQt)
	MainWindow *mw = new MainWindow();

	//Miramos si hay algun argumento y cargamos el fichero
	if (argc > 1)
	{
		if (argc > 2)
		{
			for (int i=1; i<argc; i++)
			{
				printf("%s\n", argv[i]);
				mw->import_File(argv[i]);
			}
		}
		else 
		{
			mw->load_Scene(argv[1]);
		}
	}// if (argc > 1)

	//Mostramos la ventana principal 
	mw->show();

	//Entramos en el mainLoop de la aplicación
	SoQt::mainLoop();

	//Liberamos memoria
	delete mw;
	delete settings;

	return 0;
}

