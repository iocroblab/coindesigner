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

#ifdef USE_ASSIMP
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>

#include <Inventor/nodes/SoShape.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/SbMatrix.h>

#include <iostream>
#include <map>


struct Vertex {
    aiVector3D position;

    aiVector3D normal;
};


struct Mesh {
    std::size_t addVertex(const Vertex &vertex) {
        std::map<Vertex,std::size_t>::const_iterator it(verticesMap.find(vertex));
        if (it != verticesMap.end()) {
            return it->second;
        } else {
            std::size_t index(vertices.size());
            vertices.push_back(vertex);
            verticesMap.insert(std::make_pair(vertex,index));
            return index;
        }
    }


    std::vector<Vertex> vertices;

    std::vector<aiFace> faces;

private:
    struct cmpVertex {
        bool operator()(const aiVector3D &a, const aiVector3D &b) const {
            return a[0] < b[0] ||
                    (a[0] == b[0] &&
                    (a[1] < b[1] ||
                    (a[1] == b[1] &&
                    a[2] < b[2])));
        }

        bool operator()(const Vertex &a, const Vertex &b) const {
            return cmpVertex::operator ()(a.position,b.position) ||
                    (!cmpVertex::operator ()(b.position,a.position) &&
                     cmpVertex::operator ()(a.normal,b.normal));
        }
    };


    std::map<Vertex,std::size_t,cmpVertex> verticesMap;
};


struct Material {
    SbColor ambient;

    SbColor diffuse;

    SbColor specular;

    SbColor emissive;

    float shininess;

    float transparency;
};


aiMaterial *getMaterial(const Material &material) {
    aiMaterial *aiMat(new aiMaterial);

    aiColor3D color;
    float value;

    //Add diffuse color
    color.r = material.diffuse[0];
    color.g = material.diffuse[1];
    color.b = material.diffuse[2];
    if (aiReturn_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_DIFFUSE)) {
        std::cout << "Error at setting diffuse color" << std::endl;
    }

    //Add specular color
    color.r = material.specular[0];
    color.g = material.specular[1];
    color.b = material.specular[2];
    if (aiReturn_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_SPECULAR)) {
        std::cout << "Error at setting specular color" << std::endl;
    }

    //Add ambient color
    color.r = material.ambient[0];
    color.g = material.ambient[1];
    color.b = material.ambient[2];
    if (aiReturn_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_AMBIENT)) {
        std::cout << "Error at setting ambient color" << std::endl;
    }

    //Add emissive color
    color.r = material.emissive[0];
    color.g = material.emissive[1];
    color.b = material.emissive[2];
    if (aiReturn_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_EMISSIVE)) {
        std::cout << "Error at setting emissive color" << std::endl;
    }

    //Add transparency
//    value = 1.0 - material.transparency;
//    if (aiReturn_SUCCESS != aiMat->AddProperty(&value,1,AI_MATKEY_OPACITY)) {
//        std::cout << "Error at setting transparency" << std::endl;
//    }

    //Add shininess
    value = material.shininess;
    if (aiReturn_SUCCESS != aiMat->AddProperty(&value,1,AI_MATKEY_SHININESS_STRENGTH)) {
        std::cout << "Error at setting shininess" << std::endl;
    }

    return aiMat;
}


struct Geometry {
    void addFace(const SoCallbackAction *const action, const SoPrimitiveVertex *const *const vertices,
                 unsigned int numIndices) {
        //Add material
        Material material;
        action->getMaterial(material.ambient,material.diffuse,material.specular,
                            material.emissive,material.shininess,material.transparency,
                            vertices[0]->getMaterialIndex());//All vertices are supposed to have the same material index
        Mesh &mesh(addMaterial(material));

        //Add position and normal of the vertices
        std::size_t indices[numIndices];
        const SbMatrix &positionMatrix(action->getModelMatrix());
        const SbMatrix normalMatrix(positionMatrix.inverse().transpose());
        //Normals need to be transformed using the transpose of the inverse model matrix
        for (unsigned int i(0); i < numIndices; ++i) {
            SbVec3f position, normal;
            positionMatrix.multVecMatrix(vertices[i]->getPoint(),position);
            normalMatrix.multDirMatrix(vertices[i]->getNormal(),normal);

            Vertex vertex;
            for (unsigned int j(0); j < 3; ++j) {
                vertex.position[j] = position[j];
                vertex.normal[j] = normal[j];
            }

            indices[i] = mesh.addVertex(vertex);
        }

        //Add face
        SoShapeHints::VertexOrdering vertexOrdering(action->getVertexOrdering());
        if (vertexOrdering == SoShapeHints::UNKNOWN_ORDERING) {
            if (numIndices == 3) {
                SbVec3f n = SbVec3f(vertices[1]->getPoint()-vertices[0]->getPoint()).
                        cross(vertices[2]->getPoint()-vertices[0]->getPoint());
                if (vertices[0]->getNormal().dot(n) > 0) {//All vertices are supposed to have the same normal
                    vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
                } else {
                    vertexOrdering = SoShapeHints::CLOCKWISE;
                }
            } else {
                vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
            }
        }
        //In Assimp face vertices are specified in a counter-clockwise order.
        if (vertexOrdering == SoShapeHints::COUNTERCLOCKWISE) {
            aiFace face;
            face.mNumIndices = numIndices;
            face.mIndices = new unsigned int[numIndices];
            for (unsigned int i = 0; i < numIndices; ++i) {
                face.mIndices[i] = indices[i];
            }
            mesh.faces.push_back(face);
        } else {
            aiFace face;
            face.mNumIndices = numIndices;
            face.mIndices = new unsigned int[numIndices];
            for (unsigned int i = 0; i < numIndices; ++i) {
                face.mIndices[numIndices-1-i] = indices[i];
            }
            mesh.faces.push_back(face);
        }
    }

