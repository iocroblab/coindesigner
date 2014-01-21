cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	./include
	./include/ivfix
	./include/qhulllib
	$$system(coin-config
	--includedir)
	$(COIN3DDIR)/include)
set ( coindesigner_HDRS
	include/cds_util.h
	include/cds_globals.h
	include/3dsLoader.h
	)

set ( coindesigner_SRCS
	main.cpp
	mainwindow.cpp
	mainwindow2.cpp
	cds_util.cpp
	cds_viewers.cpp
	cds_parser.cpp
	cds_scanner.cpp
	cds_globals.cpp
	3dsLoader.cpp
	settingsDialog.cpp
	SoStream.cpp
	src_editor.cpp
	cppexport_options.cpp
	mfield_editor.cpp
	ivfix_options.cpp
	qslim_options.cpp
	tetgen_options.cpp
	IfBuilder.cpp
	IfFlattener.cpp
	IfReplacer.cpp
	IfSorter.cpp
	IfCollector.cpp
	IfHasher.cpp
	IfReporter.cpp
	IfStripper.cpp
	IfCondenser.cpp
	IfHolder.cpp
	IfShape.cpp
	IfTypes.cpp
	IfFixer.cpp
	IfMerger.cpp
	IfShapeList.cpp
	IfWeeder.cpp
	qh_geom.cpp
	qh_io.cpp
	qh_poly.cpp
	qh_stat.cpp
	qh_geom2.cpp
	qh_mem.cpp
	qh_poly2.cpp
	qh_user.cpp
	qhulllib.cpp
	qh_global.cpp
	qh_merge.cpp
	qhull_interface.cpp
	qh_qset.cpp
	qhull.cpp
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
	include/mainwindow.h
	include/src_editor.h
	include/mfield_editor.h
	include/qslim_options.h
	include/cds_viewers.h
	include/cppexport_options.h
	include/ivfix_options.h
	include/settingsDialog.h
	include/tetgen_options.h
	)
QT4_WRAP_CPP(MOCS ${coindesigner_MOCS})

add_executable ( coindesigner ${coindesigner_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( coindesigner  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
