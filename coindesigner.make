
#Bison: Opcion -t para activar mensajes "Se esperaba xxx":1
#Bison: Opcion -d para generar el fichero y.tab.h
YFLAGS = -d -t 
#Flex:  Opcion -i para no distinguir entre mayusculas y minusculas
LFLAGS = -i

QHULLLIB_OBJ=tmp/geom2.o tmp/geom.o tmp/global.o tmp/io.o tmp/mem.o tmp/merge.o tmp/poly2.o tmp/poly.o \
             tmp/qhull.o tmp/qhull_interface.o tmp/qhulllib.o tmp/qset.o tmp/stat.o tmp/user.o

CDSLIB_OBJ= tmp/cds_parser.o tmp/cds_scanner.o tmp/3dsLoader.o tmp/SoStream.o

RESOURCES=images/gui.qrc images/nodes.qrc

#Esto debe hacerse desde autoconfig!!!
#PREFIX="/usr/local"
#VERSION="1.3"
#CXXFLAGS+=-DUSE_VOLEON -I`coin-config --includedir` `soqt-config --cppflags` -g -O0
#LDFLAGS+=-lSimVoleon `soqt-config --ldflags --libs`

all : coindesigner cdsview

coindesigner : include/*.h src/*.cpp ui/*.ui images/* coindesigner.pro 
	qmake -o - coindesigner.pro | sed 's/-O2/-O0 -g/g' > Makefile
	#qmake -o - coindesigner.pro > Makefile
	$(MAKE) -f Makefile 
	rm Makefile
	lupdate coindesigner.pro
	lrelease coindesigner.pro

cdsview : tmp/cdsview.o tmp/cds_parser.o tmp/cds_scanner.o tmp/3dsLoader.o tmp/SoStream.o
	$(CXX) $(LDFLAGS) -o $@ $< 

images.pro : images images/gui images/nodes images/gui/* images/nodes/*
	echo -n "IMAGES += " > $@
	echo `ls images/gui/*.png` >> $@
	echo -n "IMAGES += " >> $@
	echo `ls images/nodes/*.png` >> $@

tmp/%.o : %.o
	mkdir -p tmp
	mv $< $@

%.o : %.cpp coindesigner.pro
	$(CXX) -c $(CXXFLAGS) $< 

src/cds_parser.cpp include/cds_parser.h : src/cds_parser.y
	yacc $(YFLAGS) src/cds_parser.y
	mv -f y.tab.c src/cds_parser.cpp
	mv -f y.tab.h include/cds_parser.h

src/cds_scanner.cpp : src/cds_scanner.l include/cds_parser.h
	lex -i src/cds_scanner.l 
	mv lex.yy.c src/cds_scanner.cpp

coindesigner.vcproj : coindesigner.pro
	qmake -t vcapp -o $@ coindesigner.pro
	lupdate coindesigner.pro
	lrelease coindesigner.pro

update_doc:
	wget -r -np -nv -N http://doc.coin3d.org/Coin
	wget -r -np -nv -N http://doc.coin3d.org/SIMVoleon/

clean : 
	qmake coindesigner.pro
	$(MAKE) -f Makefile clean
	rm -rf core Makefile tmp .qmake.internal.cache

distclean : clean 
	qmake coindesigner.pro
	$(MAKE) -f Makefile distclean
	rm -rf coindesigner cdsview Makefile tmp .qmake.internal.cache 




