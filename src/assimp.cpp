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


#include "assimp.h"

#ifdef USE_ASSIMP
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>

#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoIndexedPointSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoIndexedTriangleStripSet.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/SoPrimitiveVertex.h>

#include <iostream>
#include <algorithm>
#include <map>


SoMaterial *getMaterial(const aiMaterial *material) {
    SoMaterial *soMat(new SoMaterial);

    aiColor3D color;
    float value;

    //Add diffuse color
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE,color)) {
        soMat->diffuseColor.setValue(color.r,
                                     color.g,
                                     color.b);
    }

    //Add specular color
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR,color)) {
        soMat->specularColor.setValue(color.r,
                                      color.g,
                                      color.b);
    }

    //Add ambient color
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT,color)) {
        soMat->ambientColor.setValue(color.r,
                                     color.g,
                                     color.b);
    }

    //Add emissive color
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE,color)) {
        soMat->emissiveColor.setValue(color.r,
                                      color.g,
                                      color.b);
    }

    //Add transparency
    if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY,value)) {
        soMat->transparency.setValue(1.0-value);
    }

    //Add shininess
    if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS_STRENGTH,value)) {
        soMat->shininess.setValue(value);
    }

    return soMat;
}


SoIndexedShape *getShape(const aiMesh *mesh) {
    if (!mesh->HasPositions() ||
            !mesh->HasFaces()) return NULL;

    SoIndexedShape *shape;
    std::size_t numIndices;
    switch (mesh->mPrimitiveTypes) {
        case aiPrimitiveType_POINT:
            shape = new SoIndexedPointSet;
            numIndices = 1;
        break;
        case aiPrimitiveType_LINE:
            shape = new SoIndexedLineSet;
            numIndices = 2;
        break;
        case aiPrimitiveType_TRIANGLE:
            shape = new SoIndexedTriangleStripSet;
            numIndices = 3;
        break;
        default:
        return NULL;
        break;
    }

    SoVertexProperty *vertexProperty(new SoVertexProperty);
    shape->vertexProperty.setValue(vertexProperty);

    float vertices[mesh->mNumVertices][3];
    for (std::size_t i(0); i < mesh->mNumVertices; ++i) {
        vertices[i][0] = mesh->mVertices[i][0];
        vertices[i][1] = mesh->mVertices[i][1];
        vertices[i][2] = mesh->mVertices[i][2];
    }
    vertexProperty->vertex.setValues(0,mesh->mNumVertices,vertices);

    if (mesh->HasNormals()) {
        float normals[mesh->mNumVertices][3];
        for (std::size_t i(0); i < mesh->mNumVertices; ++i) {
            normals[i][0] = mesh->mNormals[i][0];
            normals[i][1] = mesh->mNormals[i][1];
            normals[i][2] = mesh->mNormals[i][2];
        }
        vertexProperty->normal.setValues(0,mesh->mNumVertices,normals);
    }

    int indices[mesh->mNumFaces*(numIndices+1)];
    for (std::size_t i(0); i < mesh->mNumFaces; ++i) {
        for (std::size_t j(0); j < numIndices; ++j) {
            indices[i*(numIndices+1)+j] = mesh->mFaces[i].mIndices[j];
        }
        indices[i*(numIndices+1)+numIndices] = -1;
    }
    shape->coordIndex.setValues(0,mesh->mNumFaces*(numIndices+1),indices);

    return shape;
}


SoSeparator *getMesh(const aiMesh *mesh, const aiMaterial *material,
                     SoSeparator *meshSep = NULL) {
    if (!meshSep) {
        meshSep = new SoSeparator;
        meshSep->setName(SbName(mesh->mName.C_Str()));
    }

    //Add material
    meshSep->addChild(getMaterial(material));

    //Add shape
    SoIndexedShape *shape = getShape(mesh);
    if (shape) meshSep->addChild(shape);

    return meshSep;
}


bool hasMesh(const aiNode *node) {
    if (node->mNumMeshes > 0) return true;
    for (std::size_t i(0); i < node->mNumChildren; ++i) {
        if (hasMesh(node->mChildren[i])) return true;
    }
    return false;
}


void addNode(SoSeparator *parent, const aiNode *node,
             aiMaterial **materials, aiMesh **meshes) {
    if (hasMesh(node)) {
        SoSeparator *nodeSep;
        if ((!node->mParent || node->mTransformation.IsIdentity()) &&
                node->mNumMeshes == 0) {
            nodeSep = parent;
        } else {
            //Create separator
            nodeSep = new SoSeparator;
            nodeSep->setName(SbName(node->mName.C_Str()));
            parent->addChild(nodeSep);

            //Add meshes
            aiMesh *mesh;
            if (node->mNumMeshes == 1 && node->mNumChildren == 0) {
                getMesh(meshes[node->mMeshes[0]],
                        materials[meshes[node->mMeshes[0]]->mMaterialIndex],
                        nodeSep);
            } else {
                for (std::size_t i(0); i < node->mNumMeshes; ++i) {
                    mesh = meshes[node->mMeshes[i]];
                    nodeSep->addChild(getMesh(mesh,
                                              materials[mesh->mMaterialIndex]));
                }
            }
        }
        //Add children nodes
        for (std::size_t i(0); i < node->mNumChildren; ++i) {
            addNode(nodeSep,node->mChildren[i],materials,meshes);
        }
    }
}


