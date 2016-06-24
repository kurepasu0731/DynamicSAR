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
 * @brief   ���b�V���f�[�^�̊m��
 * 
 * @param   Vertices[in]	���_���	
 * @param   Indices[in]		�C���f�b�N�X���
 */
void Mesh::MeshEntry::init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices)
{
    NumIndices = (unsigned int)Indices.size();

	// VBO�̐���
    glGenBuffers(1, &VBO);
  	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

	// IBO�̐���
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
 * @brief   �e�N�X�`���̊J��
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
 * @brief   ���b�V���f�[�^�̓ǂݍ���
 * 
 * @param   Filename[in]	���b�V���f�[�^�̃t�@�C����
 *
 * @return	�����������ǂ���
 */
bool Mesh::loadMesh(const std::string& Filename)
{
    // ������
    clear();
    
    bool Ret = false;
    Assimp::Importer Importer;

	// �V�[���̓ǂݍ���
    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    
    if (pScene) {
        Ret = initFromScene(pScene, Filename);		// �V�[���̏�������
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    return Ret;
}



/**
 * @brief   �V�[���̏�����
 * 
 * @param   pScene[in]		�V�[��
 * @param   Filename[in]	���b�V���f�[�^�̃t�@�C����
 *
 * @return	�����������ǂ���
 */
bool Mesh::initFromScene(const aiScene* pScene, const std::string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    // ���b�V���̏�����
    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) 
	{
        const aiMesh* paiMesh = pScene->mMeshes[i];
        initMesh(i, paiMesh);
    }

	// �}�e���A���̏�������
    return initMaterials(pScene, Filename);
}


/**
 * @brief   ���b�V���̏�����
 * 
 * @param   Index[in]		���b�V���̔ԍ�
 * @param   paiMesh[in]		���b�V��
 */
void Mesh::initMesh(unsigned int Index, const aiMesh* paiMesh)
{
    m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;	// �}�e���A���ԍ�
    
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// ���_�̓o�^
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

	// VBO�CIBO�̓o�^
    m_Entries[Index].init(Vertices, Indices);
}


/**
 * @brief   �}�e���A���̏�����
 * 
 * @param   pScene[in]		�V�[��
 * @param   Filename[in]	���b�V���f�[�^�̃t�@�C����
 *
 * @return	�����������ǂ���
 */
bool Mesh::initMaterials(const aiScene* pScene, const std::string& Filename)
{
    // �t�@�C��������f�B���N�g���̎擾
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

    // �}�e���A���̏�����
    for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) 
	{
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        m_Textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
		{
            aiString Path;

			// �e�N�X�`���̎擾
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
					m_originalTextures.emplace_back(m_Textures[i]->m_image.clone()); //�ێ��p
                }
            }
        }

        // �I�u�W�F�N�g�Ƀe�N�X�`���������ꍇ�́C���F�̉摜��p����
        if (!m_Textures[i]) 
		{
            m_Textures[i] = new Texture(GL_TEXTURE_2D, "white.png");

            Ret = m_Textures[i]->load();
        }
    }

    return Ret;
}


/**
 * @brief   �����_�����O
 */
void Mesh::render()
{
	// �V�F�[�_�̕ϐ��ɒ��_����Ή��t����
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) 
	{
		// VBO�̎w��
        glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VBO);
		// ���_���̊i�[�ꏊ�Ə����̎w��
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);						// ���_
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);		// UV
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);		// �@��

		// IBO�̎w��
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IBO);

        const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

		// �e�N�X�`���̐ݒ�
        if (MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex]) 
		{
            m_Textures[MaterialIndex]->bind(GL_TEXTURE0);	// �e�N�X�`���̑Ή��t��
        }

		// ���_����]�����ĕ`��
        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

	// ���_�̑Ή��t���̉���
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}


/**
 * @brief   ���_�̂݃����_�����O
 */
void Mesh::vertex_render()
{
	// �V�F�[�_�̕ϐ��ɒ��_����Ή��t����
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) 
	{
		// VBO�̎w��
        glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VBO);
		// ���_���̊i�[�ꏊ�Ə����̎w��
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);						// ���_
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);		// UV
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);		// �@��

		// IBO�̎w��
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IBO);

        const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

		// �e�N�X�`���̐ݒ�
        if (MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex]) 
		{
            m_Textures[MaterialIndex]->bind(GL_TEXTURE0);	// �e�N�X�`���̑Ή��t��
        }

		// ���_����]�����ĕ`��
        glDrawElements(GL_POINTS, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

	// ���_�̑Ή��t���̉���
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

//**written by fujisawa**********************************************************************//
//����Đ��p
void Mesh::exChangeTexture(int model_id, cv::Mat m_image)
{
	const unsigned int MaterialIndex = m_Entries[model_id].MaterialIndex;
	m_Textures[MaterialIndex]->updateFromMat(GL_TEXTURE_2D, m_image);
}

//�f�t�H���g�̃e�N�X�`���ɖ߂�
void Mesh::setDefaultTexture(int model_id)
{
	const unsigned int MaterialIndex = m_Entries[model_id].MaterialIndex;
	m_Textures[MaterialIndex]->updateFromMat(GL_TEXTURE_2D, m_originalTextures[model_id]);
}

//�e�N�X�`���摜�̕ύX
void Mesh::setTexture(int model_id, std::string fileName)
{
	cv::Mat img = cv::imread(fileName);
	const unsigned int MaterialIndex = m_Entries[model_id].MaterialIndex;
	m_Textures[MaterialIndex]->updateFromMat(GL_TEXTURE_2D, img);
}

//*******************************************************************************************//
