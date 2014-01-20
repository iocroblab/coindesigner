/*
    This file is part of coindesigner. (https://github.com/jmespadero/coindesigner)

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
        if (argc == 2)
        {
            //Only one argument. Call load_scene
            mw->load_Scene(argv[1]);
        }
        else
        {
            //Several arguments. Call import_File on each argument
            for (int i=1; i<argc; i++)
			{
				printf("%s\n", argv[i]);
				mw->import_File(argv[i]);
			}
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