    std::vector<Mesh> meshes;

    std::vector<aiMaterial*> materials;

private:
    Mesh &addMaterial(const Material &material) {
        std::map<Material,std::size_t>::const_iterator it(materialsMap.find(material));
        if (it != materialsMap.end()) {
            return meshes.at(it->second);
        } else {
            std::size_t index(materials.size());
            materials.push_back(getMaterial(material));
            materialsMap.insert(std::make_pair(material,index));
            meshes.push_back(Mesh());
            return meshes.at(index);
        }
    }


    struct cmpMaterial {
        bool operator()(const SbColor &a, const SbColor &b) const {
            return a[0] < b[0] ||
                    (a[0] == b[0] &&
                    (a[1] < b[1] ||
                    (a[1] == b[1] &&
                    a[2] < b[2])));
        }

        bool operator()(const Material &a, const Material &b) const {
            return cmpMaterial::operator ()(a.ambient,b.ambient) ||
                    (!cmpMaterial::operator ()(b.ambient,a.ambient) &&
                     (cmpMaterial::operator ()(a.diffuse,b.diffuse) ||
                      (!cmpMaterial::operator ()(b.diffuse,a.diffuse) &&
                       (cmpMaterial::operator ()(a.emissive,b.emissive) ||
                        (!cmpMaterial::operator ()(b.emissive,a.emissive) &&
                         (cmpMaterial::operator ()(a.specular,b.specular) ||
                          (!cmpMaterial::operator ()(b.specular,a.specular) &&
                           (a.shininess < b.shininess ||
                            (a.shininess == b.shininess &&
                             a.transparency < b.transparency)))))))));
        }
    };


    std::map<Material,std::size_t,cmpMaterial> materialsMap;
};


aiTexture *getTexture(const unsigned char *pixels, const SbVec2s &size, unsigned int numComp) {
    aiTexture *texture(new aiTexture);
    texture->mWidth = size[0];
    texture->mHeight = size[1];
    texture->pcData = new aiTexel[texture->mWidth*texture->mHeight];
    for (std::size_t i(0); i < texture->mWidth; ++i) {
        for (std::size_t j(0); j < texture->mHeight; ++j) {
            switch (numComp) {
                case 1://Grayscale
                    texture->pcData[texture->mHeight*i+j].r = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].g = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].b = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].a = 255;
                    break;
                case 2://Grayscale+Alpha
                    texture->pcData[texture->mHeight*i+j].r = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].g = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].b = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].a = pixels[numComp*(texture->mHeight*i+j)+1];
                    break;
                case 3://RGB
                    texture->pcData[texture->mHeight*i+j].r = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].g = pixels[numComp*(texture->mHeight*i+j)+1];
                    texture->pcData[texture->mHeight*i+j].b = pixels[numComp*(texture->mHeight*i+j)+2];
                    texture->pcData[texture->mHeight*i+j].a = 255;
                    break;
                case 4://RGBA
                    texture->pcData[texture->mHeight*i+j].r = pixels[numComp*(texture->mHeight*i+j)+0];
                    texture->pcData[texture->mHeight*i+j].g = pixels[numComp*(texture->mHeight*i+j)+1];
                    texture->pcData[texture->mHeight*i+j].b = pixels[numComp*(texture->mHeight*i+j)+2];
                    texture->pcData[texture->mHeight*i+j].a = pixels[numComp*(texture->mHeight*i+j)+3];
                    break;
            }
        }
    }

    return texture;
}


void getTriangle(void *data, SoCallbackAction *action,
                 const SoPrimitiveVertex *v1,
                 const SoPrimitiveVertex *v2,
                 const SoPrimitiveVertex *v3) {
    const SoPrimitiveVertex *vertices[3] = {v1,v2,v3};
    ((Geometry*)data)->addFace(action,vertices,3);
}


void getLine(void *data, SoCallbackAction *action,
             const SoPrimitiveVertex *v1,
             const SoPrimitiveVertex *v2) {
    const SoPrimitiveVertex *vertices[2] = {v1,v2};
    ((Geometry*)data)->addFace(action,vertices,2);
}


void getPoint(void *data, SoCallbackAction *action,
              const SoPrimitiveVertex *v) {
    ((Geometry*)data)->addFace(action,&v,1);
}


