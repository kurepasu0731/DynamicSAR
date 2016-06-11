#include "primitive.h"




/**
 * @brief   ���_�o�b�t�@�I�u�W�F�N�g�̏����ݒ�
 * 
 * @param   vertex_vect[in]		���_���
 * @param   normal_vect[in]		�@�����
 * @param   index_vect[in]		�C���f�b�N�X���
 */
void Primitive::init(const std::vector<glm::vec3> &vertex_vect, const std::vector<glm::vec3> &normal_vect, const std::vector<unsigned int> &index_vect)
{
	// VBO�̐���
    glGenBuffers(2, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertex_vect.size(), &vertex_vect[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normal_vect.size(), &normal_vect[0], GL_STATIC_DRAW);

	// IBO�̐���
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*index_vect.size(), &index_vect[0], GL_STATIC_DRAW);

	//�@�o�C���h�������̂����ǂ�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



/**
 * @brief   ���̏�����
 * 
 * @param   division[in]		������
 */
void Primitive::initSphere(int division)
{
	if (division <= 0)
		division = 1;

	std::vector<glm::vec3> vertex_vect;			// ���_
	std::vector<glm::vec3> normal_vect;			// �@��
	std::vector<unsigned int> index_vect;		// �C���f�b�N�X


	// �����̃x�N�g��
	std::vector<glm::vec3> init_vectors(24);
	init_vectors[0] = glm::vec3(1.0, 0.0, 0.0);		// (1,1,1)
	init_vectors[1] = glm::vec3(0.0, 1.0, 0.0);
	init_vectors[2] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[3] = glm::vec3(0.0, 1.0, 0.0);		// (-1,1,1)
	init_vectors[4] = glm::vec3(-1.0, 0.0, 0.0);
	init_vectors[5] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[6] = glm::vec3(-1.0, 0.0, 0.0);	// (-1,-1,1)
	init_vectors[7] = glm::vec3(0.0, -1.0, 0.0);
	init_vectors[8] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[9] = glm::vec3(0.0, -1.0, 0.0);	// (1,-1,1)
	init_vectors[10] = glm::vec3(1.0, 0.0, 0.0);
	init_vectors[11] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[12] = glm::vec3(0.0, 1.0, 0.0);	// (1,1,-1)
	init_vectors[13] = glm::vec3(1.0, 0.0, 0.0);
	init_vectors[14] = glm::vec3(0.0, 0.0, -1.0);
	init_vectors[15] = glm::vec3(-1.0, 0.0, 0.0);	// (-1,1,-1)
	init_vectors[16] = glm::vec3(0.0, 1.0, 0.0);
	init_vectors[17] = glm::vec3(0.0, 0.0, -1.0);
	init_vectors[18] = glm::vec3(0.0, -1.0, 0.0);	// (-1,-1,-1)
	init_vectors[19] = glm::vec3(-1.0, 0.0, 0.0);
	init_vectors[20] = glm::vec3(0.0, 0.0, -1.0);
	init_vectors[21] = glm::vec3(1.0, 0.0, 0.0);	// (1,-1,-1)
	init_vectors[22] = glm::vec3(0.0, -1.0, 0.0);
	init_vectors[23] = glm::vec3(0.0, 0.0, -1.0);



	int j = 0;

	// ��8�ʑ̂̊e�O�p�`�𕪊�
	for (int i = 0; i < 24; i+=3)
	{
		// ����1
		for (int p = 0; p < division; ++p)
		{			
			glm::vec3 edge_p1 = glm::slerp<float>(init_vectors[i], init_vectors[i+1], (float)p/(float)division);			// p left
			glm::vec3 edge_p2 = glm::slerp<float>(init_vectors[i+2], init_vectors[i+1], (float)p/(float)division);			// p right
			glm::vec3 edge_p3 = glm::slerp<float>(init_vectors[i], init_vectors[i+1], (float)(p+1)/(float)division);		// p+1 left
			glm::vec3 edge_p4 = glm::slerp<float>(init_vectors[i+2], init_vectors[i+1], (float)(p+1)/(float)division);		// p+1 right

			// ����2
			for (int q = 0; q <(division-p); ++q)
			{
				glm::vec3 a, b, c, d;
				a = glm::slerp<float>(edge_p1, edge_p2, (float)q/(float)(division-p));
				b = glm::slerp<float>(edge_p1, edge_p2, (float)(q+1)/(float)(division-p));
				if (edge_p3 == edge_p4)
				{
					c = edge_p3;
					d = edge_p4;
				} else {
					c = glm::slerp<float>(edge_p3, edge_p4, (float)q/(float)(division-p-1));
					d = glm::slerp<float>(edge_p3, edge_p4, (float)(q+1)/(float)(division-p-1));
				}

				vertex_vect.push_back(a);
				vertex_vect.push_back(b);
				vertex_vect.push_back(c);
				index_vect.push_back(j++);
				index_vect.push_back(j++);
				index_vect.push_back(j++);

				if (q < division-p-1) {
					vertex_vect.push_back(c);
					vertex_vect.push_back(b);
					vertex_vect.push_back(d);
					index_vect.push_back(j++);
					index_vect.push_back(j++);
					index_vect.push_back(j++);
				}
			}
		}
	}

	// �C���f�b�N�X�̃T�C�Y
	index_size = (unsigned int)index_vect.size();

	// ���_�@�������߂�
	calcVertexNormals(vertex_vect, index_vect, normal_vect);

	// vbo ibo�̏����ݒ�
	init(vertex_vect, normal_vect, index_vect);
}


 /**
 * @brief   �{�b�N�X�̏�����
 */
void Primitive::initCube()
{
	std::vector<glm::vec3> vertex_vect(8);	// ���_

	vertex_vect[0] = glm::vec3(-1.0f, -1.0f, -1.0f);
	vertex_vect[1] = glm::vec3(-1.0f, -1.0f,  1.0f);
	vertex_vect[2] = glm::vec3(-1.0f,  1.0f,  1.0f);
	vertex_vect[3] = glm::vec3(-1.0f,  1.0f, -1.0f);
	vertex_vect[4] = glm::vec3( 1.0f,  1.0f, -1.0f);
	vertex_vect[5] = glm::vec3( 1.0f, -1.0f, -1.0f);
	vertex_vect[6] = glm::vec3( 1.0f, -1.0f,  1.0f);
	vertex_vect[7] = glm::vec3( 1.0f,  1.0f,  1.0f);


	static const GLuint index[] = {
		0, 1, 2,
		2, 3, 0,
		5, 0, 3,
		3, 4, 5,
		5, 6, 1,
		1, 0, 5,
		6, 5, 4,
		4, 7, 6,
		7, 2, 1,
		1, 6, 7,
		7, 4, 3,
		3, 2, 7
	};

	std::vector<glm::vec3> normal_vect;		// �@��
	std::vector<unsigned int> index_vect(index, std::end(index));	// �C���f�b�N�X

	// �C���f�b�N�X�̃T�C�Y
	index_size = (unsigned int)index_vect.size();

	// ���_�@�������߂�
	calcVertexNormals(vertex_vect, index_vect, normal_vect);

	// vbo ibo�̏����ݒ�
	init(vertex_vect, normal_vect, index_vect);
}


/**
 * @brief   ���[�U��`�̔z��̏�����
 * 
 * @param   vertex_vect[in]		���_���
 */
void Primitive::initVector(const std::vector<glm::vec3> &vertex_vect)
{

	std::vector<glm::vec3> normal_vect;		// �@��
	std::vector<unsigned int> index_vect(vertex_vect.size());	// �C���f�b�N�X

	// �C���f�b�N�X�͓��͔z��̏��Ԃ����̂܂ܗp����
	for (int i = 0; i < index_vect.size(); ++i)
	{
		index_vect[i] = i;
	}

	// �C���f�b�N�X�̃T�C�Y
	index_size = (unsigned int)index_vect.size();

	// ���_�@�������߂�
	calcVertexNormals(vertex_vect, index_vect, normal_vect);

	// vbo ibo�̏����ݒ�
	init(vertex_vect, normal_vect, index_vect);
}


/**
 * @brief	�����_�����O
 */
void Primitive::render()
{
	// �V�F�[�_�̕ϐ��ɒ��_����Ή��t����
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// VBO�̎w��
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	// ���_���̊i�[�ꏊ�Ə����̎w��
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// ���_

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	// ���_���̊i�[�ꏊ�Ə����̎w��
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);			// �@��

	// IBO�̎w��
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDrawElements(GL_TRIANGLES, index_size, GL_UNSIGNED_INT, (void*)0);


	// ���_�̑Ή��t���̉���
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

	//�@�o�C���h�������̂����ǂ�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/**
 * @brief	���C���[�t���[���ɂ�郌���_�����O
 */
void Primitive::render_wire()
{
	// ���C���[�t���[���Ń����_�����O
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );

	// �����_�����O
	render();

	// ���ɖ߂�
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
}


/**
 * @brief	�_�ɂ�郌���_�����O
 */
void Primitive::render_point(GLfloat size)
{
	// �_�̑傫��
	glPointSize(size);

	// �_�Ń����_�����O
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT );

	// �����_�����O
	render();

	// ���ɖ߂�
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
}


/**
 * @brief   ���_�ƃ|���S���̃C���f�b�N�X���璸�_�@�������߂�
 * 
 * @param   vertex_vect[in]			���_���
 * @param   index_vect[in]			�C���f�b�N�X���
 * @param   normal_vect[in,out]		�@�����
 */
void Primitive::calcVertexNormals(const std::vector<glm::vec3> &vertex_vect, const std::vector<unsigned int> &index_vect, std::vector<glm::vec3> &normal_vect)
{
	// ������
	normal_vect.clear();
	normal_vect.resize(vertex_vect.size(), glm::vec3(0.0f,0.0f,0.0f));

	// ���_�̖@�������߂�
	for(unsigned int n = 0; n < normal_vect.size(); ++n)
	{
		glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);

		// �C���f�b�N�X�̒����玩�g�̔ԍ���T��
		for(unsigned int i = 0; i < index_size; i = i+3)
		{
			int index1 = index_vect.at(i);
			int index2 = index_vect.at(i+1);
			int index3 = index_vect.at(i+2);

			// �O�p�`�̒��_�Ɋ܂܂�Ă�����
			if(n==index1 || n==index2 || n==index3)
			{
				// �@�������߂�
				glm::vec3 normal0 = glm::normalize(glm::cross(
					vertex_vect[index2] - vertex_vect[index1],
					vertex_vect[index3] - vertex_vect[index1]));

				// �@���̑��a
				normal += normal0;
			}
		}

		// ���K��
		normal = glm::normalize(normal);

		// �ǉ�
		normal_vect[n] = normal;
	}
}