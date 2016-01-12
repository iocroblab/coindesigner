#ifndef CONVERTER_H
#define CONVERTER_H

#include <assimp/scene.h>

#include <Inventor/actions/SoCallbackAction.h>

#include <stack>
#include <vector>
#include <map>
#include <set>

#include <QDir>


struct Texture {
    Texture() : image(NULL), wrapS(SoTexture2::REPEAT), wrapT(SoTexture2::REPEAT) {}
    std::string filename;
    aiTexture* image;
    SoTexture2::Wrap wrapS;
    SoTexture2::Wrap wrapT;
};


struct Material {
    SbColor ambient;
    SbColor diffuse;
    SbColor specular;
    SbColor emissive;
    float shininess;
    float transparency;
    Texture texture;
};


struct Vertex {
    aiVector3D coord;
    aiVector3D normal;
    aiVector3D texCoord;
};


class Mesh {
public:
    Mesh(std::size_t material) : materialIndex(material), primitiveTypes_(0) {
    }

    std::size_t addVertex(const Vertex &vertex) {
        std::map<Vertex,std::size_t>::const_iterator it(verticesMap_.find(vertex));
        if (it != verticesMap_.end()) {
            return it->second;
        } else {
            std::size_t index(vertices_.size());
            vertices_.push_back(vertex);
            verticesMap_.insert(std::make_pair(vertex,index));
            return index;
        }
    }

    const std::vector<Vertex> &getVertices() const {
        return vertices_;
    }

    void addFace(const aiFace &face) {
        if (face.mNumIndices > 0 && facesSet_.find(face) == facesSet_.end()) {
            switch (face.mNumIndices) {
            case 1:
                primitiveTypes_ = primitiveTypes_ | aiPrimitiveType_POINT;
                break;
            case 2:
                primitiveTypes_ = primitiveTypes_ | aiPrimitiveType_LINE;
                break;
            case 3:
                primitiveTypes_ = primitiveTypes_ | aiPrimitiveType_TRIANGLE;
                break;
            default:
                primitiveTypes_ = primitiveTypes_ | aiPrimitiveType_POLYGON;
                break;
            }
            faces_.push_back(face);
            facesSet_.insert(face);
        }
    }

    const std::vector<aiFace> &getFaces() const {
        return faces_;
    }

    unsigned int getPrimitiveTypes() const {
        return primitiveTypes_;
    }


    const std::size_t materialIndex;

protected:
    unsigned int primitiveTypes_;

    std::vector<Vertex> vertices_;

    struct cmpVertex {
        bool operator()(const aiVector3D &a, const aiVector3D &b) const {
            return a.x < b.x ||
                    (a.x == b.x &&
                     (a.y < b.y ||
                      (a.y == b.y &&
                       a.z < b.z)));
        }

        bool operator()(const Vertex &a, const Vertex &b) const {
            return cmpVertex()(a.coord,b.coord) ||
                    (!cmpVertex()(b.coord,a.coord) &&
                     (cmpVertex()(a.normal,b.normal) ||
                      (!cmpVertex()(b.normal,a.normal) &&
                       cmpVertex()(a.texCoord,b.texCoord))));
        }
    };

    std::map<Vertex,std::size_t,cmpVertex> verticesMap_;

    std::vector<aiFace> faces_;

    struct cmpFace {
        bool operator()(const aiFace &a, const aiFace &b) const {
            if (a.mNumIndices < b.mNumIndices) return true;

            for (unsigned int i(0); i < b.mNumIndices; ++i) {
                if (a.mIndices[i] < b.mIndices[i]) return true;
            }

            return false;
        }
    };

    std::set<aiFace,cmpFace> facesSet_;
};


class Converter {
public:
    Converter();

    ~Converter();

    aiScene *convert(SoNode *root, QDir const &sceneDir);

    void addFace(const SoCallbackAction *const action, const SoPrimitiveVertex *const *const vertices,
                 unsigned int numIndices);

    aiScene *scene_;

    std::stack<aiNode*> nodes_;

    std::stack<const SoNode*> textures_;

    std::map<std::size_t,Mesh> shapeMeshes_;

    std::vector<aiMesh*> meshes_;

    std::vector<aiMaterial*> materials_;

    QDir sceneDir_;

    struct cmpMaterial {
        bool operator()(const SbColor &a, const SbColor &b) const {
            return a[0] < b[0] ||
                    (a[0] == b[0] &&
                    (a[1] < b[1] ||
                    (a[1] == b[1] &&
                    a[2] < b[2])));
        }

        bool operator()(const Texture &a, const Texture &b) const {
            return a.filename < b.filename ||
                    (a.filename == b.filename &&
                     (a.wrapS < b.wrapS ||
                      (a.wrapS == b.wrapS &&
                       (a.wrapT < b.wrapT ||
                        (a.wrapT == b.wrapT &&
                         a.image < b.image)))));
        }

        bool operator()(const Material &a, const Material &b) const {
            return cmpMaterial()(a.ambient,b.ambient) ||
                    (!cmpMaterial()(b.ambient,a.ambient) &&
                     (cmpMaterial()(a.diffuse,b.diffuse) ||
                      (!cmpMaterial()(b.diffuse,a.diffuse) &&
                       (cmpMaterial()(a.emissive,b.emissive) ||
                        (!cmpMaterial()(b.emissive,a.emissive) &&
                         (cmpMaterial()(a.specular,b.specular) ||
                          (!cmpMaterial()(b.specular,a.specular) &&
                           (a.shininess < b.shininess ||
                            (a.shininess == b.shininess &&
                             (a.transparency < b.transparency ||
                              (a.transparency == b.transparency &&
                               cmpMaterial()(a.texture,b.texture))))))))))));
        }
    };

    std::map<Material,std::size_t,cmpMaterial> materialsMap_;

protected:
    aiMaterial *getMaterial(const Material &material) const;

    std::size_t addMaterial(const Material &material);

    Texture getTexture(const SoNode* soNode, const SoCallbackAction *action) const;

    void addTexture(aiMaterial *material,const Texture &texture) const;

    aiTexture *getTextureImage(const unsigned char *pixels, const SbVec2s &size,
                               unsigned int numComp) const;

    Mesh &getMesh(std::size_t materialIndex);

    void clear();
};

#endif // CONVERTER_H
