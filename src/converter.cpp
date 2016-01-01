#include "converter.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/misc/SoChildList.h>
#include <Inventor/SoPrimitiveVertex.h>
#ifdef __COIN__
#include <Inventor/VRMLnodes/SoVRMLImageTexture.h>
#endif

#include <iostream>
#include <sstream>


#define SSTR(x) dynamic_cast<std::ostringstream&>((std::ostringstream() << std::dec << x)).str()


SoCallbackAction::Response
preGroupCB(void *data, SoCallbackAction*, const SoNode *node) {
//    std::cout << "preGroup " << node->getTypeId().getName()
//              << " " << node->getName() << std::endl;

    Converter *converter((Converter*)data);

    //Create a new node and add it to the stack
    aiNode *newNode(new aiNode);
    newNode->mParent = converter->nodes_.top();
    aiNode **newChildren(new aiNode*[converter->nodes_.top()->mNumChildren+1]);
    if (converter->nodes_.top()->mNumChildren > 0) {
        std::copy(converter->nodes_.top()->mChildren,
                  converter->nodes_.top()->mChildren+converter->nodes_.top()->mNumChildren,
                  newChildren);
        delete[] converter->nodes_.top()->mChildren;
    }
    newChildren[converter->nodes_.top()->mNumChildren] = newNode;
    converter->nodes_.top()->mChildren = newChildren;
    converter->nodes_.top()->mNumChildren++;
    converter->nodes_.push(newNode);

    if (node->isOfType(SoSeparator::getClassTypeId())) {
        if (converter->textures_.empty()) {
            converter->textures_.push(NULL);
        } else {
            converter->textures_.push(converter->textures_.top());
        }
    }

    return SoCallbackAction::CONTINUE;
}


SoCallbackAction::Response
postGroupCB(void *data, SoCallbackAction *action, const SoNode *node) {
//    std::cout << "postGroup " << node->getTypeId().getName()
//              << " " << node->getName() << std::endl;

    Converter *converter((Converter*)data);

    //Pop the node from the stack
    converter->nodes_.pop();

    //Pop the texture if the group is a SoSeparator
    if (node->isOfType(SoSeparator::getClassTypeId())) converter->textures_.pop();

    return SoCallbackAction::CONTINUE;
}


SoCallbackAction::Response
preTextureCB(void *data, SoCallbackAction *, const SoNode *node) {
//    std::cout << "preTexture " << node->getTypeId().getName()
//              << " " << node->getName() << std::endl;

    Converter *converter((Converter*)data);

    if (!converter->textures_.empty()) converter->textures_.pop();
    converter->textures_.push(node);

    return SoCallbackAction::CONTINUE;
}


SoCallbackAction::Response
preShapeCB(void *data, SoCallbackAction *action, const SoNode *node) {
//    std::cout << "preShape " << node->getTypeId().getName()
//              << " " << node->getName() << std::endl;

    //Clear the data from the previous shape callback
    ((Converter*)data)->shapeMeshes_.clear();

    return SoCallbackAction::CONTINUE;
}


