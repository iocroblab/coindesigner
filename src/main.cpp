
#include <qapplication.h>
#include <qsettings.h>
#include <qresource.h>
#include <QByteArray>

#include <mainwindow.h>
#include <settingsDialog.h>

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

	//Cargamos la configuracion por defecto, sin sobreescribir valores
	{
		settingsDialog settingDlg;
		settingDlg.setToDefault(false);
	}

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

