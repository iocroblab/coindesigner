cmake_minimum_required( VERSION 2.6 )

project(coindesigner)

########### Modules path ###############
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)


########### Options ####################
option( CDS_GENERATE_FILES "Regenerate flex and bision/yacc files" ON )


set(CDS_INCLUDES "")
set(CDS_LIBRARIES "")
#set(CDS_INCLUDES ${CDS_INCLUDES} ${})
#set(CDS_LIBRARIES ${CDS_LIBRARIES} ${})

# graphics library
#### Coin3D package ###############
find_package( Coin REQUIRED )
if( COIN_LIBRARY_FOUND )
   message( STATUS "Using Coin library")
   add_definitions( -DCOIN_DLL )
   set(CDS_INCLUDES ${CDS_INCLUDES} ${COIN_INCLUDE_DIR})
   set(CDS_LIBRARIES ${CDS_LIBRARIES} ${COIN_LIBRARY})
   #include_directories( ${COIN_INCLUDE_DIR} )
else( COIN_LIBRARY_FOUND )
   message(SEND_ERROR "coindesigner needs Coin library")
endif(COIN_LIBRARY_FOUND )

#### Assimp ######
find_package( Assimp )
if( ASSIMP_FOUND )
   message( STATUS "assimp Package FOUND")
   set(CDS_LIBRARIES ${CDS_LIBRARIES} ${ASSIMP_LIBRARIES} )
   add_definitions( -DUSE_ASSIMP )
else( ASSIMP_FOUND )
   message( STATUS "assimp Package NOT FOUND")
endif( ASSIMP_FOUND )


