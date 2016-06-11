#ifndef PRIMITIVE_H
#define	PRIMITIVE_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>

/**
 * @brief   �v���~�e�B�u�Ȑ}�`�𐶐�����N���X
 */
class Primitive
{

public:
	Primitive()
	{
		VBO[0] = 0xffffffff;
		IBO = 0xffffffff;
	};

	~Primitive()
	{
		if (VBO[0] != 0xffffffff){
			glDeleteBuffers(2, VBO);
		}

		if (IBO != 0xffffffff){
			glDeleteBuffers(1, &IBO);
		}
	};

	// ���_�o�b�t�@�I�u�W�F�N�g�̏����ݒ�
	void init(const std::vector<glm::vec3> &vertex_vect, const std::vector<glm::vec3> &normal_vect, const std::vector<unsigned int> &index_vect);

	// ���̏�����
	void initSphere(int division);

	// �{�b�N�X�̏�����
	void initCube();

	// ���[�U��`�̔z��̏�����
	void initVector(const std::vector<glm::vec3> &vertex_vect);

	// �����_�����O
    void render();

	// ���C���[�t���[���ɂ�郌���_�����O
	void render_wire();

	// �_�ɂ�郌���_�����O
	void render_point(GLfloat size);

	// ���_�ƃ|���S���̃C���f�b�N�X���璸�_�@�������߂�
	void calcVertexNormals(const std::vector<glm::vec3> &vertex_vect, const std::vector<unsigned int> &index_vect, std::vector<glm::vec3> &normal_vect);

private:
	GLuint VBO[2];		// Vertex Buffer Object
	GLuint IBO;			// Index Buffer Object
	unsigned int index_size; 
};


#endif