

TEMPLATE = app
TARGET = 
DEFINES	+= CDS_VERSION='"2.0"'

#Directorios donde buscar codigo fuente
DEPENDPATH += . 
INCLUDEPATH += ./include ./include/ivfix ./include/qhulllib
VPATH += ./src ./src/ivfix ./src/qhulllib

#Directorios donde almacenar codigo intermedio
UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp
RCC_DIR = tmp

#coindesigner puede importar ficheros DXF si se ha instalado previamente
#la libreria DIME. Disponibles en http://www.coin3d.org/download/
#Para dar soporte a DXF mediante dime, decomentar las siguientes lineas.
#coindesigner can import DXF files if DIME libs are available. You can
#download dime from http://www.coin3d.org/download/ and install them.
#To add support to DXF throught dime, uncomment next 3 lines
#DEFINES += USE_DIME
#unix:LIBS += -ldime
#win32:LIBS += $(COIN3DDIR)\lib\dime.lib

#Soporte para representar imagenes volumetricas con la libreria
#SIMVoleon, disponible en http://dev.sim.no/SIM_Voleon/
#Para dar soporte a SIMVoleon, decomentar las siguientes lineas.
#Coindesigner can show volumetrix data using the SIMVoleon library,
#which you can download from http://dev.sim.no/SIM_Voleon/
#To add support to SIMVoleon, uncoment next 3 lines.
DEFINES += USE_VOLEON SIMVOLEON_DLL
unix:LIBS += -lSimVoleon
win32:LIBS += $(COIN3DDIR)\lib\simvoleon2.lib

# Input
SOURCES +=	main.cpp	mainwindow.cpp	cds_util.cpp	cds_viewers.cpp\
		cds_parser.cpp	cds_scanner.cpp	3dsLoader.cpp	SoStream.cpp \
#TODO	IfBuilder.cpp    IfFlattener.cpp  IfReplacer.cpp   IfSorter.cpp \
#TODO	IfCollector.cpp  IfHasher.cpp     IfReporter.cpp   IfStripper.cpp \
#TODO	IfCondenser.cpp  IfHolder.cpp     IfShape.cpp      IfTypes.cpp \
#TODO	IfFixer.cpp      IfMerger.cpp     IfShapeList.cpp  IfWeeder.cpp \
#TODO	qh_geom.cpp      qh_io.cpp        qh_poly.cpp      qh_stat.cpp  \
#TODO	qh_geom2.cpp     qh_mem.cpp       qh_poly2.cpp     qh_user.cpp  \
#TODO	qhulllib.cpp     qh_global.cpp    qh_merge.cpp     qhull_interface.cpp \
#TODO	qh_qset.cpp      qhull.cpp 
	

HEADERS	+=	include/mainwindow.h include/cds_util.h include/cds_globals.h \
		include/3dsLoader.h \
		include/qslim_options.h include/cds_viewers.h 

FORMS	=	ui/mainwindow.ui \
		ui/cds_editor.ui \
		ui/src_view.ui \
		ui/qslim_options4.ui
			

RESOURCES = images/icons.qrc demos/demos.qrc

#TODO TRANSLATIONS = translations/coindesigner_es.ts

#TODO INSTALL = test

unix {
  INCLUDEPATH += $$system(coin-config --includedir)
  #SoQt3 LIBS  += $$system(/usr/bin/soqt-config --ldflags --libs)
  LIBS  += $$system(/usr/local/bin/soqt-config --ldflags --libs)
  INSTALL += coindesigner
}

win32 {
  DEFINES += COIN_DLL SOQT_DLL
  INCLUDEPATH += "$(COIN3DDIR)\include"
  LIBS += $(COIN3DDIR)\lib\simage1.lib $(COIN3DDIR)\lib\coin2.lib $(COIN3DDIR)\lib\soqt1.lib
  INSTALL+=Release/coindesigner.exe
}