##############################################################################
# START Required packages for GUI
find_package( Qt4 REQUIRED QtCore QtGui QtXml )
if( QT4_FOUND )
   include(${QT_USE_FILE})
   add_definitions( ${QT_DEFINITIONS} )
   set(CDS_INCLUDES ${CDS_INCLUDES} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR})
   set(CDS_LIBRARIES ${CDS_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
else ( QT4_FOUND )
   message(SEND_ERROR "coindesigner needs Qt4 library")
endif( QT4_FOUND )

##############################################################################
find_package( SoQt4 REQUIRED )
if( SOQT_LIBRARY_FOUND )
   add_definitions( -DSOQT_DLL -DCOIN_DLL )
   set(CDS_INCLUDES ${CDS_INCLUDES} ${SOQT_INCLUDE_DIR})
   set(CDS_LIBRARIES ${CDS_LIBRARIES} ${SOQT_LIBRARY})
else ( SOQT_LIBRARY_FOUND )
   message(SEND_ERROR "coindesigner needs SoQt4 library")
endif( SOQT_LIBRARY_FOUND )
# END Required packages for GUI
##############################################################################

##############################################################################
find_package( Simage )
if( Simage_FOUND )
   message(STATUS "Simage library found with ${Simage_INCLUDE_DIRS}")
   add_definitions( ${Simage_DEFINITIONS} )
   set(CDS_INCLUDES ${CDS_INCLUDES} ${Simage_INCLUDE_DIRS})
   set(CDS_LIBRARIES ${CDS_LIBRARIES} ${Simage_LIBRARIES})
else ( Simage_FOUND )
   # We have not found a nice way to check simage in all the platforms.
   # We make a trial and error approach
   message(STATUS "coindesigner needs Simage library. CMake has not been able to found it. It will try in a rude way.")
   set(CDS_LIBRARIES ${CDS_LIBRARIES} -lsimage)
endif( Simage_FOUND )

##############################################################################
find_package( Dime ) # Optional
if( Dime_FOUND )
   add_definitions( ${Dime_DEFINITIONS} -DUSE_DIME )
   set(CDS_INCLUDES ${CDS_INCLUDES} ${Dime_INCLUDE_DIRS})
   set(CDS_LIBRARIES ${CDS_LIBRARIES} ${Dime_LIBRARIES})
   message(STATUS "Using the dime lib to import dxf files")
else ( Dime_FOUND )
   message(STATUS "Dime not found, could not work with dxf files")
endif( Dime_FOUND )
##############################################################################


###Pending
#Soporte para representar imagenes volumetricas con la libreria
#SIMVoleon, disponible en http://dev.sim.no/SIM_Voleon/
#Para dar soporte a SIMVoleon, decomentar las siguientes lineas.
#Coindesigner can show volumetrix data using the SIMVoleon library,
#which you can download from http://dev.sim.no/SIM_Voleon/
#To add support to SIMVoleon, uncoment next 3 lines.
#DEFINES += USE_VOLEON SIMVOLEON_DLL
#unix:LIBS += -lSimVoleon
#win32:LIBS += $(COIN3DDIR)\lib\simvoleon2.library

if(CDS_GENERATE_FILES)
   find_package(BISON )
   if(BISON_FOUND)
      set(BisonOutput ${CMAKE_BINARY_DIR}/cds_parser.cpp)
      set(YFLAGS "-t")
      add_custom_command(
         OUTPUT ${BisonOutput}
         COMMAND ${BISON_EXECUTABLE} ${YFLAGS}
               --defines=${CMAKE_BINARY_DIR}/cds_parser.h
               --output=${BisonOutput}
               ${CMAKE_SOURCE_DIR}/src/cds_parser.y
         COMMENT "Generating cds_parser.cpp")
   else(BISON_FOUND)
      set ( CDS_GENERATE_FILES OFF )
      set ( BisonOutput src/cds_parser.cpp)
   endif(BISON_FOUND)

   find_package(FLEX )
   if(FLEX_FOUND)
      set(FlexOutput ${CMAKE_BINARY_DIR}/cds_scanner.cpp)
      add_custom_command(
         OUTPUT ${FlexOutput}
         COMMAND ${FLEX_EXECUTABLE} -i
               --outfile=${FlexOutput}
               ${CMAKE_SOURCE_DIR}/src/cds_scanner.l
         COMMENT "Generating cds_scanner.cpp")
   else(FLEX_FOUND)
      set ( CDS_GENERATE_FILES OFF )
      set ( FlexOutput src/cds_scanner.cpp)
   endif(FLEX_FOUND)
else(CDS_GENERATE_FILES)
   set ( FlexOutput src/cds_scanner.cpp)
   set ( BisonOutput src/cds_parser.cpp)
endif(CDS_GENERATE_FILES)

add_library(cds_parser ${BisonOutput} ${FlexOutput})

set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )
add_definitions(-DCDS_VERSION="2.0")
include_directories ( ${CMAKE_SOURCE_DIR}/src ${CDS_INCLUDES}
                      ${CMAKE_BINARY_DIR})


set ( CDS_HDRS src/cds_util.h
               src/cds_globals.h
               src/3dsLoader.h)

set ( CDS_SRCS ${CDS_SRCS} src/main.cpp
               src/mainwindow.cpp
               src/mainwindow2.cpp
               src/assimp.cpp
               src/cds_util.cpp
               src/cds_viewers.cpp
               #src/cds_scanner.cpp
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
               src/qhulllib/qhull.cpp)

set ( CDS_UIS  ui/mainwindow.ui
               ui/cds_editor.ui
               ui/src_view.ui
               ui/mfield_editor.ui
               ui/cppexport_options.ui
               ui/ivfix_options.ui
               ui/qslim_options.ui
               ui/tetgen_options.ui
               ui/settingsDialog.ui )

QT4_WRAP_UI(UIS ${CDS_UIS})

set ( CDS_RSCS images/icons.qrc
               demos/demos.qrc)
QT4_ADD_RESOURCES(RSCS ${CDS_RSCS})

set ( CDS_TRS  translations/coindesigner_es.ts)
QT4_ADD_TRANSLATION(TRS ${CDS_TRS})

set ( CDS_MOCS src/mainwindow.h
               src/src_editor.h
               src/mfield_editor.h
               src/qslim_options.h
               src/cds_viewers.h
               src/cppexport_options.h
               src/ivfix_options.h
               src/settingsDialog.h
               src/tetgen_options.h)
QT4_WRAP_CPP(MOCS ${CDS_MOCS})

add_executable ( coindesigner ${CDS_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
add_executable ( cdsview src/cdsview.cpp src/cds_scanner.cpp src/cds_parser.cpp )

target_link_libraries ( coindesigner ${CDS_LIBRARIES} cds_parser)
target_link_libraries ( cdsview ${SOQT_LIBRARY} ${COIN_LIBRARY} )

install(TARGETS coindesigner cdsview DESTINATION bin)
