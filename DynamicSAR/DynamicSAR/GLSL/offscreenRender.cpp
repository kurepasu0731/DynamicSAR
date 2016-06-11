#include "offscreenRender.h"


/**
 * @brief   初期処理
 * 
 * @param   width[in]		画像の幅
 * @param   height[in]		画像の高さ	
 * @param   MSAA[in]		マルチサンプルアンチエイリアシングするかどうか
 * @param   sanmple[in]		マルチサンプルアンチエイリアシングする場合のサンプリング量
 */
void OffscreenRender::init(int width, int height, bool MSAA, int sanmple)
{
	offscreen_width = width;
	offscreen_height = height;

	// MSAAを使う場合
	if (MSAA)
	{
		MSAA_flag = true;

		// レンダーバッファ(RGB)の作成
		glGenRenderbuffers( 1, &rboMSAAID[0] );
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[0] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_RGB, width, height);	// メモリの確保
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// レンダーバッファ(depth)の作成
		glGenRenderbuffers( 1, &rboMSAAID[1] );
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[1] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_DEPTH24_STENCIL8, width, height);	// メモリの確保
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// フレームバッファの作成
		glGenFramebuffers( 1, &fboMSAAID );
		glBindFramebuffer( GL_FRAMEBUFFER, fboMSAAID );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboMSAAID[0] );	// レンダーバッファと対応付ける
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboMSAAID[1] );	// レンダーバッファと対応付ける
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	// レンダーバッファ(RGB)の作成
	glGenRenderbuffers( 1, &rboID[0] );
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[0] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_RGB, width, height);		// メモリの確保
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// レンダーバッファ(depth)の作成
	glGenRenderbuffers( 1, &rboID[1] );
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[1] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);		// メモリの確保
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// テクスチャバッファ(RGB)の作成
	glGenTextures( 1, &texID[0]);
	glBindTexture( GL_TEXTURE_2D, texID[0]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	// テクスチャバッファ(depth)の作成
	glGenTextures( 1, &texID[1]);
	glBindTexture( GL_TEXTURE_2D, texID[1]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);


	// フレームバッファの作成
	glGenFramebuffers( 1, &fboID );
	glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboID[0] );	// レンダーバッファと対応付ける
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID[1] );	// レンダーバッファと対応付ける
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID[0], 0);	// テクスチャバッファと対応付ける
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texID[1], 0);	// テクスチャバッファと対応付ける
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );


	// FBOができているかのチェック
    if(checkFramebufferStatus() == false ){
        exit(0);
    }
}


/**
 * @brief   レンダリングサイズの変更
 * 
 * @param   width[in]		画像の幅
 * @param   height[in]		画像の高さ	
 * @param   MSAA[in]		マルチサンプルアンチエイリアシングするかどうか
 * @param   sanmple[in]		マルチサンプルアンチエイリアシングする場合のサンプリング量
 */
void OffscreenRender::changeRenderSize(int width, int height, bool MSAA, int sanmple)
{
	offscreen_width = width;
	offscreen_height = height;

	// MSAAを使う場合
	if (MSAA)
	{
		MSAA_flag = true;

		// レンダーバッファ(RGB)の作成
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[0] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_RGB, width, height);	// メモリの確保
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// レンダーバッファ(depth)の作成
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[1] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_DEPTH24_STENCIL8, width, height);	// メモリの確保
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// フレームバッファの作成
		glBindFramebuffer( GL_FRAMEBUFFER, fboMSAAID );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboMSAAID[0] );	// レンダーバッファと対応付ける
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboMSAAID[1] );	// レンダーバッファと対応付ける
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	// レンダーバッファ(RGB)の作成
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[0] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_RGB, width, height);		// メモリの確保
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// レンダーバッファ(depth)の作成
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[1] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);		// メモリの確保
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// テクスチャバッファ(RGB)の作成
	glBindTexture( GL_TEXTURE_2D, texID[0]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	// テクスチャバッファ(depth)の作成
	glBindTexture( GL_TEXTURE_2D, texID[1]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	// フレームバッファの作成
	glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboID[0] );	// レンダーバッファと対応付ける
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID[1] );	// レンダーバッファと対応付ける
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID[0], 0);	// テクスチャバッファと対応付ける
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texID[1], 0);	// テクスチャバッファと対応付ける
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


