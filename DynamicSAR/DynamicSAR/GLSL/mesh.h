#ifndef MESH_H
#define	MESH_H

#include <map>
#include <vector>
#include <GL/glew.h>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>			// Output data structure
#include <assimp/postprocess.h>		// Post processing flags

#include <glm/glm.hpp>

#include "texture.h"


/**
 * @brief   頂点データを扱う構造体
 */
struct Vertex
{
    glm::vec3 m_pos;		// 頂点位置
    glm::vec2 m_tex;		// テクスチャ座標
    glm::vec3 m_normal;		// 法線ベクトル

    Vertex() {}

    Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
    }
};



/**
 * @brief   メッシュを生成するクラス
 */
class Mesh
{
public:
    Mesh();

    ~Mesh();

	// ファイルからメッシュを読み込む
    bool loadMesh(const std::string& Filename);

	// レンダリング
    void render();

	// 頂点のみレンダリング
	void vertex_render();
	
	//動画再生用:テクスチャ画像の更新
	void exChangeTexture(int model_id, cv::Mat m_image);

	//デフォルトのテクスチャに戻す
	void setDefaultTexture(int model_id);

private:
    bool initFromScene(const aiScene* pScene, const std::string& Filename);
    void initMesh(unsigned int Index, const aiMesh* paiMesh);
    bool initMaterials(const aiScene* pScene, const std::string& Filename);
    void clear();

	// メッシュのデータ
    struct MeshEntry {
        MeshEntry();

        ~MeshEntry();

        void init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices);

        GLuint VBO;						// Vertex Buffer Object
        GLuint IBO;						// Index Buffer Object
        unsigned int NumIndices;		// インデックスの数
        unsigned int MaterialIndex;		// マテリアルの番号
    };

    std::vector<MeshEntry> m_Entries;	// オブジェクトのメッシュデータ
    std::vector<Texture*> m_Textures;	// オブジェクトのテクスチャデータ

    std::vector<cv::Mat> m_originalTextures;	// オブジェクトの初期テクスチャデータ(保持用)
};


#endif	/* MESH_H */