#include <iostream>
#include "texture.h"




/**
 * @brief   �e�N�X�`���̓ǂݍ���
 *
 * @returns �ǂݍ��ݐ������ǂ���
 */
bool Texture::load()
{
	// OpenCV�ɂ��摜�ǂݍ���
	cv::Ptr<IplImage> iplimg = cvLoadImage(m_fileName.c_str(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cv::Mat fileImage = cv::cvarrToMat(iplimg);
    m_image = fileImage.clone();
   
    if (m_image.empty()) {
        std::cout << "Error loading texture '" << m_fileName  << std::endl;
        return false;
    }

	if(m_image.channels() == 3)
		cv::cvtColor(m_image, m_image, CV_BGR2BGRA);	// 4�`�����l���ɕϊ�

    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.size().width, m_image.size().height, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_image.data);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(m_textureTarget, 0);
    
	width = m_image.cols;
	height = m_image.rows;

    return true;
}



/**
 * @brief   �e�N�X�`���̓ǂݍ���
 * 
 * @param   TextureTarget[in]	�e�N�X�`���̃^�C�v
 * @param   FileName[in]		�e�N�X�`���̃t�@�C����	
 * 
 * @returns �ǂݍ��ݐ������ǂ���
 */
bool Texture::loadImage(GLenum TextureTarget, const std::string& FileName)
{
	m_textureTarget = TextureTarget;
    m_fileName      = FileName;

	// OpenCV�ɂ��摜�ǂݍ���
	cv::Ptr<IplImage> iplimg = cvLoadImage(m_fileName.c_str(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cv::Mat fileImage = cv::cvarrToMat(iplimg);
    m_image = fileImage.clone();
   
    if (m_image.empty()) {
        std::cout << "Error loading texture '" << m_fileName  << std::endl;
        return false;
    }

	if(m_image.channels() == 4)
		cv::cvtColor(m_image, m_image, CV_BGRA2BGR);	// 3�`�����l���ɕϊ�

    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(m_textureTarget, 0, GL_RGB, m_image.size().width, m_image.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, m_image.data);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(m_textureTarget, 0);
    
	width = m_image.cols;
	height = m_image.rows;

    return true;
}


/**
 * @brief   cv::Mat���e�N�X�`���Ƃ��ėp����
 * 
 * @param   TextureTarget[in]	�e�N�X�`���̃^�C�v
 * @param   image[in]			�e�N�X�`���Ƃ��ėp����摜	
 * 
 * @returns �ǂݍ��ݐ������ǂ���
 */
void Texture::loadFromMat(GLenum TextureTarget, const cv::Mat& image)
{
	m_textureTarget = TextureTarget;
	m_image = image.clone();

	if(m_image.channels() == 1)
		cv::cvtColor(m_image, m_image, CV_GRAY2BGR);	// 3�`�����l���ɕϊ�

	if(m_image.channels() == 4)
		cv::cvtColor(m_image, m_image, CV_BGRA2BGR);	// 3�`�����l���ɕϊ�

    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(m_textureTarget, 0, GL_RGB, m_image.size().width, m_image.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, m_image.data);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(m_textureTarget, 0);
    
	width = m_image.cols;
	height = m_image.rows;
}


/**
 * @brief   cv::Mat���X�V
 * 
 * @param   TextureTarget[in]	�e�N�X�`���̃^�C�v
 * @param   image[in]			�e�N�X�`���Ƃ��ėp����摜	
 * 
 * @returns �ǂݍ��ݐ������ǂ���
 */
void Texture::updateFromMat(GLenum TextureTarget, const cv::Mat& image)
{
	m_textureTarget = TextureTarget;
	m_image = image.clone();

	if(m_image.channels() == 1)
		cv::cvtColor(m_image, m_image, CV_GRAY2BGR);	// 3�`�����l���ɕϊ�

	if(m_image.channels() == 4)
		cv::cvtColor(m_image, m_image, CV_BGRA2BGR);	// 3�`�����l���ɕϊ�

    glBindTexture(m_textureTarget, m_textureObj);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(m_textureTarget, 0, GL_RGB, m_image.size().width, m_image.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, m_image.data);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(m_textureTarget, 0);
}



/**
 * @brief   �e�N�X�`���̑Ή��t��
 * 
 * @param   TextureUnit[in]	�e�N�X�`��ID	
 */
void Texture::bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