SoCallbackAction::Response
postShapeCB(void *data, SoCallbackAction *action, const SoNode *node) {
//    std::cout << "postShape " << node->getTypeId().getName()
//              << " " << node->getName() << std::endl;

    Converter *converter((Converter*)data);

    if (!converter->shapeMeshes_.empty()) {
        //Add shape meshes
        std::size_t i(0);
        for (std::map<std::size_t,Mesh>::const_iterator it(converter->shapeMeshes_.begin());
             it != converter->shapeMeshes_.end(); ++it) {
            aiMesh *mesh(new aiMesh);

            //Set name
            mesh->mName.Set(node->getName().getString());
            if (converter->shapeMeshes_.size() > 0) {
                mesh->mName.Append(SSTR(i).c_str());
            }
            i++;

            //Set material
            mesh->mMaterialIndex = it->second.materialIndex;

            //Set primitive types
            mesh->mPrimitiveTypes = it->second.getPrimitiveTypes();

            //Set vertex properties
            const std::vector<Vertex> &vertices(it->second.getVertices());
            mesh->mNumVertices = vertices.size();
            mesh->mVertices = new aiVector3D[vertices.size()];
            mesh->mNormals = new aiVector3D[vertices.size()];
            mesh->mNumUVComponents[0] = 2;
            mesh->mTextureCoords[0] = new aiVector3D[vertices.size()];
            for (std::size_t j(0); j < vertices.size(); ++j) {
                mesh->mVertices[j] = vertices.at(j).coord;
                mesh->mNormals[j] = vertices.at(j).normal;
                mesh->mTextureCoords[0][j] = vertices.at(j).texCoord;
            }

            //Set faces
            const std::vector<aiFace> &faces(it->second.getFaces());
            mesh->mNumFaces = faces.size();
            mesh->mFaces = new aiFace[faces.size()];
            std::copy(faces.begin(),faces.end(),mesh->mFaces);

//            std::cout << "Mesh " << i << ": " <<  vertices.size() << " vertices and "
//                      << faces.size() << " faces" << std::endl;
            converter->meshes_.push_back(mesh);
        }

        //Add mesh indices to current node
        aiNode *ainode = converter->nodes_.top();
        unsigned int *indices(new unsigned int[ainode->mNumMeshes+
                converter->shapeMeshes_.size()]);
        if (ainode->mNumMeshes > 0) {
            std::copy(ainode->mMeshes,ainode->mMeshes+ainode->mNumMeshes,indices);
            delete[] ainode->mMeshes;
        }
        for (std::size_t i(0); i < converter->shapeMeshes_.size(); ++i) {
            indices[ainode->mNumMeshes+i] = converter->meshes_.size()-
                    converter->shapeMeshes_.size()+i;
        }
        ainode->mNumMeshes += converter->shapeMeshes_.size();
        ainode->mMeshes = indices;
    }

    return SoCallbackAction::CONTINUE;
}


void triangleCB(void *data, SoCallbackAction *action,
                const SoPrimitiveVertex *v0,
                const SoPrimitiveVertex *v1,
                const SoPrimitiveVertex *v2) {
    const SoPrimitiveVertex *vertices[3] = {v0,v1,v2};
    ((Converter*)data)->addFace(action,vertices,3);
}


void lineCB(void *data, SoCallbackAction *action,
            const SoPrimitiveVertex *v0,
            const SoPrimitiveVertex *v1) {
    const SoPrimitiveVertex *vertices[2] = {v0,v1};
    ((Converter*)data)->addFace(action,vertices,2);
}


void pointCB(void *data, SoCallbackAction *action,
             const SoPrimitiveVertex *v) {
    ((Converter*)data)->addFace(action,&v,1);
}


Converter::Converter() : scene_(NULL) {
}


Converter::~Converter() {
    clear();
}


aiScene *Converter::convert(SoNode *root, QDir const &sceneDir) {
    //Clear memory
    clear();
    scene_ = new aiScene;
    sceneDir_ = sceneDir;

    //Create a root node and push it onto the stack
    scene_->mRootNode = new aiNode("root");
    nodes_.push(scene_->mRootNode);

    //Create callback actions for the inventor nodes
    //These callback functions perform the conversion
    //note: if one class is derived from the other and both callbacks
    //are registered, both functions will be called
    SoCallbackAction action;
    action.addPreCallback(SoGroup::getClassTypeId(),preGroupCB,this);
    action.addPostCallback(SoGroup::getClassTypeId(),postGroupCB,this);
    action.addPreCallback(SoTexture2::getClassTypeId(),preTextureCB,this);
#ifdef __COIN__
    action.addPreCallback(SoVRMLImageTexture::getClassTypeId(),preTextureCB,this);
#endif
    action.addPreCallback(SoShape::getClassTypeId(),preShapeCB,this);
    action.addPostCallback(SoShape::getClassTypeId(),postShapeCB,this);
    action.addTriangleCallback(SoShape::getClassTypeId(),triangleCB,this);
    action.addLineSegmentCallback(SoShape::getClassTypeId(),lineCB,this);
    action.addPointCallback(SoShape::getClassTypeId(),pointCB,this);

    //Traverse the inventor scene graph
    action.apply(root);

    //Copy meshes into scene
    scene_->mNumMeshes = meshes_.size();
    scene_->mMeshes = new aiMesh*[meshes_.size()];
    std::copy(meshes_.begin(),meshes_.end(),scene_->mMeshes);
    meshes_.clear();

    //Copy materials into scene
    scene_->mNumMaterials = materials_.size();
    scene_->mMaterials = new aiMaterial*[materials_.size()];
    std::copy(materials_.begin(),materials_.end(),scene_->mMaterials);
    materials_.clear();

    assert(!nodes_.empty() && "Node stack underflow.");
    assert(textures_.empty() && "Texture stack was left at inconsistent state.");

    return scene_;
}


