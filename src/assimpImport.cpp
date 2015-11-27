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


#include "assimpImport.h"

#ifdef USE_ASSIMP
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/importerdesc.h>
#include <assimp/postprocess.h>

#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoIndexedPointSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoIndexedTriangleStripSet.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoTexture2.h>

#include <iostream>


SoTransform *getTransform(const aiMatrix4x4 &matrix) {
    aiVector3D scaling;
    aiQuaternion rotation;
    aiVector3D position;
    matrix.Decompose(scaling,rotation,position);

    SoTransform *transform(new SoTransform);
    transform->translation.setValue(position[0],
                                    position[1],
                                    position[2]);
    transform->rotation.setValue(rotation.x,
                                 rotation.y,
                                 rotation.z,
                                 rotation.w);
    transform->scaleFactor.setValue(scaling[0],
                                    scaling[1],
                                    scaling[2]);

    return transform;
}


SoNode *getTexture(const aiTexture *const texture) {
    if (texture->mHeight == 0) {//Compressed texture
        std::cout << "Found compressed texture" << std::endl;
        //texture->pcData is a pointer to a memory buffer of size mWidth containing the compressed texture data
        return NULL;
    } else {//Uncompressed texture
        unsigned char pixels[texture->mWidth*texture->mHeight*4];
        for (std::size_t i(0); i < texture->mWidth; ++i) {
            for (std::size_t j(0); j < texture->mHeight; ++j) {
                pixels[4*(texture->mHeight*i+j)+0] = texture->pcData[texture->mHeight*i+j].r;
                pixels[4*(texture->mHeight*i+j)+1] = texture->pcData[texture->mHeight*i+j].g;
                pixels[4*(texture->mHeight*i+j)+2] = texture->pcData[texture->mHeight*i+j].b;
                pixels[4*(texture->mHeight*i+j)+3] = texture->pcData[texture->mHeight*i+j].a;
            }
        }
        SoTexture2 *soTexture(new SoTexture2);
        soTexture->image.setValue(SbVec2s(texture->mWidth,texture->mHeight),4,pixels);

        return soTexture;
    }
}


SoMaterial *getMaterial(const aiMaterial *const material) {
    SoMaterial *soMat(new SoMaterial);

    aiString name;
    aiColor3D color;
    float value;

    //Add name
    if (AI_SUCCESS == material->Get(AI_MATKEY_NAME,name)) {
        soMat->setName(SbName(name.C_Str()));
    }

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


    std::vector<std::pair<aiTextureType,std::string> > textureType;
    textureType.push_back(std::make_pair(aiTextureType_NONE,"none"));
    textureType.push_back(std::make_pair(aiTextureType_DIFFUSE,"diffuse"));
    textureType.push_back(std::make_pair(aiTextureType_SPECULAR,"specular"));
    textureType.push_back(std::make_pair(aiTextureType_AMBIENT,"ambient"));
    textureType.push_back(std::make_pair(aiTextureType_EMISSIVE,"emissive"));
    textureType.push_back(std::make_pair(aiTextureType_HEIGHT,"height"));
    textureType.push_back(std::make_pair(aiTextureType_NORMALS,"normals"));
    textureType.push_back(std::make_pair(aiTextureType_SHININESS,"shininess"));
    textureType.push_back(std::make_pair(aiTextureType_OPACITY,"opacity"));
    textureType.push_back(std::make_pair(aiTextureType_DISPLACEMENT,"displacement"));
    textureType.push_back(std::make_pair(aiTextureType_LIGHTMAP,"lightmap"));
    textureType.push_back(std::make_pair(aiTextureType_REFLECTION,"reflection"));
    textureType.push_back(std::make_pair(aiTextureType_UNKNOWN,"unknwon"));

    aiTextureMapping mappings[] = {aiTextureMapping_UV, //0
                                   aiTextureMapping_SPHERE, //1
                                   aiTextureMapping_CYLINDER, //2
                                   aiTextureMapping_BOX, //3
                                   aiTextureMapping_PLANE, //4
                                   aiTextureMapping_OTHER}; //5

    aiTextureOp operations[] = {aiTextureOp_Multiply, //0
                                aiTextureOp_Add, //1
                                aiTextureOp_Subtract, //2
                                aiTextureOp_Divide, //3
                                aiTextureOp_SmoothAdd, //4
                                aiTextureOp_SignedAdd}; //5

    aiTextureMapMode mapModes[] = {aiTextureMapMode_Wrap, //0
                                   aiTextureMapMode_Clamp, //1
                                   aiTextureMapMode_Mirror, //2
                                   aiTextureMapMode_Decal}; //3


    bool found(false);
    std::vector<SoTexture2*> textures;
    for (std::vector<std::pair<aiTextureType,std::string> >::const_iterator it(textureType.begin());
         it != textureType.end(); ++it) {
        for (std::size_t i(0); i < material->GetTextureCount(it->first); ++i) {
            aiString path;
            aiTextureMapping mapping;
            unsigned int uvIndex;
            float blendFactor;
            aiTextureOp operation;
            aiTextureMapMode mapMode[3];
            if (AI_SUCCESS == material->GetTexture(it->first,i,&path,&mapping,&uvIndex,&blendFactor,&operation,mapMode)) {
                found = true;
                std::cout << it->second << ": " << path.C_Str() << " " << mapping << " " << uvIndex << " "
                          << blendFactor << " " << operation << " " << mapMode[0] << " " << mapMode[1] << " " << mapMode[2];
                std::cout << std::endl;
                SoTexture2 *texture(new SoTexture2);
                texture->filename.setValue(path.C_Str());
                if (mapping == aiTextureMapping_UV) {
                    switch (mappingMode[0]) {
                        case aiTextureMapMode_Wrap:
                            texture->wrapS.setValue(SoTexture2::REPEAT);
                        break;
                        case aiTextureMapMode_Clamp:
                            texture->wrapS.setValue(SoTexture2::CLAMP);
                        break;
                    }
                    switch (mappingMode[1]) {
                        case aiTextureMapMode_Wrap:
                            texture->wrapT.setValue(SoTexture2::REPEAT);
                        break;
                        case aiTextureMapMode_Clamp:
                            texture->wrapT.setValue(SoTexture2::CLAMP);
                        break;
                    }
                }
            }
        }
    }
    if (found) std::cout << std::endl;

    return soMat;
}