aiScene *Inventor2Assimp(SoSeparator *const root) {
    //Get geometry
    Geometry geometry;
    SoCallbackAction callbackAction;
    callbackAction.addTriangleCallback(SoShape::getClassTypeId(),getTriangle,&geometry);
    callbackAction.addLineSegmentCallback(SoShape::getClassTypeId(),getLine,&geometry);
    callbackAction.addPointCallback(SoShape::getClassTypeId(),getPoint,&geometry);
    callbackAction.apply(root);

    //Create scene
    aiScene *scene(new aiScene);

    //Set materials
    scene->mNumMaterials = geometry.materials.size();
    scene->mMaterials = new aiMaterial*[scene->mNumMaterials];
    std::copy(geometry.materials.begin(),geometry.materials.end(),scene->mMaterials);

    //Set meshes
    scene->mNumMeshes = geometry.meshes.size();
    scene->mMeshes = new aiMesh*[scene->mNumMeshes];
    for (std::size_t i(0); i < scene->mNumMeshes; ++i) {
        //Set mesh
        scene->mMeshes[i] = new aiMesh;

        //Set material
        scene->mMeshes[i]->mMaterialIndex = i;

        //Set vertex coordinates and normals
        scene->mMeshes[i]->mNumVertices = geometry.meshes.at(i).vertices.size();
        scene->mMeshes[i]->mVertices =  new aiVector3D[scene->mMeshes[i]->mNumVertices];
        scene->mMeshes[i]->mNormals = new aiVector3D[scene->mMeshes[i]->mNumVertices];
        for (std::size_t j(0); j < scene->mMeshes[i]->mNumVertices; ++j) {
            scene->mMeshes[i]->mVertices[j] =  geometry.meshes.at(i).vertices.at(j).position;
            scene->mMeshes[i]->mNormals[j] = geometry.meshes.at(i).vertices.at(j).normal;
        }

        //Set faces (i.e. points, lines or triangles)
        scene->mMeshes[i]->mNumFaces = geometry.meshes.at(i).faces.size();
        scene->mMeshes[i]->mFaces = new aiFace[scene->mMeshes[i]->mNumFaces];
        for (std::size_t j(0); j < scene->mMeshes[i]->mNumFaces; ++j) {
            scene->mMeshes[i]->mFaces[j] = geometry.meshes.at(i).faces.at(j);
            switch (scene->mMeshes[i]->mFaces[j].mNumIndices) {
                case 1: //Point
                    scene->mMeshes[i]->mPrimitiveTypes = scene->mMeshes[i]->mPrimitiveTypes |
                            aiPrimitiveType_POINT;
                break;
                case 2: //Line
                    scene->mMeshes[i]->mPrimitiveTypes = scene->mMeshes[i]->mPrimitiveTypes |
                            aiPrimitiveType_LINE;
                break;
                case 3: //Triangle
                    scene->mMeshes[i]->mPrimitiveTypes = scene->mMeshes[i]->mPrimitiveTypes |
                            aiPrimitiveType_TRIANGLE;
                break;
            }
        }
    }

    //Set root node (the one and only scene node)
    scene->mRootNode = new aiNode;
    scene->mRootNode->mNumMeshes = scene->mNumMeshes;
    scene->mRootNode->mMeshes = new unsigned int[scene->mNumMeshes];
    for (std::size_t i(0); i < scene->mNumMeshes; ++i) {
        scene->mRootNode->mMeshes[i] = i;
    }

    return scene;
}


bool exportScene(const std::string &filename, SoSeparator *root, std::string *const error) {
    try {
        //Check output format
        Assimp::Exporter exporter;
        std::string extension(filename.substr(filename.find_last_of(".")+1));
        std::string formatID;
        for (std::size_t i(0); i < exporter.GetExportFormatCount() && formatID.empty(); ++i) {
            if (exporter.GetExportFormatDescription(i)->fileExtension == extension) {
                formatID = exporter.GetExportFormatDescription(i)->id;
            }
        }
        if (formatID.empty()) return false;

        //Convert from Inventor to Assimp
        const aiScene *const scene(Inventor2Assimp(root));

        if (aiReturn_SUCCESS == exporter.Export(scene,formatID,filename/*,
                                          aiProcess_JoinIdenticalVertices | //No repeated vertices
                                          aiProcess_SortByPType | //Split meshes with more than one primitive
                                          aiProcess_RemoveRedundantMaterials | //Check for redundant materials
                                          aiProcess_OptimizeMeshes | //Reduce the number of meshes
                                          aiProcess_OptimizeGraph | //Optimize the scene hierarchy
                                          aiProcess_GenNormals | //Generate missing normals
                                          aiProcess_FindInvalidData*/)) { //Remove/Fix invalid data
            delete scene;
            return true;
        } else {
            if (error) *error = exporter.GetErrorString();
            delete scene;
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
bool exportScene(std::string filename, SoSeparator *root, std::string *error) {
    if (error) *error = "Assimp not available";
    return false;
}


std::vector<std::pair<std::string,std::vector<std::string> > > assimpExportedFormats() {
    return std::vector<std::pair<std::string,std::vector<std::string> > >();
}
#endif // USE_ASSIMP
