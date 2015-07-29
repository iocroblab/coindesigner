Coindesigner
============

Coindesigner is a RAD (rapid application development) system to build 3D applications 
using Coin3D openInventor. It allows to write simple 3D scenarios just using 
drag&drop, so users require no programming skills to use it. 

Coindesiginer was originally written by Jose M. Espadero and Tomas Aguado (2004-2008).
This version is a fork made by the people from Institute of Industrial and Control 
Engineering - Technical University of Catalonia. Mainly the fork has incorporated:
- CMake as build system
- Import files using Assimp library
- Can save the model in pointcloud format PCD
- Generation of deb package

This set or programs is licensed under the GPL, please read the
file GPL.txt for more details about the license.

Please, note that executable files are also affected by the licenses
of QT and Coin3D. Please, refer to their respective licenses for
more details.

Please, note that the coin3D reference documentation is covered by the 
coin3D license. Visit http://coin3d.org for more details.

Please, don't forget to read the Acknowledges section at the end of
this document. This program is based in the effort of many else people
other than the authors.


DESCRIPTION

coindesigner is a system equivalent to glade or Qt-designer, where we can
choose components from the openInventor toolkit, add them somewhere in our
scene and configure them in a very easy way... and all changes are applied
to the scene on the fly!

The project includes cdsview, a multiplatform external viewer that allows
to distribute the scenarios without need of the RAD in most modern OS.

Some characteristics of the tool are:

    * Reads and write scenes in native openInventor file format (.iv)
    * Can import geometry from VRML, 3DS, OFF (from geomview), SMF, 
        alias/wavefront(.obj), sphereTree (.sph) and XYZ point clouds file formats. 
    * Can import all the files that can parse the Assimp library (http://assimp.sourceforge.net/main_features_formats.html)
    * Can save in openInventor format (.iv) and export to VRML2, SMF,
      OBJ, STL, OFF and XYZ file formats, and PCD.
    * It includes cdsview, an minimalistic external viewer that allows
      you to view and distribute scenes without need of coindesigner.
    * Allows to use manips to configure SpotLights, PointLights and 
      Transform nodes. Just use the right mouse button to interchange 
      between those nodes and his manip!
    * GUI has been translated to English and Spanish, if you want to help
      us to translate to your languaje write the author. A translation work
      takes just about 90 minutes using Qt-Linguist.
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

Default languaje is english. If you want to see coindesigner in your own
languaje then you have to define the LANG enviroment variable to be one of
EN (for English), ES (for Spanish) or PT (for Portuguese) and then run 
coindesigner. Please, ensure that the translation files (*.qs) are found 
in the current directory.

BUILDING FROM THE SOURCES (LINUX)

You will need this libraries installed and working
 * CMake as build tool
   $apt-get install cmake
 
 * Qt version 4.3.2 or newer. Usually shipped with most Linux distributions
   today. For instance in Debian:
   $apt-get install libqt4-dev

 * Simage, Coin3D and SoQt. 
   $apt-get install libsimage-dev libsoqt4-dev

 * Optionally:
   - you can use SIM Voleon to visualice volumetric data. Visit 
     http://coin3d.org to read how to download and install these libraries.

   - you can use libassimp to import more types of files
     $apt-get install libassimp-dev

To build coindesigner and cdsview 

1) Uncompress the coindesigner-x.x.tgz package

   tar xvfz coindesigner-x.x.tgz

2) Go to the new directory
  
   cd coindesigner-x.x

3) Run the make comand
   $mkdir build
   $cd build
   $cmake ..
   $make


This will create two executable files:

* Coindesigner: The RAD system

* cdsview: The standalone multiplatform viewer


To use the translation files, you must run coindesigner in the 
same directory that the coindesigner_your_language.qm file. Please
ensure that the LANG enviroment variable is adecuate to your language.


ACKNOWLEDGES

"If I have seen further, it is by standing upon the shoulders of giants"
(Isaac Newton, but I found it in Oscar Robles PhD)
Well, most of my giants are working on the Modelling and Virtual Reality
Group of the Rey Juan Carlos university. Their names are Marcos, Sofía, Oscar,
Susana, Pablo, Luis, Angel, Luismi, Cesar, Carolina, and many more people that
work in the group.

The 3ds importer is part of coin3d package and was developed by PC John.
This piece of code is usually available when you compile coin3D with
the "--enable-3ds" flags, which is not the default. To avoid everybody 
to have to recompile coin3d, we decided to incorporate those file to
our project

Most icons used in coindesigner UI are part of Crystal Icons. This icon 
theme was created by Everaldo and is now developed by him in association
with the Crystal Artists Staff. See http://linuxcult.com/node/10
Svg icons are from the tango project

Icons for sceneGraph nodes are based on "Dia Shapes for Open Inventor Scenegraphs",
package, compiled by Alejandro Sierra.
 
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

Last but not least, many fragment of coindesigner are based on examples found in
the Coin3D and Qt documentation. We would like to acknowledge that work and 
the help received from Lars J. Aas, Morten Eriksen and Peder Blekken from 
Kongsberg SIM (aka System in Motion) through the coin-discuss mail list.
They are the Coin on Coindesigner.