SoIndexedShape *getShape(const aiMesh *const mesh) {
    if (!mesh->HasPositions() ||
            !mesh->HasFaces()) return NULL;

    if (mesh->GetNumUVChannels() > 0) {
        std::cout << "Mesh has " << mesh->GetNumUVChannels()
                  << " uv channels with [" << mesh->mNumUVComponents[0];
        for (unsigned int i(1); i < mesh->GetNumUVChannels(); ++i ) {
            std::cout << mesh->mNumUVComponents[i] << " ";
        }
        std::cout << "] components" << std::endl;
    }

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


SoSeparator *getMesh(const aiMesh *const mesh, const aiMaterial *const material,
                     SoSeparator *meshSep = NULL) {
    if (!meshSep) {
        meshSep = new SoSeparator;
        meshSep->setName(SbName(mesh->mName.C_Str()));
    }

    //Add material
    meshSep->addChild(getMaterial(material));

    //Add shape
    SoIndexedShape *shape(getShape(mesh));
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


void addNode(SoSeparator *const parent, const aiNode *const node,
             const aiMaterial *const *const materials, const aiMesh *const *const meshes,
             const aiTexture *const *const textures) {
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

            //Add transform
            if (node->mParent && !node->mTransformation.IsIdentity())
                nodeSep->addChild(getTransform(node->mTransformation));

            //Add meshes
            if (node->mNumMeshes == 1 && node->mNumChildren == 0) {
                getMesh(meshes[node->mMeshes[0]],
                        materials[meshes[node->mMeshes[0]]->mMaterialIndex],
                        nodeSep);
            } else {
                for (std::size_t i(0); i < node->mNumMeshes; ++i) {
                    nodeSep->addChild(getMesh(meshes[node->mMeshes[i]],
                                              materials[meshes[node->mMeshes[i]]->mMaterialIndex]));
                }
            }
        }
        //Add children nodes
        for (std::size_t i(0); i < node->mNumChildren; ++i) {
            addNode(nodeSep,node->mChildren[i],materials,meshes,textures);
        }
    }
}


SoSeparator *Assimp2Inventor(const aiScene *const scene) {
    SoSeparator *root(new SoSeparator);
    std::cout << "I have " << scene->mNumTextures << " textures embedded" << std::endl;
    addNode(root,scene->mRootNode,scene->mMaterials,scene->mMeshes,scene->mTextures);
    return root;
}


SoSeparator *importScene(const std::string &filename, std::string *const error) {
    try {
        Assimp::Importer importer;

        //Set the parts of the data structure to be removed
        /*importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                                    aiComponent_TANGENTS_AND_BITANGENTS |
                                    aiComponent_COLORS |
                                    aiComponent_TEXCOORDS |
                                    aiComponent_BONEWEIGHTS |
                                    aiComponent_ANIMATIONS |
                                    aiComponent_TEXTURES |
                                    aiComponent_LIGHTS |
                                    aiComponent_CAMERAS);*/

        //Set the post processing step to be applied
        unsigned int postProcessSteps
                (//aiProcess_JoinIdenticalVertices | //Remove repeated vertices
                 aiProcess_Triangulate | //Convert polygons to triangles
                 //aiProcess_RemoveComponent | //Remove the previously specified components
                 aiProcess_GenNormals | //Generate missing normals
                 aiProcess_ValidateDataStructure | //Make sure everything is OK
                 //aiProcess_RemoveRedundantMaterials | //Check for redundant materials
                 //aiProcess_FixInfacingNormals  | //Invert normals facing inwards
                 aiProcess_SortByPType | //Split meshes with more than one primitive (i.e. points, lines, trinagles, polygons)
                 aiProcess_FindDegenerates | //Convert degenerated primitives to proper ones
                 aiProcess_FindInvalidData //| //Remove/Fix invalid data
                 //aiProcess_OptimizeMeshes | //Reduce the number of meshes
                 //aiProcess_OptimizeGraph | //Optimize the scene hierarchy
                 //aiProcess_Debone //Remove bones losslessly
                 );

        //Import scene
        const aiScene *const scene(importer.ReadFile(filename,postProcessSteps));

        if (scene) {
            //Convert from Assimp to Inventor
            return Assimp2Inventor(scene);
        } else {
            if (error) *error = importer.GetErrorString();
            return NULL;
        }
    } catch (...) {
        if (error) *error = "File could not be imported.";
        return NULL;
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


#else // USE_ASSIMP
SoSeparator *importScene(std::string filename, std::string *error) {
    if (error) *error = "Assimp not available";
    return NULL;
}


std::vector<std::string> assimpImportedExtensions() {
    std::vector<std::string> importedExtensions;
    return importedExtensions;
}


std::vector<std::pair<std::string,std::vector<std::string> > > assimpImportedFormats() {
    return std::vector<std::pair<std::string,std::vector<std::string> > >();
}
#endif // USE_ASSIMP
