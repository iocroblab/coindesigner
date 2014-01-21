
#Bison: Opcion -t para activar mensajes "Se esperaba xxx":1
#Bison: Opcion -d para generar el fichero y.tab.h
YFLAGS = -d -t 
#Flex:  Opcion -i para no distinguir entre mayusculas y minusculas
LFLAGS = -i

QHULLLIB_OBJ=tmp/geom2.o tmp/geom.o tmp/global.o tmp/io.o tmp/mem.o tmp/merge.o tmp/poly2.o tmp/poly.o \
             tmp/qhull.o tmp/qhull_interface.o tmp/qhulllib.o tmp/qset.o tmp/stat.o tmp/user.o

CDSLIB_OBJ= tmp/cds_parser.o tmp/cds_scanner.o tmp/3dsLoader.o tmp/SoStream.o

QMAKE=qmake
RESOURCES=images/gui.qrc images/nodes.qrc

#Esto debe hacerse desde autoconfig!!!
#PREFIX="/usr/local"
#VERSION="2.0"
#CXXFLAGS+=-DUSE_VOLEON -I`coin-config --includedir` `soqt-config --cppflags` -g -O0
#LDFLAGS+=-lSimVoleon `soqt-config --ldflags --libs`

all : coindesigner cdsview

coindesigner : src/*.h src/*.cpp ui/*.ui images/* coindesigner.pro translations
	$(QMAKE) -o - coindesigner.pro > Makefile
	$(MAKE) -f Makefile 
	#rm Makefile

cdsview : src/cdsview.cpp tmp/cds_scanner.o tmp/cds_parser.o
	$(CXX) -o $@ $+ `soqt-config --cppflags --ldflags --libs`

rebuild: distclean
	$(MAKE) -f coindesigner.make all 

tmp/%.o : %.o
	mkdir -p tmp
	mv $< $@

tmp/%.o : src/%.cpp coindesigner.pro
	$(CXX) -c -o $@ $(CXXFLAGS) $< 

src/cds_parser.cpp src/cds_parser.h : src/cds_parser.y
	yacc $(YFLAGS) src/cds_parser.y
	mv -f y.tab.c src/cds_parser.cpp
	mv -f y.tab.h src/cds_parser.h

src/cds_scanner.cpp : src/cds_scanner.l src/cds_parser.h
	lex -i src/cds_scanner.l 
	mv lex.yy.c src/cds_scanner.cpp

coindesigner.vcproj : coindesigner.pro
	$(QMAKE) -t vcapp -o $@ coindesigner.pro

translations : coindesigner.pro 
	lupdate-qt4 coindesigner.pro
	lrelease-qt4 -compress coindesigner.pro

clean : 
	$(QMAKE) coindesigner.pro
	$(MAKE) -f Makefile clean
	$(RM) -f src/cds_parser.o  src/cds_scanner.o  src/cdsview.o
	#$(RM) -rf core Makefile tmp .qmake.internal.cache

distclean : 
	$(QMAKE) coindesigner.pro
	$(MAKE) -f Makefile distclean
	$(RM) -rf coindesigner cdsview core Makefile tmp .qmake.internal.cache 

