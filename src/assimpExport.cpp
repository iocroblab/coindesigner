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


#include "assimpExport.h"
#include "converter.h"

#ifdef USE_ASSIMP
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>


bool exportScene(const std::string &filename, SoSeparator *root, std::string *const error) {
    try {
        //Check output format
        Assimp::Exporter exporter;
        std::string extension(QFileInfo(filename.c_str()).suffix().toStdString());
        std::string formatID;
        for (std::size_t i(0); i < exporter.GetExportFormatCount() && formatID.empty(); ++i) {
            if (exporter.GetExportFormatDescription(i)->fileExtension == extension) {
                formatID = exporter.GetExportFormatDescription(i)->id;
            }
        }
        if (formatID.empty()) return false;

        //Convert from Inventor to Assimp
        Converter converter;
        const aiScene *const scene(converter.convert(root,QFileInfo(filename.c_str()).absoluteDir()));
        if (aiReturn_SUCCESS == exporter.Export(scene,formatID,filename/*,
                                          aiProcess_JoinIdenticalVertices | //No repeated vertices
                                          aiProcess_SortByPType | //Split meshes with more than one primitive
                                          aiProcess_RemoveRedundantMaterials | //Check for redundant materials
                                          aiProcess_OptimizeMeshes | //Reduce the number of meshes
                                          aiProcess_OptimizeGraph | //Optimize the scene hierarchy
                                          aiProcess_GenNormals | //Generate missing normals
                                          aiProcess_FindInvalidData*/)) { //Remove/Fix invalid data
            return true;
        } else {
            if (error) *error = exporter.GetErrorString();
            return false;
        }
    } catch (...) {
        if (error) *error = "File could not be exported.";
        return false;
    }
}


std::vector<std::pair<std::string,std::string> > assimpExportedFormats() {
    std::vector<std::pair<std::string,std::string> > exportedFormats;
    Assimp::Exporter exporter;
    const aiExportFormatDesc *exporterDesc;
    for (std::size_t i(0); i < exporter.GetExportFormatCount(); ++i) {
        exporterDesc = exporter.GetExportFormatDescription(i);

        exportedFormats.push_back(std::make_pair(exporterDesc->id,
                                                 exporterDesc->fileExtension));
    }

    return exportedFormats;
}


#else // USE_ASSIMP
bool exportScene(const std::string filename, SoSeparator *root, std::string *error) {
    if (error) *error = "Assimp not available";
    return false;
}


std::vector<std::pair<std::string,std::vector<std::string> > > assimpExportedFormats() {
    return std::vector<std::pair<std::string,std::vector<std::string> > >();
}
#endif // USE_ASSIMP
