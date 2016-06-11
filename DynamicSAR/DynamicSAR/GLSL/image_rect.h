#ifndef IMAGE_RECT_H
#define IMAGE_RECT_H


#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>


/**
 * @brief   ��`�𐶐�����N���X
 * 
 * @note	�|�X�g�G�t�F�N�g�������Ŏg�p
 */
class ImageRect
{
public:
	ImageRect(){}

	virtual ~ImageRect(){}

	// ��������
	void init();

	// ��`�̃��T�C�Y
	void resizeRectangle(int width, int height);

	// �`��
	void draw();


	GLfloat position[12];		// �摜�̒��_
	GLuint vboID;				// vbo��ID
};


#endif