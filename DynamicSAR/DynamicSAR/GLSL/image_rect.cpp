#include "image_rect.h"


/**
 * @brief   ��������
 */
void ImageRect::init()
{
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vboID);   
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
        
    //�f�[�^��VBO�ɃR�s�[
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), position, GL_DYNAMIC_DRAW);
}


/**
 * @brief   ��`�̃f�[�^�̃��T�C�Y
 * 
 * @param   width[in]		����
 * @param   height[in]		�c��
 */
void ImageRect::resizeRectangle(int width, int height)
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
}


/**
 * @brief   �`��
 */
void ImageRect::draw()
{
	// VBO�ŕ`��
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
	// �`��
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// �f�t�H���g�ɖ߂�
    glDisableVertexAttribArray(0);
}