#include <assert.h>

#include "mesh.h"


Mesh::MeshEntry::MeshEntry()
{
    VBO = 0xffffffff;
    IBO = 0xffffffff;
    NumIndices  = 0;
    MaterialIndex = 0xFFFFFFFF;
};

Mesh::MeshEntry::~MeshEntry()
{
    if (VBO != 0xffffffff)
    {
        glDeleteBuffers(1, &VBO);
    }

    if (IBO != 0xffffffff)
    {
        glDeleteBuffers(1, &IBO);
    }
}


/**
 * @brief   メッシュデータの確保
 * 
 * @param   Vertices[in]	頂点情報	
 * @param   Indices[in]		インデックス情報
 */
void Mesh::MeshEntry::init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices)
{
    NumIndices = (unsigned int)Indices.size();

	// VBOの生成
    glGenBuffers(1, &VBO);
  	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// IBOの生成
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &Indices[0], GL_STATIC_DRAW);
}



Mesh::Mesh()
{
}


Mesh::~Mesh()
{
    clear();
}


/**
 * @brief   テクスチャの開放
 */
void Mesh::clear()
{
    for (unsigned int i = 0 ; i < m_Textures.size() ; i++) 
	{
        if(m_Textures[i])
		{
			delete m_Textures[i];
			m_Textures[i]  = NULL;
		}
    }
}


/**
 * @brief   メッシュデータの読み込み
 * 
 * @param   Filename[in]	メッシュデータのファイル名
 *
 * @return	成功したかどうか
 */
bool Mesh::loadMesh(const std::string& Filename)
{
    // 初期化
    clear();
    
    bool Ret = false;
    Assimp::Importer Importer;

	// シーンの読み込み
    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    
    if (pScene) {
        Ret = initFromScene(pScene, Filename);		// シーンの初期処理
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    return Ret;
}



/**
 * @brief   シーンの初期化
 * 
 * @param   pScene[in]		シーン
 * @param   Filename[in]	メッシュデータのファイル名
 *
 * @return	成功したかどうか
 */
bool Mesh::initFromScene(const aiScene* pScene, const std::string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    // メッシュの初期化
    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) 
	{
        const aiMesh* paiMesh = pScene->mMeshes[i];
        initMesh(i, paiMesh);
    }

	// マテリアルの初期処理
    return initMaterials(pScene, Filename);
}


/**
 * @brief   メッシュの初期化
 * 
 * @param   Index[in]		メッシュの番号
 * @param   paiMesh[in]		メッシュ
 */
void Mesh::initMesh(unsigned int Index, const aiMesh* paiMesh)
{
    m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;	// マテリアル番号
    
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// 頂点の登録
    for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) 
	{
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        Vertex v(glm::vec3(pPos->x, pPos->y, pPos->z),
                 glm::vec2(pTexCoord->x, pTexCoord->y),
                 glm::vec3(pNormal->x, pNormal->y, pNormal->z));

        Vertices.push_back(v);
    }

    for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++) 
	{
        const aiFace& Face = paiMesh->mFaces[i];

        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }

	// VBO，IBOの登録
    m_Entries[Index].init(Vertices, Indices);
}


/**
 * @brief   マテリアルの初期化
 * 
 * @param   pScene[in]		シーン
 * @param   Filename[in]	メッシュデータのファイル名
 *
 * @return	成功したかどうか
 */
bool Mesh::initMaterials(const aiScene* pScene, const std::string& Filename)
{
    // ファイルがあるディレクトリの取得
    std::string::size_type SlashIndex = Filename.find_last_of("/");
    std::string Dir;

    if (SlashIndex == std::string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

	//**written by fujisawa********//
	m_originalTextures.clear();

    // マテリアルの初期化
    for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) 
	{
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
		{
            aiString Path;

			// テクスチャの取得
            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) 
			{
                std::string FullPath = Dir + "/" + Path.data;
                m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                if (!m_Textures[i]->load()) 
				{
                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    Ret = false;
                }
                else 
				{
                    printf("Loaded texture '%s'\n", FullPath.c_str());
					//**written by fujisawa********//
					m_originalTextures.emplace_back(m_Textures[i]->m_image.clone()); //保持用
                }
            }
        }

        // オブジェクトにテクスチャが無い場合は，白色の画像を用いる
        if (!m_Textures[i]) 
		{
            m_Textures[i] = new Texture(GL_TEXTURE_2D, "white.png");

            Ret = m_Textures[i]->load();
        }
    }

    return Ret;
}


/**
 * @brief   レンダリング
 */
void Mesh::render()
{
	// シェーダの変数に頂点情報を対応付ける
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) 
	{
		// VBOの指定
        glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VBO);
		// 頂点情報の格納場所と書式の指定
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);						// 頂点
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);		// UV
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);		// 法線

		// IBOの指定
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IBO);

        const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

		// テクスチャの設定
        if (MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex]) 
		{
            m_Textures[MaterialIndex]->bind(GL_TEXTURE0);	// テクスチャの対応付け
        }

		// 頂点情報を転送して描画
        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

	// 頂点の対応付けの解除
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}


/**
 * @brief   頂点のみレンダリング
 */
void Mesh::vertex_render()
{
	// シェーダの変数に頂点情報を対応付ける
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) 
	{
		// VBOの指定
        glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VBO);
		// 頂点情報の格納場所と書式の指定
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);						// 頂点
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);		// UV
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);		// 法線

		// IBOの指定
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IBO);

        const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

		// テクスチャの設定
        if (MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex]) 
		{
            m_Textures[MaterialIndex]->bind(GL_TEXTURE0);	// テクスチャの対応付け
        }

		// 頂点情報を転送して描画
        glDrawElements(GL_POINTS, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

	// 頂点の対応付けの解除
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

//**written by fujisawa**********************************************************************//
//動画再生用
void Mesh::exChangeTexture(int model_id, cv::Mat m_image)
{
	const unsigned int MaterialIndex = m_Entries[model_id].MaterialIndex;
	m_Textures[MaterialIndex]->updateFromMat(GL_TEXTURE_2D, m_image);
}

//デフォルトのテクスチャに戻す
void Mesh::setDefaultTexture(int model_id)
{
	const unsigned int MaterialIndex = m_Entries[model_id].MaterialIndex;
	m_Textures[MaterialIndex]->updateFromMat(GL_TEXTURE_2D, m_originalTextures[model_id]);
}

//テクスチャ画像の変更
void Mesh::setTexture(int model_id, std::string fileName)
{
	cv::Mat img = cv::imread(fileName);
	const unsigned int MaterialIndex = m_Entries[model_id].MaterialIndex;
	m_Textures[MaterialIndex]->updateFromMat(GL_TEXTURE_2D, img);
}

//*******************************************************************************************//
