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



//openInventor includes
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/SoInteraction.h>

#ifdef WIN32
  //SoWin includes
  #include <Inventor/Win/SoWin.h>
  #include <Inventor/Win/viewers/SoWinExaminerViewer.h>
  #include <Inventor/Win/viewers/SoWinFlyViewer.h>
  #include <Inventor/Win/viewers/SoWinPlaneViewer.h>

  //Definition
  HWND mainwin = NULL;
  SoWinRenderArea *viewer = NULL;
  #define SoExaminerViewer SoWinExaminerViewer
  #define SoFlyViewer SoWinFlyViewer
  #define SoPlaneViewer SoWinPlaneViewer
  #define SoRenderArea SoWinRenderArea

#else

  //SoQt includes
  #include <qwidget.h>
  #include <Inventor/Qt/SoQt.h>
  #include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
  #include <Inventor/Qt/viewers/SoQtFlyViewer.h>
  #include <Inventor/Qt/viewers/SoQtPlaneViewer.h>

  //Definition
  QWidget* mainwin = NULL;
  SoQtRenderArea *viewer = NULL;
  #define SoExaminerViewer SoQtExaminerViewer
  #define SoFlyViewer SoQtFlyViewer
  #define SoPlaneViewer SoQtPlaneViewer
  #define SoRenderArea SoQtRenderArea
#endif

#ifdef USE_DIME
  #include <dime/Input.h>
  #include <dime/Output.h>
  #include <dime/Model.h>
  #include <dime/State.h>
  #include <dime/convert/convert.h>
#endif

#ifdef USE_VOLEON
  #include <VolumeViz/nodes/SoTransferFunction.h>
  #include <VolumeViz/nodes/SoVolumeData.h>
  #include <VolumeViz/nodes/SoVolumeRender.h>
  #include <VolumeViz/nodes/SoVolumeRendering.h>
#endif

/*************DEFINICION DEL TIPO YYSTYPE PARA EL PARSER****************/
#define YYSTYPE_IS_DECLARED 1
typedef union { 
        int entero;
        float real;
        char * pchar;
      } YYSTYPE;

/******************DEFINICION DE LAS FUNCIONES YY DEL PARSER**************/
void yyerror(char *s);
int yylex (void);
int yyparse (void);
void yyrestart (FILE *input_file);

/************** Variables de comunicaci�n con el parser *******************/

/* Fichero del que se leeran los datos a cargar */
extern FILE *yyin;

/* Variable que contiene el numero de linea actual. Mantenida en scanner.l */
extern unsigned yylinenum;

/* Separator de donde colgamos el resultado de la lectura del parser*/
extern SoSeparator * yyGeometry;



//The one an only... root node
SoSeparator * root = NULL;

//Background grey level
float bgColor = 0.0f;

//Filename where dump the scene if necessary
char *out_filename = NULL;

//Enable FPS profiler
bool use_profile = false;

//Enable FullScreen
bool fullScreen = false;

//Enable Decoration
bool decoration = true;

//Render interval for profiling
#define RENDER_INTERVAL 0.0001

typedef enum {ExaminerViewer, FlyViewer, PlaneViewer, RenderArea} ViewerType;
ViewerType viewerType = ExaminerViewer;

static void
renderCallback( void * , SoSensor * )
{
  viewer->render();
} // renderCallback()


//Funcion que sustituye el visor actual por un ExaminerViewer
void nuevoSoExaminerViewer()
{

    //if (mainwin) delete mainwin;
    //mainwin = new QWidget(NULL, "Viewer");

    SoExaminerViewer * eviewer = new SoExaminerViewer (mainwin);
    eviewer->setSceneGraph (root);
    eviewer->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
    eviewer->setBackgroundColor(SbColor(bgColor,bgColor,bgColor));
	eviewer->setFullScreen(fullScreen);
	eviewer->setDecoration(decoration);
    eviewer->setTitle ("cdsView - ExaminerViewer");
    eviewer->show ();
    viewer = eviewer;
}

