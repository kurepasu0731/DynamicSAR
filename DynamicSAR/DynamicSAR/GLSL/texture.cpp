#include <iostream>
#include "texture.h"




/**
 * @brief   テクスチャの読み込み
 *
 * @returns 読み込み成功かどうか
 */
bool Texture::load()
{
	// OpenCVによる画像読み込み
	cv::Ptr<IplImage> iplimg = cvLoadImage(m_fileName.c_str(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cv::Mat fileImage = cv::cvarrToMat(iplimg);
    m_image = fileImage.clone();
   
    if (m_image.empty()) {
        std::cout << "Error loading texture '" << m_fileName  << std::endl;
        return false;
    }

	if(m_image.channels() == 3)
		cv::cvtColor(m_image, m_image, CV_BGR2BGRA);	// 4チャンネルに変換

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
 * @brief   テクスチャの読み込み
 * 
 * @param   TextureTarget[in]	テクスチャのタイプ
 * @param   FileName[in]		テクスチャのファイル名	
 * 
 * @returns 読み込み成功かどうか
 */
bool Texture::loadImage(GLenum TextureTarget, const std::string& FileName)
{
	m_textureTarget = TextureTarget;
    m_fileName      = FileName;

	// OpenCVによる画像読み込み
	cv::Ptr<IplImage> iplimg = cvLoadImage(m_fileName.c_str(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cv::Mat fileImage = cv::cvarrToMat(iplimg);
    m_image = fileImage.clone();
   
    if (m_image.empty()) {
        std::cout << "Error loading texture '" << m_fileName  << std::endl;
        return false;
    }

	if(m_image.channels() == 4)
		cv::cvtColor(m_image, m_image, CV_BGRA2BGR);	// 3チャンネルに変換

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
 * @brief   cv::Matをテクスチャとして用いる
 * 
 * @param   TextureTarget[in]	テクスチャのタイプ
 * @param   image[in]			テクスチャとして用いる画像	
 * 
 * @returns 読み込み成功かどうか
 */
void Texture::loadFromMat(GLenum TextureTarget, const cv::Mat& image)
{
	m_textureTarget = TextureTarget;
	m_image = image.clone();

	if(m_image.channels() == 1)
		cv::cvtColor(m_image, m_image, CV_GRAY2BGR);	// 3チャンネルに変換

	if(m_image.channels() == 4)
		cv::cvtColor(m_image, m_image, CV_BGRA2BGR);	// 3チャンネルに変換

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
 * @brief   cv::Matを更新
 * 
 * @param   TextureTarget[in]	テクスチャのタイプ
 * @param   image[in]			テクスチャとして用いる画像	
 * 
 * @returns 読み込み成功かどうか
 */
void Texture::updateFromMat(GLenum TextureTarget, const cv::Mat& image)
{
	m_textureTarget = TextureTarget;
	m_image = image.clone();

	if(m_image.channels() == 1)
		cv::cvtColor(m_image, m_image, CV_GRAY2BGR);	// 3チャンネルに変換

	if(m_image.channels() == 4)
		cv::cvtColor(m_image, m_image, CV_BGRA2BGR);	// 3チャンネルに変換

    glBindTexture(m_textureTarget, m_textureObj);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(m_textureTarget, 0, GL_RGB, m_image.size().width, m_image.size().height, 0, GL_BGR, GL_UNSIGNED_BYTE, m_image.data);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glBindTexture(m_textureTarget, 0);
}



/**
 * @brief   テクスチャの対応付け
 * 
 * @param   TextureUnit[in]	テクスチャID	
 */
void Texture::bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
