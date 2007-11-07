
COINDESIGNER
Written by Jose M. Espadero and Tomas Aguado (2004)

This set or programs is licensed under the GPL, please read the
file GPL.txt for more details about the license.

Please, note that executable files are also affected by the licenses
of QT and Coin3D. Please, refer to their respectives licenses for
more details.

Please, note that the coin3D reference documentation is covered by the 
coin3D license. Visit http://coin3d.org for more details.

Please, don't forget to read the Acknowledges section at the end of
this document. This program is based in the efford of many else people
other than the authors.


DESCRIPTION

Coindesigner is a RAD (rapid application development) system to build 3D 
applications using openInventor. It allows to write simple 3D scenarios
just using drag&drop, so users require no programming acknoledges at all
to use it.

coindesigner is a system equivalent to glade or qt-designer, where we can
choose components from the openInventor toolkit, add them somewhere in our
scene and configure them in a very easy way... and all changes are applied
to the scene on the fly!

The project includes cdsview, a multiplatform external viewer that allows
to distribute the scenarios without need of the RAD in most modern OS.

Some characteristics of the tool are:

    * Reads and write scenes in native openInventor file format
    * Can import geometry from VRML, 3DS, OFF (from geomview), SMF, OBJ,
       .sph and XYZ point clouds file formats. 
    * Can save in openInventor format (.iv) and export to VRML2, SMF,
      OBJ, STL, OFF and XYZ file formats
    * It includes cdsview, an minimalistic external viewer that allows
      you to view and distribute scenes without need of coindesigner.
    * Allows to use manips to configure SpotLights, PointLights and 
      Transform nodes. Just use the right mouse button to interchange 
      between those nodes and his manip!
    * GUI has been translated to English, Spanish and Portuguese
    * Includes the complete coin reference and several step by step
      tutorials directly brownsables from the application.
    * Allow some operations as mesh reduction (with QSLIM), scene
      simplification (ivfix), mesh triangulation and convex hull
      computation.
    * Support for volume rendering using SIM Voleon.
    * It's open source released under GPL license. 

COIN3D REFERENCE AND TUTORIALS

The coin3d reference and the step by step tutorials of coindesigner
are now packed in separate files. You can download them from the
downloads section in http://coindesigner.sourceforge.net and
unpack in the same directory where the executable of coindesigner is.

BINARIES FOR WINDOWS

If this package contains the binaries for windows, then you can directly
execute coindesigner.exe. The source code can be downloaded from the
http://coindesigner.sf.net website.

Default languaje is spanish. If you want to see coindesigner in your own
languaje then you have to define the LANG enviroment variable to be one of
EN (for English), ES (for Spanish) or PT (for Portuguese) and then run 
coindesigner. Please, ensure that the translation files (*.qs) are found 
in the current directory.

Please, note that this package contains some .dll files distributed with coin
(simage1.dll , coin2.dll and sowin1.dll). You can download source code and
upgrade those files in http://www.coin3d.org. Note that those files are covered
by the coin3d license, which you can read in the above website.
 
Please, note that this package contains a .dll file distributed with qt-free
(qt-mt3.dll). You can download source code and upgrade this file in the
http://kde-cygwin.sourceforge.net. Note that this file is covered by the GPL
license, you can obtain inside this package.

BUILDING FROM THE SOURCES (LINUX)

You will need this libraries installed and working

 * Qt version 3.2.0 or newer. Usually shipped with most Linux distributions
   today. Visit http://www.trolltech.com if you need to install it.

 * Simage, Coin3D and SoQt. Please visit http://coin3d.org to read how to
   download and install these libraries.


Please, ensure that the "soqt-config" command is in the path. This is
usually installed by SoQt library.

To build coindesigner and cdsview 

1) Uncompress the coindesigner-x.x.tgz package

   tar xvfz coindesigner-x.x.tgz

2) Go to the new directory
  
   cd coindesigner-x.x

3) Run the gmake comand
   
   gmake


This will create two executable files:

* Coindesigner: The RAD system

* cdsview: The standalone multiplatform viewer


To use the translation files, you must run coindesigner in the 
same directory that the coindesigner_your_language.qm file. Please
ensure that the LANG enviroment variable is adecuate to your language.


BUILDING FROM THE SOURCES (WIN32)

You will need this libraries installed and working

 * Qt version 3.2.0 or newer. Visit http://www.trolltech.com to purchase a
   license of QT, or visit http://kde-cygwin.sourceforge.net to download
   and compile the free version of qt.

 * Simage, Coin3D and SoQt. Please visit http://coin3d.org to read how to
   download and install these libraries. 

 * A C++ compiler, which could be Visual Studio 6.0 , Visual Studio .net
   CYGWIN, or any other compiler. Please refer to QT documentation to see 
   what compilers are supported.

To build coindesigner from sources

1) Uncompress the coindesigner-x.x.tgz package

2) Open a command shell and go to the new directory
  
3) Ensure you have working values for the enviroment variables QTDIR
   (where you installed Qt), COIN3DDIR (where you installed Coin3d)

Note: If you are using MS Visual C++, you can try at this point one
of the .vcproj files provided with the package, and try to build coindesigner
from the IDE instead of using qmake and nmake.

4) Ensure you have defined the QMAKESPEC variable. This variable tells
   qmake which C++ compiler you are going to use. Valid values are
   "win32-msvc", "win32-msvc.net", "win32-g++", etc... refer to QT 
   documentation to assign the value to this variable.

5) Ensure that qmake and your compiler are in the path.
   
6) Execute "qmake -win32"

7) Execute make (or nmake, or gmake)


QSLIM

The mesh reduction tool is based in qslim, an algorithm designed and 
coded by Michael Garland. To use qslim, you must download it from the web
http://graphics.cs.uiuc.edu/~garland/software/qslim.html and put the 
executable of QSLIM in the same directory of coindesigner or elsewhere
in the path. You will see if QSLIM is available in the "help->about" menu.

Note: If you don't want to compile qslim, can download binaries from here:
http://graphics.cs.uiuc.edu/~garland/software/qslim20.html

ACKNOWLEDGES

The 3ds importer is part of coin3d package and was developed by PC John.
This piece of code is usually available when you compile coin3D with
the "--enable-3ds" flags, which is not the default. To avoid everybody 
to have to recompile coin3d, we decided to incorporate those file to
our project

Most icons used in coindesigner are part of Crystal Icons. This icon 
theme was created by Everaldo and is now developed by him in association
with the Crystal Artists Staff. See http://linuxcult.com/node/10

The ivfix code used inside of coindesigner was originally written by 
the SGI crew, and released as GPL code in 2000. 

The qslim program was written by Michael Garland and released under the 
GPL. The web of the author is http://graphics.cs.uiuc.edu/~garland/

The convex hull calculator is based in QHullLib, which was writen by 
John W. Ratcliff. It is an adaptation of the qhull program to allow 
an easy way to use qhull routines using C++, without need to call any
external program. It's not easy to find the standalone qhulllib code
(due broken links :-( ), but it can be found inside ODFRocket, an open
framework which has Copyright (c) 2004 Open Dynamics Framework Group.
Please, read README_qhulllib.txt and README_qhull.txt for more details.

