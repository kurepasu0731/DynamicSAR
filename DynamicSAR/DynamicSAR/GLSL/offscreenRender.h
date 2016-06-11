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
 * @brief   オフスクリーンレンダリングするクラス
 * 
 * @note	RGB,Depthでのレンダリングに対応	
 *			マルチサンプルアンチエイリアシングに対応
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

	void init(int width, int height, bool MSAA=false, int sanmple=16);					// 初期処理

	inline void startRender()							// オフスクリーンレンダリング開始
	{
		// 描画先の切り替え
		if(MSAA_flag)
			glBindFramebuffer(GL_FRAMEBUFFER, fboMSAAID);
		else
			glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	}

	inline void endRender()								// オフスクリーンレンダリング終了
	{
		// 描画先をデフォルトに
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// レンダリングサイズの変更
	void changeRenderSize(int width, int height, bool MSAA=false, int sanmple=16);

	// オフスクリーンレンダリングの結果を保存
	void saveRenderRGB(const std::string& fileName);									// レンダリング結果の保存(RGB)
	void saveRenderRGB_Clip(const std::string& fileName, int width, int height);		// レンダリング結果の保存(RGB)(中央領域をクリッピング)
	void saveRenderDepth(const std::string& fileName);									// レンダリング結果の保存(depth)
	void saveRenderDepthXML(const std::string& fileName, float near, float far);		// レンダリング結果の保存(depthのxml)
	void saveRenderDepthXML_Clip(const std::string& fileName, float near, float far, int width, int height);	// レンダリング結果の保存(depthのxml)中央領域をクリッピング)
	void saveRenderPointCloud(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix);	// レンダリング結果の保存(Pointのxml)
	void saveRenderPointCloud_Clip(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, int width, int height);	// レンダリング結果の保存(Pointのxml)(中央領域をクリッピング)

	// オフスクリーンレンダリングの結果を返す
	void getRenderRGB(cv::Mat& colorMat);
	void getRenderDepth(cv::Mat& depthMat);
	void getRenderNormal(cv::Mat& normalMat);
	void getRenderPointCloud(cv::Mat& pointcloudMat, glm::mat4& viewMatrix, glm::mat4& projectionMatrix);

	bool checkFramebufferStatus();						// フレームバッファの取得状況の確認


	GLuint fboID;					// フレームバッファ
	GLuint rboID[2];				// レンダーバッファ
	GLuint texID[2];				// テクスチャバッファ
	GLuint fboMSAAID;				// フレームバッファ(マルチサンプルアンチエイリアシング)
	GLuint rboMSAAID[2];			// レンダーバッファ(マルチサンプルアンチエイリアシング)
	bool MSAA_flag;					// マルチサンプルアンチエイリアシングを使うかどうか
	int offscreen_width;			// オフスクリーンの横幅
	int offscreen_height;			// オフスクリーンの縦幅
};

#endif