//Funcion que sustituye el visor actual por un FlyViewer
void nuevoSoFlyViewer()
{
    //if (mainwin) delete mainwin;
    //mainwin = new QWidget(NULL, "Viewer");

    SoFlyViewer * eviewer = new SoFlyViewer (mainwin);
    eviewer->setSceneGraph (root);
    eviewer->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
    eviewer->setBackgroundColor(SbColor(bgColor,bgColor,bgColor));
	eviewer->setFullScreen(fullScreen);
	eviewer->setDecoration(decoration);
    eviewer->setTitle ("cdsView - FlyViewer");
    eviewer->show ();
    viewer = eviewer;
}

//Funcion que sustituye el visor actual por un PlaneViewer
void nuevoSoPlaneViewer()
{
    //if (mainwin) delete mainwin;
    //mainwin = new QWidget(NULL, "Viewer");

    SoPlaneViewer * eviewer = new SoPlaneViewer (mainwin);
    eviewer->setSceneGraph (root);
    eviewer->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
    eviewer->setBackgroundColor(SbColor(bgColor,bgColor,bgColor));
	eviewer->setFullScreen(fullScreen);
	eviewer->setDecoration(decoration);
    eviewer->setTitle ("cdsView - PlaneViewer");
    eviewer->show ();
    viewer = eviewer;
}

//Funcion que sustituye el visor actual por un RenderArea
void nuevoSoRenderArea()
{
    //if (mainwin) delete mainwin;
    //mainwin = new QWidget(NULL, "Viewer");

    SoRenderArea * eviewer = new SoRenderArea (mainwin);
    eviewer->setSceneGraph (root);
    eviewer->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
    eviewer->setBackgroundColor(SbColor(bgColor,bgColor,bgColor));
	eviewer->setFullScreen(fullScreen);
    eviewer->setTitle ("cdsView - PlaneViewer");
    eviewer->show ();
    viewer = eviewer;
}


void ayuda()
{
        fprintf (stderr, 
  "\n"
  "cdsview is a 3D viewer for various file formats (mainly .iv  and .wrl)\n"
  "This program is part of coindesigner (https://github.com/jmespadero/coindesigner)\n"
  "Usage: cdsview [options] file [file ...]\n"
  "Options:\n"
  "  -e , --examiner : View on ExaminerViewer\n"
  "  -f , --fly      : View on FlyViewer\n"
  "  -p , --plane    : View on PlaneViewer\n"
  "  -r , --render   : View on RenderArea\n"
  "  -pg, --prof     : Enable render profiler\n"
  "  -bg greyLevel   : Change background grey level\n"
  "  --fullscreen    : Run in fullscreen mode\n"
  "  --no_decoration : Hide decoration (coin3D controls)\n"
  "  -o filename     : Dump scene file in .iv format and exit\n"
  );

}