/**
 * @brief   レンダリング結果の保存(RGB)
 * 
 * @param   fileName[in]		保存する画像のファイル名
 */
void OffscreenRender::saveRenderRGB(const std::string& fileName)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_8UC3, cv::Scalar(0,0,0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGB, GL_UNSIGNED_BYTE, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// 画像の反転(x軸周り)

	cv::cvtColor(cvmtx, cvmtx, CV_RGB2BGR);		// 色変換

	// 画像の保存
	cv::imwrite(fileName, cvmtx);
}


/**
 * @brief   レンダリング結果の保存(RGB)(中央領域をクリッピング)
 * 
 * @param   fileName[in]		保存する画像のファイル名
 * @param   width[in]			クリッピングする幅
 * @param   height[in]			クリッピングする高さ
 */
void OffscreenRender::saveRenderRGB_Clip(const std::string& fileName, int width, int height)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_8UC3, cv::Scalar(0,0,0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGB, GL_UNSIGNED_BYTE, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// 画像の反転(x軸周り)

	cv::cvtColor(cvmtx, cvmtx, CV_RGB2BGR);		// 色変換

	if (width < cvmtx.cols && height < cvmtx.rows) {
		// クリッピング
		cv::Mat clip(cvmtx, cv::Rect((cvmtx.cols-width)/2, (cvmtx.rows-height)/2, width, height));
		cv::imwrite(fileName, clip);

	} else {
		// 画像の保存
		cv::imwrite(fileName, cvmtx);
	}
}


/**
 * @brief   レンダリング結果の保存(depth)
 * 
 * @param   fileName[in]		保存する画像のファイル名
 */
void OffscreenRender::saveRenderDepth(const std::string& fileName)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// 画像の反転(x軸周り)

	cvmtx.convertTo(cvmtx, CV_8UC1, 255);		// 255段階に変換

	// 画像の保存
	cv::imwrite(fileName, cvmtx);
}


/**
 * @brief   レンダリング結果の保存(depthのxml)
 * 
 * @param   fileName[in]		保存するファイル名(xml)
 * @param   near[in]			Nearクリップ面
 * @param   far[in]				Farクリップ面
 *
 * @note	リアルの深度値を格納(x,y:ピクセル,z:深度値(m))
 */
void OffscreenRender::saveRenderDepthXML(const std::string& fileName, float near, float far)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// 画像の反転(x軸周り)

	// リアルの深度値に変換
	for(int i = 0; i < cvmtx.total(); ++i)
	{
		cvmtx.at<float>(i) = near / (1.0f - cvmtx.at<float>(i) * (1.0f - near / far));

		// farより以上の場合は無効値(-1)とする
		if( cvmtx.at<float>(i) >= far)
			cvmtx.at<float>(i) = -1.0f;
	}

	// xmlファイルで保存
	cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
	cv::write(cvfs, "depthMat", cvmtx);
}


/**
 * @brief   レンダリング結果の保存(depthのxml)(中央領域をクリッピング)
 * 
 * @param   fileName[in]		保存するファイル名(xml)
 * @param   near[in]			Nearクリップ面
 * @param   far[in]				Farクリップ面
 * @param   width[in]			クリッピングする幅
 * @param   height[in]			クリッピングする高さ
 *
 * @note	リアルの深度値を格納(x,y:ピクセル,z:深度値(m))
 */
void OffscreenRender::saveRenderDepthXML_Clip(const std::string& fileName, float near, float far, int width, int height)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// 画像の反転(x軸周り)

	// リアルの深度値に変換
	for(int i = 0; i < cvmtx.total(); ++i)
	{
		cvmtx.at<float>(i) = near / (1.0f - cvmtx.at<float>(i) * (1.0f - near / far));

		// farより以上の場合は無効値(-1)とする
		if( cvmtx.at<float>(i) >= far)
			cvmtx.at<float>(i) = -1.0f;
	}

	
	if (width < cvmtx.cols && height < cvmtx.rows) {
		// クリッピング
		cv::Mat clip(cvmtx, cv::Rect((cvmtx.cols-width)/2, (cvmtx.rows-height)/2, width, height));
		
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "depthMat", clip);

	} else {
		// xmlファイルで保存
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "depthMat", cvmtx);
	}
}