void Converter::addFace(const SoCallbackAction *const action,
                        const SoPrimitiveVertex *const *const vertices,
                        unsigned int numIndices) {
    //All vertices must have the same material index
    bool sameMaterial(true);
    for (unsigned int i(1); i < numIndices && sameMaterial; ++i) {
        sameMaterial = (vertices[0]->getMaterialIndex() == vertices[i]->getMaterialIndex());
    }
    assert(sameMaterial && "Vertices of the same face do not have the same material index.");

    //Add material
    Material material;
    action->getMaterial(material.ambient,material.diffuse,material.specular,
                        material.emissive,material.shininess,material.transparency,
                        vertices[0]->getMaterialIndex());
    material.texture = getTexture(textures_.top(),action);
    std::size_t materialIndex(addMaterial(material));

    if (materialIndex == 1) {
        const Material &a(material);
        const Material &b(materialsMap_.begin()->first);
        std::cout << cmpMaterial()(a.ambient,b.ambient) << " " << cmpMaterial()(b.ambient,a.ambient) << std::endl;
        std::cout << cmpMaterial()(a.diffuse,b.diffuse) << " " << cmpMaterial()(b.diffuse,a.diffuse) << std::endl;
        std::cout << cmpMaterial()(a.specular,b.specular) << " " << cmpMaterial()(b.specular,a.specular) << std::endl;
        std::cout << cmpMaterial()(a.emissive,b.emissive) << " " << cmpMaterial()(b.emissive,a.emissive) << std::endl;
        std::cout << (a.shininess < b.shininess) << " " << (b.shininess < a.shininess) << std::endl;
        std::cout << (a.transparency < b.transparency) << " " << (b.transparency < a.transparency) << std::endl;
        std::cout << (a.texture.filename < b.texture.filename) << " " << (b.texture.filename < a.texture.filename) << std::endl;
        std::cout << (a.texture.wrapS < b.texture.wrapS) << " " << (b.texture.wrapS < a.texture.wrapS) << std::endl;
        std::cout << (a.texture.wrapT < b.texture.wrapT) << " " << (b.texture.wrapT < a.texture.wrapT) << std::endl;
        std::cout << (a.texture.image < b.texture.image) << " " << (b.texture.image < a.texture.image) << std::endl;
        std::cout << std::endl;
    }

    //Get mesh
    Mesh &mesh(getMesh(materialIndex));

    //Add vertex properties
    std::size_t indices[numIndices];
    const SbMatrix &coordMatrix(action->getModelMatrix());
    const SbMatrix normalMatrix(coordMatrix.inverse().transpose());
    const SbMatrix &texCoordMatrix(action->getTextureMatrix());
    for (unsigned int i(0); i < numIndices; ++i) {
        SbVec3f coord;
        SbVec3f normal;
        SbVec4f texCoord;
        coordMatrix.multVecMatrix(vertices[i]->getPoint(),coord);
        normalMatrix.multDirMatrix(vertices[i]->getNormal(),normal);
        texCoordMatrix.multVecMatrix(vertices[i]->getTextureCoords(),texCoord);

        Vertex vertex;
        for (unsigned int j(0); j < 3; ++j) {
            vertex.coord[j] = coord[j];
            vertex.normal[j] = normal[j];
            vertex.texCoord[j] = texCoord[j];
        }

        indices[i] = mesh.addVertex(vertex);
    }

    //Add face
    aiFace face;
    face.mNumIndices = numIndices;
    face.mIndices = new unsigned int[numIndices];
    switch (action->getVertexOrdering()) {
    case SoShapeHints::UNKNOWN_ORDERING:
    case SoShapeHints::COUNTERCLOCKWISE:
        for (unsigned int i = 0; i < numIndices; ++i) {
            face.mIndices[i] = indices[i];
        }
        break;
    case SoShapeHints::CLOCKWISE:
        for (unsigned int i = 0; i < numIndices; ++i) {
            face.mIndices[numIndices-1-i] = indices[i];
        }
        break;
    }
    mesh.addFace(face);
}


