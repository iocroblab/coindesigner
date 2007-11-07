TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

DEFINES	+= VERSION='"1.4"'

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


#ATENCION, esto debe hacerse desde autoconfig!!!
DEFINES += PREFIX='"/usr/local"'

HEADERS	+= cds_config.h \
	cds_viewers.h \
	cds_util.h \
	cdsFileDialog.h \
	tetgen_options.h \
	cds_config_form.h \
	3dsLoader.h \
	SoStream.h \
	IfAssert.h \
	IfBuilder.h \
	IfCollector.h \
	IfCondenser.h \
	IfFixer.h \
	IfFlattener.h \
	IfHasher.h \
	IfHolder.h \
	IfMerger.h \
	IfReplacer.h \
	IfReporter.h \
	IfShape.h \
	IfShapeList.h \
	IfSorter.h \
	IfStripper.h \
	IfTypes.h \
	IfWeeder.h \
	qh_geom.h \
	qh_io.h \
	qh_mem.h \
	qh_merge.h \
	qh_poly.h \
	qhull.h \
	qhulllib.h \
	qh_qset.h \
	qh_stat.h \
	qh_user.h


SOURCES	+= main.cpp \
	cds_config.cpp \
	cds_parser.cpp \
	cds_scanner.cpp \
	cds_viewers.cpp \
	cds_util.cpp \
	tetgen_options.cpp \
	cds_config_form.cpp \
	3dsLoader.cpp \
	SoStream.cpp \
	IfBuilder.cpp \
	IfCollector.cpp \
	IfCondenser.cpp \
	IfFixer.cpp \
	IfFlattener.cpp \
	IfHasher.cpp \
	IfHolder.cpp \
	IfMerger.cpp \
	IfReplacer.cpp \
	IfReporter.cpp \
	IfShape.cpp \
	IfShapeList.cpp \
	IfSorter.cpp \
	IfStripper.cpp \
	IfTypes.cpp \
	IfWeeder.cpp \
	qh_geom2.cpp \
	qh_geom.cpp \
	qh_global.cpp \
	qh_io.cpp \
	qh_mem.cpp \
	qh_merge.cpp \
	qh_poly2.cpp \
	qh_poly.cpp \
	qhull.cpp \
	qhull_interface.cpp \
	qhulllib.cpp \
	qh_qset.cpp \
	qh_stat.cpp \
	qh_user.cpp



FORMS	= ./form1.ui \
	mfield_editor.ui \
	matrix_editor.ui \
	ivfix_options.ui \
	cppexport_options.ui \
	qslim_options.ui

IMAGES	= images/bottom.png \
	images/down.png \
	images/edit.png \
	images/editclear.png \
	images/editcopy.png \
	images/editcut.png \
	images/editdelete.png \
	images/editshred.png \
	images/edittrash.png \
	images/exit.png \
	images/fileclose.png \
	images/filefind.png \
	images/fileimport.png \
	images/fileopen.png \
	images/fileprint.png \
	images/filequickprint.png \
	images/filesaveas.png \
	images/gear.png \
	images/help.png \
	images/redo.png \
	images/save_all.png \
	images/stop.png \
	images/top.png \
	images/undo.png \
	images/up.png \
	images/editpaste.png \
	images/filenew.png \
	images/filesave.png \
	images/toggle_log.png \
	images/attach.png \
	images/txt.png \
	images/launch.png \
	images/colorize.png \
	images/rebuild.png \
	images/reload.png \
	images/clock.png \
	images/package_utilities.png \
	images/compfile.png 

include (inventor-shapes.pro)

  UI_DIR = tmp
  MOC_DIR = tmp
  OBJECTS_DIR = tmp

unix {
  #Soporte para version cvs de 2.5.0
  #INCLUDEPATH += /home/jespa/Coin3D/Coin-2.5.0/coin-build/include
  #LIBS  += -L/home/jespa/Coin3D/Coin-2.5.0/coin-build/src/.libs
  INCLUDEPATH += $$system(coin-config --includedir)
  LIBS  += $$system(soqt-config --ldflags --libs)
}

win32 {
  DEFINES += COIN_DLL SOWIN_DLL
  DEFINES += VERSION=\"1.3\"
  INCLUDEPATH += "$(COIN3DDIR)\include"
  LIBS += $(COIN3DDIR)\lib\simage1.lib $(COIN3DDIR)\lib\coin2.lib $(COIN3DDIR)\lib\sowin1.lib qtmain.lib qui.lib qt-mt3.lib
}

TRANSLATIONS = coindesigner_en.ts coindesigner_es.ts 

