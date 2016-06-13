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
 * @brief   ���_�f�[�^�������\����
 */
struct Vertex
{
    glm::vec3 m_pos;		// ���_�ʒu
    glm::vec2 m_tex;		// �e�N�X�`�����W
    glm::vec3 m_normal;		// �@���x�N�g��

    Vertex() {}

    Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
    }
};



/**
 * @brief   ���b�V���𐶐�����N���X
 */
class Mesh
{
public:
    Mesh();

    ~Mesh();

	// �t�@�C�����烁�b�V����ǂݍ���
    bool loadMesh(const std::string& Filename);

	// �����_�����O
    void render();

	// ���_�̂݃����_�����O
	void vertex_render();
	
	//����Đ��p:�e�N�X�`���摜�̍X�V
	void exChangeTexture(int model_id, cv::Mat m_image);

	//�f�t�H���g�̃e�N�X�`���ɖ߂�
	void setDefaultTexture(int model_id);

private:
    bool initFromScene(const aiScene* pScene, const std::string& Filename);
    void initMesh(unsigned int Index, const aiMesh* paiMesh);
    bool initMaterials(const aiScene* pScene, const std::string& Filename);
    void clear();

	// ���b�V���̃f�[�^
    struct MeshEntry {
        MeshEntry();

        ~MeshEntry();

        void init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices);

        GLuint VBO;						// Vertex Buffer Object
        GLuint IBO;						// Index Buffer Object
        unsigned int NumIndices;		// �C���f�b�N�X�̐�
        unsigned int MaterialIndex;		// �}�e���A���̔ԍ�
    };

    std::vector<MeshEntry> m_Entries;	// �I�u�W�F�N�g�̃��b�V���f�[�^
    std::vector<Texture*> m_Textures;	// �I�u�W�F�N�g�̃e�N�X�`���f�[�^

    std::vector<cv::Mat> m_originalTextures;	// �I�u�W�F�N�g�̏����e�N�X�`���f�[�^(�ێ��p)
};


#endif	/* MESH_H */