std::size_t Converter::addMaterial(const Material &material) {
    std::map<Material,std::size_t>::const_iterator it(materialsMap_.find(material));
    if (it != materialsMap_.end()) {
        return it->second;
    } else {
        std::size_t index(materials_.size());
        materials_.push_back(getMaterial(material));
        materialsMap_.insert(std::make_pair(material,index));
        return index;
    }
}


aiMaterial *Converter::getMaterial(const Material &material) const {
    aiMaterial *aiMat(new aiMaterial);

    aiColor3D color;
    float value;

    //Add diffuse color
    color.r = material.diffuse[0];
    color.g = material.diffuse[1];
    color.b = material.diffuse[2];
    if (aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_DIFFUSE) != aiReturn_SUCCESS) {
        std::cout << "Error at setting diffuse color" << std::endl;
    }

    //Add specular color
    color.r = material.specular[0];
    color.g = material.specular[1];
    color.b = material.specular[2];
    if (aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_SPECULAR) != aiReturn_SUCCESS) {
        std::cout << "Error at setting specular color" << std::endl;
    }

    //Add ambient color
    color.r = material.ambient[0];
    color.g = material.ambient[1];
    color.b = material.ambient[2];
    if (aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_AMBIENT) != aiReturn_SUCCESS) {
        std::cout << "Error at setting ambient color" << std::endl;
    }

    //Add emissive color
    color.r = material.emissive[0];
    color.g = material.emissive[1];
    color.b = material.emissive[2];
    if (aiMat->AddProperty(&color,1,AI_MATKEY_COLOR_EMISSIVE) != aiReturn_SUCCESS) {
        std::cout << "Error at setting emissive color" << std::endl;
    }

    //Add transparency
    //    value = 1.0 - material.transparency;
    //    if (aiMat->AddProperty(&value,1,AI_MATKEY_OPACITY) != aiReturn_SUCCESS) {
    //        std::cout << "Error at setting transparency" << std::endl;
    //    }

    //Add shininess
    value = material.shininess;
    if (aiMat->AddProperty(&value,1,AI_MATKEY_SHININESS_STRENGTH) != aiReturn_SUCCESS) {
        std::cout << "Error at setting shininess" << std::endl;
    }

    //Add texture
    addTexture(aiMat,material.texture);

    return aiMat;
}


void Converter::addTexture(aiMaterial *material, const Texture &texture) const {
    if (!texture.filename.empty()) {
        //Add texture path
        aiString path(texture.filename);
        if (material->AddProperty(&path,AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE,0)) !=
                aiReturn_SUCCESS) {
            std::cout << "Error at setting texture path." << std::endl;
        }

        //Set texture mapping
        int value(aiTextureMapping_UV);
        if (material->AddProperty(&value,1,
                                  AI_MATKEY_MAPPING(aiTextureType_DIFFUSE,0)) !=
                aiReturn_SUCCESS) {
            std::cout << "Error at setting texture mapping." << std::endl;
        }
        value = (texture.wrapS == SoTexture2::CLAMP)? aiTextureMapMode_Clamp :
                                                      aiTextureMapMode_Wrap;
        if (material->AddProperty(&value,1,
                                  AI_MATKEY_MAPPINGMODE_U(aiTextureType_DIFFUSE,0)) !=
                aiReturn_SUCCESS) {
            std::cout << "Error at setting texture U mapping mode." << std::endl;
        }
        value = (texture.wrapT == SoTexture2::CLAMP)? aiTextureMapMode_Clamp :
                                                        aiTextureMapMode_Wrap;
        if (material->AddProperty(&value,1,
                                  AI_MATKEY_MAPPINGMODE_V(aiTextureType_DIFFUSE,0)) !=
                aiReturn_SUCCESS) {
            std::cout << "Error at setting texture V mapping mode." << std::endl;
        }
    }
}