/**
 * @brief   レンダリング結果の保存(Pointのxml)
 * 
 * @param   fileName[in]				保存するファイル名(xml)
 * @param   viewMatrix[in,out]			モデルビュー行列
 * @param   projectionMatrix[in,out]	透視投影行列
 *
 * @note	3次元座標を格納(x,y,z:3次元空間の座標(m))
 */
void OffscreenRender::saveRenderPointCloud(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);


	// ワールド座標の点
	cv::Mat points(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0));

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	for(int y = 0; y < cvmtx.rows; ++y)
	{
		for(int x = 0; x < cvmtx.cols; ++x)
		{
			// 深度画像
			glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

			// 2.5次元に変換
			glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

			if(cvmtx.at<float>(y,x) < 1.0)
			{
				points.at<cv::Vec3f>(y,x)[0] = pos.x;
				points.at<cv::Vec3f>(y,x)[1] = pos.y;
				points.at<cv::Vec3f>(y,x)[2] = pos.z;
			}
			else
			{
				points.at<cv::Vec3f>(y,x)[0] = FLT_MAX;
				points.at<cv::Vec3f>(y,x)[1] = FLT_MAX;
				points.at<cv::Vec3f>(y,x)[2] = FLT_MAX;
			}
		}
	}

	// xmlファイルで保存
	cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
	cv::write(cvfs, "pointMat", points);
}


/**
 * @brief   レンダリング結果の保存(Pointのxml)(中央領域をクリッピング)
 * 
 * @param   fileName[in]				保存するファイル名(xml)
 * @param   viewMatrix[in,out]			モデルビュー行列
 * @param   projectionMatrix[in,out]	透視投影行列
 * @param   width[in]					クリッピングする幅
 * @param   height[in]					クリッピングする高さ
 *
 * @note	3次元座標を格納(x,y,z:3次元空間の座標(m))
 */
void OffscreenRender::saveRenderPointCloud_Clip(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, int width, int height)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);


	if(width < cvmtx.cols && height < cvmtx.rows)
	{
		// ワールド座標の点
		cv::Mat points(cv::Size(width, height), CV_32FC3, cv::Scalar(0));

		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );

		int count_x = 0;
		int count_y = 0;
		int start_x = (cvmtx.cols-width)/2;
		int start_y = (cvmtx.rows-height)/2;

		for(int y = start_y; y < start_y+height; ++y)
		{
			count_x = 0;
			for(int x = start_x; x < start_x+width; ++x)
			{
				// 深度画像
				glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

				// 2.5次元に変換
				glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

				if(cvmtx.at<float>(y,x) < 1.0)
				{
					points.at<cv::Vec3f>(count_y,count_x)[0] = pos.x;
					points.at<cv::Vec3f>(count_y,count_x)[1] = pos.y;
					points.at<cv::Vec3f>(count_y,count_x)[2] = pos.z;
				}
				else
				{
					points.at<cv::Vec3f>(count_y,count_x)[0] = FLT_MAX;
					points.at<cv::Vec3f>(count_y,count_x)[1] = FLT_MAX;
					points.at<cv::Vec3f>(count_y,count_x)[2] = FLT_MAX;
				}
				count_x++;
			}
			count_y++;
		}
		// xmlファイルで保存
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "pointMat", points);
	}
	else
	{	
		// ワールド座標の点
		cv::Mat points(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0));

		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );

		for(int y = 0; y < cvmtx.rows; ++y)
		{
			for(int x = 0; x < cvmtx.cols; ++x)
			{
				// 深度画像
				glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

				// 2.5次元に変換
				glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

				if(cvmtx.at<float>(y,x) < 1.0)
				{
					points.at<cv::Vec3f>(y,x)[0] = pos.x;
					points.at<cv::Vec3f>(y,x)[1] = pos.y;
					points.at<cv::Vec3f>(y,x)[2] = pos.z;
				}
				else
				{
					points.at<cv::Vec3f>(y,x)[0] = FLT_MAX;
					points.at<cv::Vec3f>(y,x)[1] = FLT_MAX;
					points.at<cv::Vec3f>(y,x)[2] = FLT_MAX;
				}
			}
		}

		// xmlファイルで保存
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "pointMat", points);
	}
}


