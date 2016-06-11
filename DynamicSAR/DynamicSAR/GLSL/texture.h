#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>

#include <GL/glew.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


/**
 * @brief   �e�N�X�`���̐ݒ�N���X
 */
class Texture
{
public:
	Texture(){}
    Texture(GLenum TextureTarget, const std::string& FileName)
	{
		m_textureTarget = TextureTarget;
		m_fileName      = FileName;
	};

    bool load();					// �e�N�X�`���̓ǂݍ���

	bool loadImage(GLenum TextureTarget, const std::string& FileName);		// �e�N�X�`���̓ǂݍ���

	void loadFromMat(GLenum TextureTarget, const cv::Mat& image);			// cv::Mat���e�N�X�`���Ƃ��ėp����
	void updateFromMat(GLenum TextureTarget, const cv::Mat& image);			// cv::Mat���X�V

    void bind(GLenum TextureUnit);	// �e�N�X�`���̑Ή��t��


    std::string m_fileName;			// �t�@�C����
    GLenum m_textureTarget;			// �e�N�X�`���̃^�C�v
    GLuint m_textureObj;			// �e�N�X�`���̖��O
    cv::Mat m_image;				// �e�N�X�`���̃f�[�^
	int width;						// �e�N�X�`���̕�
	int height;						// �e�N�X�`���̍���
};


#endif