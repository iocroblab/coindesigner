
TEMPLATE = app
TARGET = 
DEFINES	+= CDS_VERSION='"2.0"'

#Directorios donde buscar codigo fuente
DEPENDPATH += . 
INCLUDEPATH += ./include 
VPATH += ./src

#Directorios donde almacenar codigo intermedio
UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp
RCC_DIR = tmp

# Input
SOURCES =	cdsview.cpp	cds_parser.cpp	cds_scanner.cpp	
FORMS	=
RESOURCES = 
TRANSLATIONS = 

unix {
  INCLUDEPATH += $$system(coin-config --includedir)
  LIBS  += -lsimage
  #SoQt4 LIBS
  LIBS  += $$system(soqt-config --ldflags --libs)
  #LIBS += -lQuarter
  INSTALL += cdsview
}

win32 {
  DEFINES += COIN_DLL SOWIN_DLL SIMAGE_DLL
  #CONFIG += embed_manifest_exe
  CONFIG -= qt
  INCLUDEPATH += "$(COIN3DDIR)/include"
  LIBS += $(COIN3DDIR)\lib\simage1.lib $(COIN3DDIR)\lib\coin3.lib $(COIN3DDIR)\lib\sowin1.lib
  INSTALL+=Release/cdsview.exe
}

