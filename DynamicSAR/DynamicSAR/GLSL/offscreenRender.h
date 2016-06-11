#ifndef OFFSCREENRENDER_H
#define OFFSCREENRENDER_H

#include <iostream>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


/**
 * @brief   �I�t�X�N���[�������_�����O����N���X
 * 
 * @note	RGB,Depth�ł̃����_�����O�ɑΉ�	
 *			�}���`�T���v���A���`�G�C���A�V���O�ɑΉ�
 */
class OffscreenRender
{
public:
	OffscreenRender()
		: MSAA_flag (false)
		, offscreen_width (128)
		, offscreen_height (128)
	{}

	virtual ~OffscreenRender(){};

	void init(int width, int height, bool MSAA=false, int sanmple=16);					// ��������

	inline void startRender()							// �I�t�X�N���[�������_�����O�J�n
	{
		// �`���̐؂�ւ�
		if(MSAA_flag)
			glBindFramebuffer(GL_FRAMEBUFFER, fboMSAAID);
		else
			glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	}

	inline void endRender()								// �I�t�X�N���[�������_�����O�I��
	{
		// �`�����f�t�H���g��
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// �����_�����O�T�C�Y�̕ύX
	void changeRenderSize(int width, int height, bool MSAA=false, int sanmple=16);

	// �I�t�X�N���[�������_�����O�̌��ʂ�ۑ�
	void saveRenderRGB(const std::string& fileName);									// �����_�����O���ʂ̕ۑ�(RGB)
	void saveRenderRGB_Clip(const std::string& fileName, int width, int height);		// �����_�����O���ʂ̕ۑ�(RGB)(�����̈���N���b�s���O)
	void saveRenderDepth(const std::string& fileName);									// �����_�����O���ʂ̕ۑ�(depth)
	void saveRenderDepthXML(const std::string& fileName, float near, float far);		// �����_�����O���ʂ̕ۑ�(depth��xml)
	void saveRenderDepthXML_Clip(const std::string& fileName, float near, float far, int width, int height);	// �����_�����O���ʂ̕ۑ�(depth��xml)�����̈���N���b�s���O)
	void saveRenderPointCloud(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix);	// �����_�����O���ʂ̕ۑ�(Point��xml)
	void saveRenderPointCloud_Clip(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, int width, int height);	// �����_�����O���ʂ̕ۑ�(Point��xml)(�����̈���N���b�s���O)

	// �I�t�X�N���[�������_�����O�̌��ʂ�Ԃ�
	void getRenderRGB(cv::Mat& colorMat);
	void getRenderDepth(cv::Mat& depthMat);
	void getRenderNormal(cv::Mat& normalMat);
	void getRenderPointCloud(cv::Mat& pointcloudMat, glm::mat4& viewMatrix, glm::mat4& projectionMatrix);

	bool checkFramebufferStatus();						// �t���[���o�b�t�@�̎擾�󋵂̊m�F


	GLuint fboID;					// �t���[���o�b�t�@
	GLuint rboID[2];				// �����_�[�o�b�t�@
	GLuint texID[2];				// �e�N�X�`���o�b�t�@
	GLuint fboMSAAID;				// �t���[���o�b�t�@(�}���`�T���v���A���`�G�C���A�V���O)
	GLuint rboMSAAID[2];			// �����_�[�o�b�t�@(�}���`�T���v���A���`�G�C���A�V���O)
	bool MSAA_flag;					// �}���`�T���v���A���`�G�C���A�V���O���g�����ǂ���
	int offscreen_width;			// �I�t�X�N���[���̉���
	int offscreen_height;			// �I�t�X�N���[���̏c��
};

#endif