SoSeparator *Assimp2Inventor(const aiScene *scene) {
    SoSeparator *root(new SoSeparator);
    addNode(root,scene->mRootNode,scene->mMaterials,scene->mMeshes);
    return root;
}


SoSeparator *importScene(const std::string &filename, std::string *const error) {
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                                aiComponent_ANIMATIONS |
                                aiComponent_BONEWEIGHTS |
                                aiComponent_CAMERAS |
                                aiComponent_COLORS |
                                aiComponent_LIGHTS |
                                aiComponent_TANGENTS_AND_BITANGENTS |
                                aiComponent_TEXCOORDS |
                                aiComponent_TEXTURES);
    const aiScene *scene(importer.ReadFile(filename,
                                           aiProcess_Triangulate | //Everything will be triangles, lines or points
                                           aiProcess_JoinIdenticalVertices | //No repeated vertices
                                           aiProcess_SortByPType | //Split meshes with more than one primitive
                                           aiProcess_RemoveRedundantMaterials | //Check for redundant materials
                                           aiProcess_OptimizeMeshes | //Reduce the number of meshes
                                           aiProcess_OptimizeGraph | //Optimize the scene hierarchy
                                           aiProcess_Debone | //Remove bones
                                           aiProcess_RemoveComponent | //Remove the previously specified components
                                           aiProcess_GenNormals | //Generate missing normals
                                           aiProcess_FindInvalidData)); //Remove/Fix invalid data
    if (scene) {
        return Assimp2Inventor(scene);
    } else {
        if (error) *error = importer.GetErrorString();
        return NULL;
    }
}


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
    if (AI_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_DIFFUSE)) {
        std::cout << "Error at setting diffuse color" << std::endl;
    }

    //Add specular color
    color.r = material.specular[0];
    color.g = material.specular[1];
    color.b = material.specular[2];
    if (AI_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_SPECULAR)) {
        std::cout << "Error at setting specular color" << std::endl;
    }

    //Add ambient color
    color.r = material.ambient[0];
    color.g = material.ambient[1];
    color.b = material.ambient[2];
    if (AI_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_AMBIENT)) {
        std::cout << "Error at setting ambient color" << std::endl;
    }

    //Add emissive color
    color.r = material.emissive[0];
    color.g = material.emissive[1];
    color.b = material.emissive[2];
    if (AI_SUCCESS != aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_EMISSIVE)) {
        std::cout << "Error at setting emissive color" << std::endl;
    }

    //Add transparency
    value = 1.0 - material.transparency;
    if (AI_SUCCESS != aiMat->AddProperty(&value,1,AI_MATKEY_OPACITY)) {
        std::cout << "Error at setting transparency" << std::endl;
    }

    //Add shininess
    value = material.shininess;
    if (AI_SUCCESS != aiMat->AddProperty(&value,1,AI_MATKEY_SHININESS_STRENGTH)) {
        std::cout << "Error at setting shininess" << std::endl;
    }

    return aiMat;
}