Texture Converter::getTexture(const SoNode* node, const SoCallbackAction* action) const {
    Texture texture;

    if (!node) return texture;

    //Get filename
    if (node->isOfType(SoTexture2::getClassTypeId())) {
        texture.filename = ((SoTexture2*)node)->filename.getValue().getString();
#ifdef __COIN__
    } else if (node->isOfType(SoVRMLImageTexture::getClassTypeId())) {
        texture.filename = ((SoVRMLImageTexture*)node)->url.getNum() >= 1 ?
                    ((SoVRMLImageTexture*)node)->url.getValues(0)[0].getString() : "";
#endif
    } else {
        std::cout << "Unsupported texture type: "
                  << node->getTypeId().getName().getString() << std::endl;
        return texture;
    }
    //std::cout << texture.filename << " -> ";
    if (texture.filename[0] == '\"') texture.filename.erase(texture.filename.begin());
    if (texture.filename.size() > 0 && texture.filename[texture.filename.size()-1] == '\"') {
        texture.filename.erase(texture.filename.begin()+texture.filename.size()-1);
    }
    texture.filename = sceneDir_.relativeFilePath(texture.filename.c_str()).toStdString();
    //std::cout << texture.filename << std::endl;

    if (texture.filename.empty()) {
        //Embedded texture
        SbVec2s size;
        int numComp;
        const unsigned char *pixels(action->getTextureImage(size,numComp));
        if (!pixels) {
            std::cout << "Error while loading texture image." << std::endl;
            return texture;
        }
        texture.image = getTextureImage(pixels,size,numComp);
    } else {
        texture.image = NULL;
    }

    //Get wrap
    texture.wrapS = action->getTextureWrapS();
    texture.wrapT = action->getTextureWrapT();

    return texture;
}


aiTexture *Converter::getTextureImage(const unsigned char *pixels, const SbVec2s &size,
                                      unsigned int numComp) const {
    aiTexture *image(new aiTexture);
    image->mWidth = size[0];
    image->mHeight = size[1];
    image->pcData = new aiTexel[image->mWidth*image->mHeight];
    for (std::size_t i(0); i < image->mWidth; ++i) {
        for (std::size_t j(0); j < image->mHeight; ++j) {
            switch (numComp) {
                case 1://Grayscale
                    image->pcData[image->mHeight*i+j].r = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].g = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].b = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].a = 255;
                    break;
                case 2://Grayscale+Alpha
                    image->pcData[image->mHeight*i+j].r = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].g = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].b = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].a = pixels[numComp*(image->mHeight*i+j)+1];
                    break;
                case 3://RGB
                    image->pcData[image->mHeight*i+j].r = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].g = pixels[numComp*(image->mHeight*i+j)+1];
                    image->pcData[image->mHeight*i+j].b = pixels[numComp*(image->mHeight*i+j)+2];
                    image->pcData[image->mHeight*i+j].a = 255;
                    break;
                case 4://RGBA
                    image->pcData[image->mHeight*i+j].r = pixels[numComp*(image->mHeight*i+j)+0];
                    image->pcData[image->mHeight*i+j].g = pixels[numComp*(image->mHeight*i+j)+1];
                    image->pcData[image->mHeight*i+j].b = pixels[numComp*(image->mHeight*i+j)+2];
                    image->pcData[image->mHeight*i+j].a = pixels[numComp*(image->mHeight*i+j)+3];
                    break;
            }
        }
    }

    return image;
}


Mesh &Converter::getMesh(std::size_t materialIndex) {
    std::map<std::size_t,Mesh>::iterator it(shapeMeshes_.find(materialIndex));
    if (it == shapeMeshes_.end()) {
        Mesh mesh(materialIndex);
        it = shapeMeshes_.insert(std::make_pair(materialIndex,mesh)).first;
    }
    return it->second;
}


void Converter::clear() {
    delete scene_;
    scene_ = NULL;

    while (!nodes_.empty()) nodes_.pop();

    while (!textures_.empty()) textures_.pop();

    shapeMeshes_.clear();

    meshes_.clear();

    materials_.clear();

    materialsMap_.clear();
}
