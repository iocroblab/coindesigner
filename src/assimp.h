/*************************************************************************\
  Copyright 2014 Institute of Industrial and Control Engineering (IOC)
                Universitat Politecnica de Catalunya
                BarcelonaTech
   All Rights Reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
\*************************************************************************/

/* Author: Nestor Garcia Hidalgo */

#ifndef ASSIMP_H
#define ASSIMP_H

#include <Inventor/nodes/SoSeparator.h>
#include <string>
#include <vector>

SoSeparator* importScene(const std::string &filename, std::string *const error = NULL);

bool exportScene(const std::string &filename, SoSeparator *root, std::string *const error = NULL);

std::vector<std::string> assimpImportedExtensions();

std::vector< std::pair< std::string,std::vector<std::string> > > assimpImportedFormats();

std::vector< std::pair< std::string,std::vector<std::string> > > assimpExportedFormats();

#endif // ASSIMP_H
