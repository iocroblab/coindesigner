cmake_minimum_required( VERSION 2.6 )

########### Modules path ###############
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

#### Coin3D package ###############
# graphics library
find_package( Coin REQUIRED )
if( COIN_LIBRARY_FOUND )
   message( STATUS "Using Coin library")
   add_definitions( -DCOIN_DLL )
   include_directories( ${COIN_INCLUDE_DIR} )
else( COIN_LIBRARY_FOUND )
   message(SEND_ERROR "coindesigner needs Coin library")
endif(COIN_LIBRARY_FOUND )


##############################################################################
# START Required packages for GUI
find_package( Qt4 REQUIRED QtCore QtGui QtXml )
if( QT4_FOUND )
   include(${QT_USE_FILE})
   add_definitions( ${QT_DEFINITIONS} )
else ( QT4_FOUND )
   message(SEND_ERROR "coindesigner needs Qt4 library")
endif( QT4_FOUND )

##############################################################################
find_package( SoQt4 REQUIRED )
if( SOQT_LIBRARY_FOUND )
   add_definitions( -DSOQT_DLL -DCOIN_DLL )
   include_directories( ${SOQT_INCLUDE_DIR} )
else ( SOQT_LIBRARY_FOUND )
   message(SEND_ERROR "coindesigner needs SoQt4 library")
endif( SOQT_LIBRARY_FOUND )
# END Required packages for GUI
##############################################################################

##############################################################################
find_package( simage REQUIRED )
if( Simage_FOUND )
   add_definitions( ${Simage_DEFINITIONS} )
   include_directories( ${Simage_INCLUDE_DIRS} )
else ( Simage_FOUND )
   message(SEND_ERROR "coindesigner needs Simage library")
endif( Simage_FOUND )
# END Required packages for GUI
##############################################################################


#  Simage_FOUND          - Set to true when Simage is found
#  Simage_VERSION_STRING - The Simage version number
#  Simage_INCLUDE_DIRS   - Include directories for Simage
#  Simage_LIBRARY_DIRS   - Link directories for Simage   
#  Simage_LIBRARIES      - Libriaries that must be linked
#  Simage_LIBRARY        - Same as Simage_LIBRARIES
#  Simage_DEFINITIONS    - Define flags to the compiler




set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )
add_definitions(-DCDS_VERSION="2.0")
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	./src
	./src/ivfix
	./src/qhulllib
	$$system(coin-config
	--includedir)
	$(COIN3DDIR)/include)

set ( coindesigner_HDRS
	src/cds_util.h
	src/cds_globals.h
	src/3dsLoader.h
	)

set ( coindesigner_SRCS
	src/main.cpp
	src/mainwindow.cpp
	src/mainwindow2.cpp
	src/cds_util.cpp
	src/cds_viewers.cpp
	src/cds_parser.cpp
	src/cds_scanner.cpp
	src/cds_globals.cpp
	src/3dsLoader.cpp
	src/settingsDialog.cpp
	src/SoStream.cpp
	src/src_editor.cpp
	src/cppexport_options.cpp
	src/mfield_editor.cpp
	src/ivfix_options.cpp
	src/qslim_options.cpp
	src/tetgen_options.cpp
	src/ivfix/IfBuilder.cpp
	src/ivfix/IfFlattener.cpp
	src/ivfix/IfReplacer.cpp
	src/ivfix/IfSorter.cpp
	src/ivfix/IfCollector.cpp
	src/ivfix/IfHasher.cpp
	src/ivfix/IfReporter.cpp
	src/ivfix/IfStripper.cpp
	src/ivfix/IfCondenser.cpp
	src/ivfix/IfHolder.cpp
	src/ivfix/IfShape.cpp
	src/ivfix/IfTypes.cpp
	src/ivfix/IfFixer.cpp
	src/ivfix/IfMerger.cpp
	src/ivfix/IfShapeList.cpp
	src/ivfix/IfWeeder.cpp
	src/qhulllib/qh_geom.cpp
	src/qhulllib/qh_io.cpp
	src/qhulllib/qh_poly.cpp
	src/qhulllib/qh_stat.cpp
	src/qhulllib/qh_geom2.cpp
	src/qhulllib/qh_mem.cpp
	src/qhulllib/qh_poly2.cpp
	src/qhulllib/qh_user.cpp
	src/qhulllib/qhulllib.cpp
	src/qhulllib/qh_global.cpp
	src/qhulllib/qh_merge.cpp
	src/qhulllib/qhull_interface.cpp
	src/qhulllib/qh_qset.cpp
	src/qhulllib/qhull.cpp
	)

set ( coindesigner_UIS
	ui/mainwindow.ui
	ui/cds_editor.ui
	ui/src_view.ui
	ui/mfield_editor.ui
	ui/cppexport_options.ui
	ui/ivfix_options.ui
	ui/qslim_options.ui
	ui/tetgen_options.ui
	ui/settingsDialog.ui
	)
QT4_WRAP_UI(UIS ${coindesigner_UIS})

set ( coindesigner_RSCS
	images/icons.qrc
	demos/demos.qrc
	)
QT4_ADD_RESOURCES(RSCS ${coindesigner_RSCS})

set ( coindesigner_TRS
	translations/coindesigner_es.ts
	)
QT4_ADD_TRANSLATION(TRS ${coindesigner_TRS})

set ( coindesigner_MOCS
	src/mainwindow.h
	src/src_editor.h
	src/mfield_editor.h
	src/qslim_options.h
	src/cds_viewers.h
	src/cppexport_options.h
	src/ivfix_options.h
	src/settingsDialog.h
	src/tetgen_options.h
	)
QT4_WRAP_CPP(MOCS ${coindesigner_MOCS})

add_executable ( coindesigner ${coindesigner_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( coindesigner  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
					${COIN_LIBRARY} ${SOQT_LIBRARY} ${Simage_LIBRARIES})