/**
 * @brief   レンダリング結果をMat型で返す(RGB)
 * 
 * @param   colorMat[in]		レンダリングした画像
 */
void OffscreenRender::getRenderRGB(cv::Mat& colorMat)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	colorMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_8UC4, cv::Scalar(0,0,0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)colorMat.data);

	cv::cvtColor(colorMat, colorMat, CV_RGBA2BGR);		// 色変換
}



/**
 * @brief   レンダリング結果をMat型で返す(depth)
 * 
 * @param   colorMat[in]		レンダリングした画像
 */
void OffscreenRender::getRenderDepth(cv::Mat& depthMat)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	depthMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)depthMat.data);
}


/**
 * @brief   レンダリング結果をMat型で返す(Normal)
 * 
 * @param   normalMat[in]		レンダリングした画像
 */
void OffscreenRender::getRenderNormal(cv::Mat& normalMat)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	normalMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0,0,0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGB, GL_FLOAT, (void*)normalMat.data);

	// 0～1の範囲を-1～1へ変換
	float * ptr = (float *)normalMat.data;		// ポインタ
	const int length = static_cast<const int>(normalMat.step1());	// 1ステップのチャンネル数
	const int length_x = normalMat.cols * 3;						// 横のサイズ

	for (int y = 0; y < normalMat.rows; ++y)
	{
		for (int x = 0; x < length_x; x+=3)
		{
			ptr[x+0] = ptr[x+0] * 2.f - 1.f;
			ptr[x+1] = ptr[x+1] * 2.f - 1.f;
			ptr[x+2] = ptr[x+2] * 2.f - 1.f;
		}
		ptr += length;
	}
}


/**
 * @brief   レンダリング結果をMat型で返す(PointCloud)
 * 
 * @param   colorMat[in]				レンダリングした画像
 * @param   viewMatrix[in,out]			モデルビュー行列
 * @param   projectionMatrix[in,out]	透視投影行列
 */
void OffscreenRender::getRenderPointCloud(cv::Mat& pointcloudMat, glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
	// MSAAを使う場合
	if(MSAA_flag)
	{
		// MSAA用のフレームバッファから通常のフレームバッファへ送る
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// 読み込むTextureを指定
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//黒で初期化

	// Textureから読み込み
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	// ワールド座標の点
	pointcloudMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0));

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	for(int y = 0; y < pointcloudMat.rows; ++y)
	{
		for(int x = 0; x < pointcloudMat.cols; ++x)
		{
			// 深度画像
			glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

			// 2.5次元に変換
			glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

			if(cvmtx.at<float>(y,x) < 1.0)
			{
				pointcloudMat.at<cv::Vec3f>(y,x)[0] = pos.x;
				pointcloudMat.at<cv::Vec3f>(y,x)[1] = pos.y;
				pointcloudMat.at<cv::Vec3f>(y,x)[2] = pos.z;
			}
			else
			{
				pointcloudMat.at<cv::Vec3f>(y,x)[0] = FLT_MAX;
				pointcloudMat.at<cv::Vec3f>(y,x)[1] = FLT_MAX;
				pointcloudMat.at<cv::Vec3f>(y,x)[2] = FLT_MAX;
			}
		}
	}
}




/**
 * @brief   フレームバッファの取得状況の確認	
 * 
 * @returns 成功したかどうか
 */
bool OffscreenRender::checkFramebufferStatus()
{
	// check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
                std::cout << "Framebuffer complete." << std::endl;
                return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
                std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
                std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
                return false;

        case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
                return false;

        default:
                std::cout << "[ERROR] Unknow error." << std::endl;
                return false;
    }
}