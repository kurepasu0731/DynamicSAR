#include "background.h"



/**
 * @brief   ��������(�t�@�C������ǂݍ���)
 * 
 * @param   imageFile[in]		�w�i�摜�t�@�C����
 * @param   vertFile[in]		�o�[�e�b�N�X�t�@�C����
 * @param   fragFile[in]		�t���O�����g�t�@�C����
 * @param   width[in]			�摜�̕�
 * @param   height[in]			�摜�̍���
 */
void Background::init(const std::string& imageFile,const std::string &vertFile, const std::string &fragFile, int width, int height)
{

	// �v���O�����I�u�W�F�N�g���쐬
	init_GLSL( &program, vertFile.c_str(), fragFile.c_str());

	// �e�N�X�`���̓ǂݍ���
	if(!texture.loadImage(GL_TEXTURE_2D, imageFile))
	{
		std::cerr << "�w�i�摜��ǂݍ��߂܂���ł���" << std::endl;
		exit(-1);
	}

	glUseProgram(program);


	// uniform�ϐ��̏ꏊ���擾
	MatrixID = glGetUniformLocation(program, "MVP");

	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	GLint locTexture = glGetUniformLocation(program,"inputImage");
	locW = glGetUniformLocation(program,"imageWidth");
	locH = glGetUniformLocation(program,"imageHeight");

	// uniform�ϐ��ɒl��n��
	glUniform1i(locTexture, 0);
	glUniform1i(locW, width);
	glUniform1i(locH, height);


	///// ���_�z��I�u�W�F�N�g�̍쐬 /////
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vboID);   
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
        
    //�f�[�^��VBO�ɃR�s�[
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), position, GL_DYNAMIC_DRAW);

	// �����̋�`
	resizeRectangle(width, height);

	glUseProgram(0);
}


/**
 * @brief   ��������(Mat����ǂݍ���)
 * 
 * @param   imageFile[in]		�w�i�摜
 * @param   vertFile[in]		�o�[�e�b�N�X�t�@�C����
 * @param   fragFile[in]		�t���O�����g�t�@�C����
 * @param   width[in]			�摜�̕�
 * @param   height[in]			�摜�̍���
 */
void Background::init(const cv::Mat& image,const std::string &vertFile, const std::string &fragFile, int width, int height)
{

	// �v���O�����I�u�W�F�N�g���쐬
	init_GLSL( &program, vertFile.c_str(), fragFile.c_str());

	// �e�N�X�`���̓ǂݍ���
	texture.loadFromMat(GL_TEXTURE_2D, image);

	glUseProgram(program);


	// uniform�ϐ��̏ꏊ���擾
	MatrixID = glGetUniformLocation(program, "MVP");

	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	GLint locTexture = glGetUniformLocation(program,"inputImage");
	locW = glGetUniformLocation(program,"imageWidth");
	locH = glGetUniformLocation(program,"imageHeight");

	// uniform�ϐ��ɒl��n��
	glUniform1i(locTexture, 0);
	glUniform1i(locW, width);
	glUniform1i(locH, height);


	///// ���_�z��I�u�W�F�N�g�̍쐬 /////
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vboID);   
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
        
    //�f�[�^��VBO�ɃR�s�[
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), position, GL_DYNAMIC_DRAW);

	// �����̋�`
	resizeRectangle(width, height);

	glUseProgram(0);
}



/**
 * @brief   ��`�f�[�^�̃��T�C�Y
 * 
 * @param   width[in]			�摜�̕�
 * @param   height[in]			�摜�̍���
 */
void Background::resizeRectangle(int width, int height)
{
	//�A�b�v�f�[�g
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    GLfloat *ptr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if(ptr)
    {       
		ptr[0] = 0.f;
		ptr[1] = 0.f;
		ptr[2] = 0.f;
		ptr[3] = 0.f;
		ptr[4] = (float)height;
		ptr[5] = 0.f;
		ptr[6] = (float)width;
		ptr[7] = (float)height;
		ptr[8] = 0.f;
		ptr[9] = (float)width;
		ptr[10] = 0.f;
		ptr[11] = 0.f;

		glUnmapBufferARB(GL_ARRAY_BUFFER);
    }

	// �摜�T�C�Y���V�F�[�_�ɑ���
	glUseProgram(program);
	glUniform1i(locW, width);
	glUniform1i(locH, height); 

	glUseProgram(0);
}


/**
 * @brief   �V�F�[�_�֍s��̎󂯓n��
 * 
 * @param   matrix[in]			�z�u����s��
 */
void Background::changeMatrix(const glm::mat4 &matrix)
{
	// �V�F�[�_�v���O�����̊J�n
	glUseProgram(program);
	// Uniform�ϐ��ɍs��𑗂�
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &matrix[0][0]);
}


/**
 * @brief   �e�N�X�`���̃A�b�v�f�[�g
 * 
 * @param   image[in]			�e�N�X�`���̍X�V
 */
void Background::textureUpdate(const cv::Mat &image)
{
	texture.updateFromMat(GL_TEXTURE_2D, image);
}


/**
 * @brief   �`��
 */
void Background::draw()
{
	// �V�F�[�_�v���O�����̊J�n
	glUseProgram(program);

	// �e�N�X�`�����o�C���h
	texture.bind(GL_TEXTURE0);

	//VBO�ŕ`��
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
	// �`��
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// �f�t�H���g�ɖ߂�
    glDisableVertexAttribArray(0);
}