int main(int argc, char ** argv)
{
    //Comprobamos el numero de argumentos
    if (argc < 2)
    {
        ayuda();
        return -1;
    }

    SoInput * input;

    // Inicializacion de openInventor
    SoDB::init();
    SoInteraction::init();
    
    #ifdef USE_VOLEON
        SoVolumeRendering::init();
    #endif

    // Creamos el nodo raiz
    root = new SoSeparator;
    root->ref();

    int i;
	for (i=1; i<argc; i++)
	{
		//fprintf(stderr, "i=%d; argv[i]=%s\n", i, argv[i] );

		// Pantalla de ayuda 
		if (!strcmp("-h",argv[i]))
		{
			ayuda();
			return 0;
		}

		else if (!strcmp("-e", argv[i]) || !strcmp ("--examiner", argv[i]) )
		{
			viewerType = ExaminerViewer;
		}

		else if (!strcmp("-f", argv[i]) || !strcmp ("--fly", argv[i]) )
		{
			viewerType = FlyViewer;
		}

		else if (!strcmp("-p", argv[i]) || !strcmp ("--plane", argv[i]) )
		{
			viewerType = PlaneViewer;
		}

		else if (!strcmp("-r", argv[i]) || !strcmp ("--render", argv[i]) )
		{
			viewerType = RenderArea;
		}

		else if (!strcmp("-pg", argv[i]) || !strcmp ("--prof", argv[i]) )
		{
			use_profile = true;
		}

		//Background grey color
		else if (!strcmp("-bg", argv[i]) )
		{
			//Try to read a grey value
			if (!argv[i+1] || !sscanf(argv[i+1], "%f", &bgColor) )
			{
				fprintf(stderr, "Unexpected grey level value (%s)\n", argv[i+1]);
			}

			//Jump to next argument
			i = i + 1;
		}

		//Run in Full Screen
		else if (!strcmp ("--fullscreen", argv[i]) )
		{
			fullScreen = true;
		}

		//Hide decoration
		else if (!strcmp ("--no_decoration", argv[i]) )
		{
			decoration = false;
		}

		//Dump scene instead of render it
		else if (!strcmp("-o", argv[i]) )
		{
			//Try to read a grey value
			if ( !argv[i+1] )
			{
				fprintf(stderr, "You must indicate a filename to output the scene\n");
			}

			//save the filename
			out_filename = argv[i+1];

			//Jump to next argument
			i = i + 1;
		}

		else
		{
			fprintf (stderr, "Loading %s\n", argv[i]);
                        bool fileOk=false;

			input = new SoInput();
			if (!input->openFile (argv[i]))
			{
				perror (argv[i]);
				exit (-1);
			}

			//Comprobamos que es un fichero v�lido de openInventor
			if (input->isValidFile ())
			{
				//Colgamos el fichero del nodo root
				root->addChild (SoDB::readAll (input));
				fileOk = true;
			}
			else
                        {
				//Probamos nuestro parser interno (OOGL, OBJ, XYZ, SMF, etc...)

				//Abrimos el fichero de entrada
#ifdef _MSC_VER
				fopen_s(&yyin, argv[i], "r");
#else
				yyin = fopen( argv[i], "r" );
#endif

				//Comprobamos que se pudo abrir correctamente
				if (!yyin) 
				{
					perror(argv[i]);
					exit (-1);
				}
				//Comprobamos si es un fichero que podemos leer con cds_parser
				rewind (yyin);
				yyrestart(yyin);
				if (yyparse()==0)
				{
					//Colgamos el fichero del nodo root
					root->addChild (yyGeometry);
					fileOk = true;
				}

				//Cerramos el fichero
				fclose(yyin);
                        }

			//Cerramos el fichero
			input->closeFile ();
			delete input;

                        if (fileOk==false)
			{
                fprintf(stderr, "%s: Unknown file format.\n", argv[i]);
				return -1;
			}
		}
	} // for (i=1; i<argc; i++)

	//If we choose to dump scene instead of render it...
	if (out_filename)
	{
		//Escribimos el nombre de fichero
		fprintf(stderr, "Writing file %s\n", out_filename);

		//Creamos un SoWriteActon para escribir la escena
		SoWriteAction writeAction;
		writeAction.getOutput()->openFile(out_filename);
		writeAction.apply(root);
		writeAction.getOutput()->closeFile();

		// Clean up resources.
		if (viewer)
			delete viewer;
		root->unref();
		return 0;
	}


    // Inicializacion del sistema de ventanas
    #ifdef WIN32
        mainwin = SoWin::init(argv[0]);
    #else
        mainwin = SoQt::init (argv[0]);
    #endif

    //Creamos el viewer adecuado
    switch (viewerType)
    {
      case ExaminerViewer: nuevoSoExaminerViewer(); break;
      case FlyViewer: nuevoSoFlyViewer(); break;
      case PlaneViewer: nuevoSoPlaneViewer(); break;
      case RenderArea: nuevoSoRenderArea(); break;
      default : nuevoSoExaminerViewer(); break;
    }
  
  if (use_profile)
  {
      //Active render on demand
      viewer->setAutoRedraw(FALSE);
      SoTimerSensor * renderTimer = new SoTimerSensor(renderCallback, NULL );
      renderTimer->setInterval( RENDER_INTERVAL );
      renderTimer->schedule();

      //Show FPS
      putenv((char *)"COIN_SHOW_FPS_COUNTER=1");
  }


#ifdef WIN32
    SoWin::show(mainwin);
    SoWin::mainLoop();
#else
    SoQt::show(mainwin);
    SoQt::mainLoop();
#endif

    // Clean up resources.
    delete viewer;
    root->unref();

    return 0;
  }