struct Geometry {
    void addFace(SoCallbackAction *action, const SoPrimitiveVertex **vertices,
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


void getTriangle(void *data, SoCallbackAction *action,
                 const SoPrimitiveVertex *v1,
                 const SoPrimitiveVertex *v2,
                 const SoPrimitiveVertex *v3) {
    Geometry *geometry((Geometry*)data);
    const SoPrimitiveVertex *vertices[3] = {v1,v2,v3};
    geometry->addFace(action,vertices,3);
}


void getLine(void *data, SoCallbackAction *action,
             const SoPrimitiveVertex *v1,
             const SoPrimitiveVertex *v2) {
    Geometry *geometry((Geometry*)data);
    const SoPrimitiveVertex *vertices[2] = {v1,v2};
    geometry->addFace(action,vertices,2);
}


void getPoint(void *data, SoCallbackAction *action,
              const SoPrimitiveVertex *v) {
    Geometry *geometry((Geometry*)data);
    geometry->addFace(action,&v,1);
}


namespace Assimp	{

    class Importer;

    struct ScenePrivateData {

        ScenePrivateData()
            : mOrigImporter()
            , mPPStepsApplied()
            , mIsCopy()
        {}

        // Importer that originally loaded the scene though the C-API
        // If set, this object is owned by this private data instance.
        Assimp::Importer* mOrigImporter;

        // List of postprocessing steps already applied to the scene.
        unsigned int mPPStepsApplied;

        // true if the scene is a copy made with aiCopyScene()
        // or the corresponding C++ API. This means that user code
        // may have made modifications to it, so mPPStepsApplied
        // and mOrigImporter are no longer safe to rely on and only
        // serve informative purposes.
        bool mIsCopy;
    };

    // Access private data stored in the scene
    inline ScenePrivateData* ScenePriv(aiScene* in) {
        return static_cast<ScenePrivateData*>(in->mPrivate);
    }

    inline const ScenePrivateData* ScenePriv(const aiScene* in) {
        return static_cast<const ScenePrivateData*>(in->mPrivate);
    }

}

aiScene::aiScene()
    : mFlags(0)
    , mRootNode(NULL)
    , mNumMeshes(0)
    , mMeshes(NULL)
    , mNumMaterials(0)
    , mMaterials(NULL)
    , mNumAnimations(0)
    , mAnimations(NULL)
    , mNumTextures(0)
    , mTextures(NULL)
    , mNumLights(0)
    , mLights(NULL)
    , mNumCameras(0)
    , mCameras(NULL)
    , mPrivate(new Assimp::ScenePrivateData())
{
}

aiScene::~aiScene()
{
    // delete all sub-objects recursively
    delete mRootNode;

    // To make sure we won't crash if the data is invalid it's
    // much better to check whether both mNumXXX and mXXX are
    // valid instead of relying on just one of them.
    if (mNumMeshes && mMeshes)
        for( unsigned int a = 0; a < mNumMeshes; a++)
            delete mMeshes[a];
    delete [] mMeshes;

    if (mNumMaterials && mMaterials)
        for( unsigned int a = 0; a < mNumMaterials; a++)
            delete mMaterials[a];
    delete [] mMaterials;

    if (mNumAnimations && mAnimations)
        for( unsigned int a = 0; a < mNumAnimations; a++)
            delete mAnimations[a];
    delete [] mAnimations;

    if (mNumTextures && mTextures)
        for( unsigned int a = 0; a < mNumTextures; a++)
            delete mTextures[a];
    delete [] mTextures;

    if (mNumLights && mLights)
        for( unsigned int a = 0; a < mNumLights; a++)
            delete mLights[a];
    delete [] mLights;

    if (mNumCameras && mCameras)
        for( unsigned int a = 0; a < mNumCameras; a++)
            delete mCameras[a];
    delete [] mCameras;

    delete static_cast<Assimp::ScenePrivateData*>( mPrivate );
}


aiScene *Inventor2Assimp(SoSeparator *root) {
    //Get geometry
    Geometry geometry;
    SoCallbackAction callbackAction;
    callbackAction.addTriangleCallback(SoShape::getClassTypeId(),getTriangle,&geometry);
    callbackAction.addLineSegmentCallback(SoShape::getClassTypeId(),getLine,&geometry);
    callbackAction.addPointCallback(SoShape::getClassTypeId(),getPoint,&geometry);
    callbackAction.apply(root);

    aiScene *scene(new aiScene);

    scene->mNumMaterials = geometry.materials.size();
    scene->mMaterials = new aiMaterial*[scene->mNumMaterials];
    std::copy(geometry.materials.begin(),geometry.materials.end(),scene->mMaterials);

    scene->mNumMeshes = geometry.meshes.size();
    scene->mMeshes = new aiMesh*[scene->mNumMeshes];
    for (std::size_t i(0); i < scene->mNumMeshes; ++i) {
        scene->mMeshes[i] = new aiMesh;

        scene->mMeshes[i]->mMaterialIndex = i;

        scene->mMeshes[i]->mNumVertices = geometry.meshes.at(i).vertices.size();
        scene->mMeshes[i]->mVertices =  new aiVector3D[scene->mMeshes[i]->mNumVertices];
        scene->mMeshes[i]->mNormals = new aiVector3D[scene->mMeshes[i]->mNumVertices];
        for (std::size_t j(0); j < scene->mMeshes[i]->mNumVertices; ++j) {
            scene->mMeshes[i]->mVertices[j] =  geometry.meshes.at(i).vertices.at(j).position;
            scene->mMeshes[i]->mNormals[j] = geometry.meshes.at(i).vertices.at(j).normal;
        }

        scene->mMeshes[i]->mNumFaces = geometry.meshes.at(i).faces.size();
        scene->mMeshes[i]->mFaces = new aiFace[scene->mMeshes[i]->mNumFaces];
        for (std::size_t j(0); j < scene->mMeshes[i]->mNumFaces; ++j) {
            scene->mMeshes[i]->mFaces[j] = geometry.meshes.at(i).faces.at(j);
            switch (scene->mMeshes[i]->mFaces[j].mNumIndices) {
                case 1:
                    scene->mMeshes[i]->mPrimitiveTypes = scene->mMeshes[i]->mPrimitiveTypes |
                            aiPrimitiveType_POINT;
                break;
                case 2:
                    scene->mMeshes[i]->mPrimitiveTypes = scene->mMeshes[i]->mPrimitiveTypes |
                            aiPrimitiveType_LINE;
                break;
                case 3:
                    scene->mMeshes[i]->mPrimitiveTypes = scene->mMeshes[i]->mPrimitiveTypes |
                            aiPrimitiveType_TRIANGLE;
                break;
            }
        }
    }

    scene->mRootNode = new aiNode;
    scene->mRootNode->mNumMeshes = scene->mNumMeshes;
    scene->mRootNode->mMeshes = new unsigned int[scene->mNumMeshes];
    for (std::size_t i(0); i < scene->mNumMeshes; ++i) {
        scene->mRootNode->mMeshes[i] = i;
    }

    return scene;
}


bool exportScene(const std::string &filename, SoSeparator *root, std::string *const error) {
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

    //Convert Inventor to Assimp
    const aiScene *scene(Inventor2Assimp(root));

    if (AI_SUCCESS == exporter.Export(scene,formatID,filename,
                                      aiProcess_JoinIdenticalVertices | //No repeated vertices
                                      aiProcess_SortByPType | //Split meshes with more than one primitive
                                      aiProcess_RemoveRedundantMaterials | //Check for redundant materials
                                      aiProcess_OptimizeMeshes | //Reduce the number of meshes
                                      aiProcess_OptimizeGraph | //Optimize the scene hierarchy
                                      aiProcess_GenNormals | //Generate missing normals
                                      aiProcess_FindInvalidData)) { //Remove/Fix invalid data
        delete scene;
        return true;
    } else {
        if (error) *error = exporter.GetErrorString();
        delete scene;
        return false;
    }
}


std::vector<std::string> tokenize(const std::string &str, const std::string &token) {
    std::vector<std::string> tokenized;
    size_t from(0), size(str.size());
    for (size_t to(std::min(str.find(token,from),size));
         from < to; to = std::min(str.find(token,from),size)) {
        tokenized.push_back(str.substr(from,to-from));
        from = to + token.size();
    }
    return tokenized;
}


std::vector<std::string> assimpImportedExtensions() {
    aiString tmp;
    Assimp::Importer importer;
    importer.GetExtensionList(tmp);
    std::string extensions(tmp.C_Str());

    return tokenize(extensions.substr(2,std::string::npos),";*.");
}


std::vector<std::pair<std::string,std::vector<std::string> > > assimpImportedFormats() {
    std::vector<std::pair<std::string,std::vector<std::string> > > importedFormats;
    const aiImporterDesc *importerDesc;
    Assimp::Importer importer;
    std::string name;
    std::vector<std::string> extensions;
    std::size_t k, pos;
    for (std::size_t i(0); i < importer.GetImporterCount(); ++i) {
        importerDesc = importer.GetImporterInfo(i);

        name = importerDesc->mName;
        pos = name.find(" Importer");
        if (pos != std::string::npos) name.erase(pos,9);
        pos = name.find(" Reader");
        if (pos != std::string::npos) name.erase(pos,7);
        pos = name.find("\n");
        if (pos != std::string::npos) name.erase(pos,std::string::npos);
        while (name.substr(name.size()-1) == " ") {
            name.erase(name.size()-1,1);
        }
        extensions = tokenize(importerDesc->mFileExtensions," ");

        k = 0;
        while (k < importedFormats.size() &&
               importedFormats.at(k).first != name) {
            k++;
        }
        if (k < importedFormats.size()) {
            for (std::size_t j(0); j < extensions.size(); ++j) {
                importedFormats.at(k).second.push_back(extensions.at(j));
            }
        } else {
            std::pair< std::string,std::vector<std::string> > format;
            format.first = name;
            format.second = extensions;
            importedFormats.push_back(format);
        }
    }

    return importedFormats;
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
SoSeparator *importScene(std::string filename, std::string *error) {
    if (error) *error = "Assimp not available";
    return NULL;
}


bool exportScene(std::string filename, SoSeparator *root, std::string *error) {
    if (error) *error = "Assimp not available";
    return false;
}


std::vector<std::string> assimpImportedExtensions() {
    std::vector<std::string> importedExtensions;
    return importedExtensions;
}

std::vector<std::pair<std::string,std::vector<std::string> > > assimpImportedFormats() {
    return std::vector<std::pair<std::string,std::vector<std::string> > >();
}


std::vector<std::pair<std::string,std::vector<std::string> > > assimpExportedFormats() {
    return std::vector<std::pair<std::string,std::vector<std::string> > >();
}
#endif // USE_ASSIMP
