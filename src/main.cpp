
#include <qapplication.h>
#include <qsettings.h>
#include <qresource.h>
#include <qtranslator.h>
#include <QByteArray>
#include <mainwindow.h>
#include <settingsDialog.h>

///Objeto para almacenar la configuración de coindesigner
QSettings *settings;

///Puntero global a mainwindow
MainWindow *global_mw = NULL;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	//Inicializamos los resources (iconos, etc...)
	Q_INIT_RESOURCE(icons);
	Q_INIT_RESOURCE(demos);

	//Inicializamos el path de la aplicacion
	cds_dir = strdup(qPrintable(QCoreApplication::applicationDirPath()));

	//Abrimos fichero de configuración
	settings = new QSettings("coindesigner.sf.net", "coindesigner");

	//Cargamos la configuracion por defecto, sin sobreescribir valores
	{
		settingsDialog settingDlg;
		settingDlg.setToDefault(false);
	}

	//Leemos el lenguaje del interfaz de los settings
	QString lang(settings->value("lang").toString());
	qDebug("Locale=%s",qPrintable(lang));

	//Traduccion de cadenas de QT
	QTranslator qtTranslator;
	QString qtTranslationDir = QString("%1/translations").arg(getenv("QTDIR"));
	if(qtTranslator.load("qt_" + lang, qtTranslationDir))
		app.installTranslator(&qtTranslator);
	else
		qDebug("Can't load file qt_%s", qPrintable(lang));

	//Traduccion de cadenas de coindesigner
	QTranslator appTranslator;
	if (appTranslator.load("coindesigner_" + lang, "translations"))
		app.installTranslator(&appTranslator);
	else
		qDebug("Can't load file coindesigner_%s", qPrintable(lang));

	//Creamos la ventana principal (esto inicializa SoDB y SoQt)
	MainWindow *mw = new MainWindow();
	global_mw = mw;

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

