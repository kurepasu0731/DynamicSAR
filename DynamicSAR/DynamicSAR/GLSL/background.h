#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "glsl_setup.h"
#include "texture.h"

#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>



/**
 * @brief   �w�i�̕`��N���X
 */
class Background
{

public:
	Background(){};

	// ��������(�t�@�C������ǂݍ���)
	void init(const std::string& imageFile,const std::string &vertFile, const std::string &fragFile, int width, int height);
	// ��������(Mat����ǂݍ���)
	void init(const cv::Mat& image,const std::string &vertFile, const std::string &fragFile, int width, int height);

	// �T�C�Y�̕ύX
	void resizeRectangle(int width, int height);

	// �s��̎󂯓n��
	void changeMatrix(const glm::mat4 &matrix);

	// �e�N�X�`���̍X�V
	void textureUpdate(const cv::Mat &image);

	// �`��
	void draw();


	/***** �����o�ϐ� *****/

	GLuint program;				// �w�i�̃v���O�����I�u�W�F�N�g
	GLfloat position[12];		// �摜�̒��_	
	GLuint vboID;				// vbo��ID

	GLuint locW;				// �V�F�[�_�p�̉摜�̕�
	GLuint locH;				// �V�F�[�_�p�̉摜�̍���

	Texture texture;			// �e�N�X�`���N���X

	GLuint MatrixID;			// �z�u����s��
};


#endif