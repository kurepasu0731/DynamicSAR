#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>

#include <GL/glew.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


/**
 * @brief   テクスチャの設定クラス
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

    bool load();					// テクスチャの読み込み

	bool loadImage(GLenum TextureTarget, const std::string& FileName);		// テクスチャの読み込み

	void loadFromMat(GLenum TextureTarget, const cv::Mat& image);			// cv::Matをテクスチャとして用いる
	void updateFromMat(GLenum TextureTarget, const cv::Mat& image);			// cv::Matを更新

    void bind(GLenum TextureUnit);	// テクスチャの対応付け


    std::string m_fileName;			// ファイル名
    GLenum m_textureTarget;			// テクスチャのタイプ
    GLuint m_textureObj;			// テクスチャの名前
    cv::Mat m_image;				// テクスチャのデータ
	int width;						// テクスチャの幅
	int height;						// テクスチャの高さ
};


#endif