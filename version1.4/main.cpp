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

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <qapplication.h>
#include <qtextcodec.h>
#include <qprocess.h>
#include <qsettings.h>
#include "cds_config.h"
#include "form1.h"
#include "cds_config_form.h"

#ifdef _WIN32
#include <Inventor/Win/SoWin.h>
#include <direct.h>
#else
#include <Inventor/Qt/SoQt.h>
#endif


int main( int argc, char ** argv )
{
 
  QApplication a( argc, argv );

   //Directorio de la aplicacion
    #ifdef _WIN32
  	  cds_dir = getcwd(NULL, 0);
  	#else
        cds_dir = strdup(a.applicationDirPath().ascii());
    #endif

  //Abrimos fichero de configuración
   settings.setPath( "coindesigner.sf.net", "coindesigner", QSettings::User );

   //Miramos si estamos tratando de eliminar la configuración de la aplicacion
   if (argc > 1 && QString(argv[1]).lower() == "--removesettings" )
   {
      //Reset de settings
      cds_removeSettings();
      return 0;
   }

   //Introducimos los valores por defecto
   cds_defaultSettings();


    //Carga del fichero de traduccion
    QString lang_file, lang_dir;

    //Nombre del fichero con el lenguaje segun variable LANG
    lang_file.sprintf("coindesigner_%s", QTextCodec::locale() );

    QTranslator translator( 0 );
    
    //Probamos en el directorio actual
    if (!translator.load(lang_file, "."))
    {
        //Probamos en $PREFIX/coindesigner-$VERSION
#ifndef _WIN32
        lang_dir.sprintf("%s/coindesigner-%s", PREFIX , VERSION );
#endif
        if (!translator.load(lang_file, lang_dir))
        {
          fprintf(stderr,"%s: language file not found\n",lang_file.ascii());
          fprintf(stderr,"Trying %s/coindesigner_en\n",lang_dir.ascii());
          translator.load("coindesigner_en", lang_dir);
	}
    }
    	    

    a.installTranslator( &translator );

    Form1 w;
    w.show();
    w.inicializar();

    //Miramos si hay algun argumento y cargamos el fichero
    if (argc > 1)
    {
        if (argc > 2)
        {
            for (int i=1; i<argc; i++)
            {
              printf("%s\n", argv[i]);
              w.ImportarGeometria(argv[i]);
            }
        }
        else 
        {
            w.CargarEscena(argv[1]);
        }
    }// if (argc > 1)

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

#ifdef _WIN32
    SoWin::mainLoop ();
#else
    SoQt::mainLoop ();
#endif

    return 0;
    return a.exec();
        
}
