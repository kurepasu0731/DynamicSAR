#include "Tracking_thread.h"


void TrackingThread::init(const std::string& modelFile, const std::string& trackingFile, const cv::Matx33f& _camMat, const glm::mat4& _cameraMat)
{
	// 背景色
	glClearColor(1.f, 1.f, 1.f, 1.f);

	// デプステスト
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// カリング
	glEnable(GL_CULL_FACE);

	// ポイントスプライトの設定
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	// オフスクリーンレンダリングの初期化
	offscreen1.init(offscreen1.offscreen_width, offscreen1.offscreen_height);
	offscreen2.init(offscreen2.offscreen_width, offscreen2.offscreen_height);
	offscreen3.init(offscreen3.offscreen_width, offscreen3.offscreen_height);
	offscreen4.init(offscreen4.offscreen_width, offscreen4.offscreen_height);

	
	// 内部パラメータの格納
	cameraMat = _cameraMat;
	camMat = _camMat;


	///// 元のモデル用 /////

	// プログラムオブジェクトを作成する
	init_GLSL( &program_orig, "../../Shader/shading.vert", "../../Shader/shading.frag");
	glUseProgram(program_orig);

	// uniform変数の場所を取得
	MatrixID = glGetUniformLocation(program_orig, "MVP");
	ViewMatrixID = glGetUniformLocation(program_orig, "V");
	ModelMatrixID = glGetUniformLocation(program_orig, "M");

	// 光源位置の取得
	LightID = glGetUniformLocation(program_orig, "LightPosition_worldspace");
	LightPowerID = glGetUniformLocation(program_orig, "LightPower");

	// モデルの読み込み
	orig_mesh.loadMesh(modelFile);


	///// CADモデルエッジ検出用 /////
	init_GLSL( &program_CADedge, "../../Shader/diffgauss.vert", "../../Shader/diffgauss.frag");
	glUseProgram(program_CADedge);

	// シェーダのuniform変数の位置を取得
	edge_texID = glGetUniformLocation(program_CADedge, "inputImage");
	edge_MatrixID = glGetUniformLocation(program_CADedge, "MVP");
	edge_locW = glGetUniformLocation(program_CADedge,"imageWidth");
	edge_locH = glGetUniformLocation(program_CADedge,"imageHeight");

	// uniform変数に値を渡す
	glUniform1i(edge_locW, windowWidth);
	glUniform1i(edge_locH, windowHeight); 

	// メッシュの読み込み
	edge_rect.init();
	edge_rect.resizeRectangle( windowWidth, windowHeight);


	///// トラッキングモデル用 /////

	//	プログラムオブジェクトを作成する
	init_GLSL( &program_point, "../../Shader/edge.vert", "../../Shader/edge.frag");
	glUseProgram(program_point);

	// uniform変数の場所を取得
	pointMatrixID = glGetUniformLocation(program_point, "MVP");
	pointViewMatrixID = glGetUniformLocation(program_point, "V");
	pointModelMatrixID = glGetUniformLocation(program_point, "M");
	
	// 点の大きさ
	GLuint pointSizeID = glGetUniformLocation(program_point, "point_size");
	GLfloat p_size = 0.5f;
	glUniform1f(pointSizeID, p_size);

	// トラッキング用のファイルの読み込み
	pointCloud.clear();
	pointNormal.clear();
	loadPointCloudPly(trackingFile, pointCloud, pointNormal);

	// インデックスは入力配列の順番をそのまま用いる
	std::vector<unsigned int> pointIndex(pointCloud.size());
	std::vector<glm::vec3> pointIndexF(pointCloud.size());		// 扱えるインデックスは256*256*256まで

	// レンダーバッファのRGBをインデックスに割り当てる
	for (int i = 0; i < pointCloud.size(); ++i){
		pointIndex[i] = i;
		int i1 = i / (256*256);
		int i2 = i / 256;
		int i3 = i % 256;
		pointIndexF[i] = glm::vec3((float)i1/255.f,(float)i2/255.f,(float)i3/255.f);
	}

	// モデルの読み込み
	// VBOの生成
    glGenBuffers(3, pointVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*pointCloud.size(), &pointCloud[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*pointNormal.size(), &pointNormal[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*pointIndexF.size(), &pointIndexF[0], GL_STATIC_DRAW);

	// IBOの生成
    glGenBuffers(1, &pointIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pointIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*pointIndex.size(), &pointIndex[0], GL_STATIC_DRAW);

	//　バインドしたものをもどす
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	///// シェーダエフェクト用 /////

	//	プログラムオブジェクトを作成する
	init_GLSL( &program_image, "../../Shader/image_rect.vert", "../../Shader/image_rect.frag");
	glUseProgram(program_image);

	// シェーダのuniform変数の位置を取得
	tex0ID = glGetUniformLocation(program_image, "inputImage0");
	tex1ID = glGetUniformLocation(program_image, "inputImage1");
	tex2ID = glGetUniformLocation(program_image, "inputImage2");
	tex3ID = glGetUniformLocation(program_image, "inputImage3");
	imageMatrixID = glGetUniformLocation(program_image, "MVP");
	locW = glGetUniformLocation(program_image,"imageWidth");
	locH = glGetUniformLocation(program_image,"imageHeight");

	// uniform変数に値を渡す
	glUniform1i(locW, windowWidth);
	glUniform1i(locH, windowHeight); 

	// メッシュの読み込み
	image_rect.init();
	image_rect.resizeRectangle( windowWidth, windowHeight);

	///// 入力画像用 /////

	// 背景メッシュの生成
	input_mesh.init(inputImg, "../../Shader/background.vert", "../../Shader/background.frag", windowWidth, windowHeight);


	///// エッジ描画用 /////
	//	プログラムオブジェクトを作成する
	init_GLSL( &program_edge, "../../Shader/point.vert", "../../Shader/point.frag");
	glUseProgram(program_edge);

	// uniform変数の場所を取得
	pMatrixID = glGetUniformLocation(program_edge, "MVP");
	
	// 点の大きさ
	glUseProgram(program_edge);
	GLuint psizeID = glGetUniformLocation(program_edge, "point_size");
	GLfloat psize = 0.5f;
	glUniform1f(psizeID, psize);

	// VBOの生成
    glGenBuffers(1, &pVBO);
    glGenBuffers(1, &pIBO);
}


// 毎フレーム行う処理
void TrackingThread::display()
{
	// パラメータの取得
	critical_section->getTrackingParams(find_distance, error_th, edge_th1, edge_th2, trackingTime, delayTime);

	///// トラッキング /////
	tracking();

	///// シーンの描画 /////

	// ウィンドウを消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, windowWidth, windowHeight);	
	// 背景色
	glClearColor(1.f, 1.f, 1.f, 1.f);

	if (pointEdge.size() > 0) {
		//edgeTracker->scene_orig(windowWidth, windowHeight);
		draw_point(pointEdge, windowWidth, windowHeight);
	}

	// 背景
	scene_background(windowWidth, windowHeight);

	// カラーバッファを入れ替え,イベントを取得
	swapBuffers();
}


// モデルベースのトラッキング
void TrackingThread::tracking()
{
	// 入力画像の更新
	cv::Mat cap;
	irCamDev->captureImage(cap);

	// 歪み補正
	if(critical_section->use_calib_flag)
	{
		cv::Mat map1, map2;
		critical_section->getUndistortMap(map1, map2);
		cv::remap(cap, cap, map1, map2, cv::INTER_LINEAR);
	}

	// 初期状態であれば
	if (!critical_section->tracking_flag) {
		setInitPose();
	}
	
	/*double start, finish, time;
	start = static_cast<double>(cv::getTickCount());*/

	for (int i = 0; i < 1; ++i)
	{
		inputImg = cap.clone();
		
		// オフスクリーンで3Dモデルの3次元点および投影後の2次元点の取得
		offscreen_render(windowWidth, windowHeight);
		
		// 3Dモデルの輪郭線の抽出
		extractModelEdge();

		// 入力画像のエッジの抽出
		extractInputEdge();

		// 入力画像との対応を求める
		estimateCorrespondence();

		// 姿勢の更新
		if (critical_section->tracking_flag) {	

			// IRLSによる姿勢推定
			estimatePoseIRLS();
		}
	}

	/*finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );
	std::cout << "処理時間 : " << time*1000 << "ms" << std::endl;*/

	// エラー率の計算
	calcError();
}


// オフスクリーンでの描画
void TrackingThread::offscreen_render(int width, int height)
{
	///// 1パス目 /////
	offscreen1.startRender();

	// ウィンドウを消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// 背景色

	// 元のモデルの描画
	scene_orig(width, height);

	offscreen1.endRender();


	///// 2パス目 /////
	offscreen2.startRender();

	// ウィンドウを消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// 背景色

	glUseProgram(program_CADedge);

	// 平行投影
	glm::mat4 OrthoMatrix0 = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);

	// Uniform変数に行列を送る
	glUniformMatrix4fv(edge_MatrixID, 1, GL_FALSE, &OrthoMatrix0[0][0]);
	glUniform1i(edge_texID, 0);
	glUniform1i(edge_locW, width);
	glUniform1i(edge_locH, height);

	// 平面
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, offscreen1.texID[0]);		// 元モデルの画像

	// エッジ描画
	edge_rect.draw();

	glBindTexture(GL_TEXTURE_2D, 0);

	offscreen2.endRender();


	///// 3パス目 /////
	offscreen3.startRender();

	// ウィンドウを消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// 背景色

	// トラッキングモデルの描画
	scene_point(width, height);

	offscreen3.endRender();


	///// 4パス目 /////
	offscreen4.startRender();

	// ウィンドウを消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// 背景色

	glUseProgram(program_image);

	// 平行投影
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);

	// Uniform変数に行列を送る
	glUniformMatrix4fv(imageMatrixID, 1, GL_FALSE, &OrthoMatrix[0][0]);
	glUniform1i(tex0ID, 0);
	glUniform1i(tex1ID, 1);
	glUniform1i(tex2ID, 2);
	glUniform1i(tex3ID, 3);
	glUniform1i(locW, width);
	glUniform1i(locH, height);

	// 平面
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, offscreen1.texID[1]);		// 元モデルのデプス
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, offscreen2.texID[0]);		// エッジ画像
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, offscreen3.texID[0]);		// トラッキングモデルの頂点インデックス
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, offscreen3.texID[1]);		// トラッキングモデルのデプス

	image_rect.draw();

	glBindTexture(GL_TEXTURE_2D, 0);

	// エッジ点のレンダリング	
	offscreen4.getRenderRGB(indexImg);

	offscreen4.endRender();
}


// 3Dモデルの輪郭線の検出
void TrackingThread::extractModelEdge()
{
	// 初期化
	pointEdge.clear();
	correspondPoints.clear();

	// 3次元点と投影後の2次元点の取得
	for (int r = 0; r < indexImg.rows; ++r) 
	{
		cv::Vec3b* index_p = indexImg.ptr<cv::Vec3b>(r);
		for (int c = 0; c < indexImg.cols; ++c) 
		{
			int B = index_p[c][0];
			int G = index_p[c][1];
			int R = index_p[c][2];

			// 白以外のピクセルを取り出す
			if ( R != 255 && G != 255 && B != 255) {
				int index = R*256*256 + G*256 + B;
				Correspond model_point;

				// 3次元座標値
				model_point.model3D = cv::Point3f(pointCloud[index].x, pointCloud[index].y, pointCloud[index].z);

				// 2次元への投影
				cv::Vec3f normal3d = cv::Point3f(pointNormal[index].x, pointNormal[index].y, pointNormal[index].z);
				cv::Vec2f point2d;
				cv::Vec2f normal2d;
				projection3Dto2D( model_point.model3D, normal3d, point2d, normal2d);

				model_point.model2D = point2d;
				model_point.normal2D = normal2d;

				// 投影点が範囲内であれば追加
				if (point2d[0] >= 0 && point2d[0] < windowWidth && point2d[1] >= 0 && point2d[1] < windowHeight &&
					normal2d[0] != std::numeric_limits<float>::infinity() && normal2d[0] != -std::numeric_limits<float>::infinity()) 
				{
					correspondPoints.emplace_back (model_point);
					pointEdge.emplace_back(pointCloud[index]);
				}
			}
		}

	}
}


// 入力画像のエッジ検出
void TrackingThread::extractInputEdge()
{
	// グレイスケール
	cv::Mat grayImg;
	if (inputImg.channels() == 3){
		cv::cvtColor(inputImg, grayImg, CV_RGB2GRAY);
	}else {
		grayImg = inputImg.clone();
	}

	// ガウシアンフィルタ
	//cv::GaussianBlur(grayImg, grayImg, cv::Size(3,3), 3, 3);

	// エッジ検出
	cv::Canny (grayImg, cannyImg, edge_th1, edge_th2);

	// Sobelフィルタ
	cv::Sobel(grayImg, sobelImgX, CV_32F, 1, 0, 3, 1.0, 0.0, cv::BORDER_REPLICATE);
	cv::Sobel(grayImg, sobelImgY, CV_32F, 0, 1, 3, 1.0, 0.0, cv::BORDER_REPLICATE);

	//cv::imshow("j", cannyImg);
}


// 入力画像との対応を推定
void TrackingThread::estimateCorrespondence()
{
	edge_img = cannyImg.clone();
	cv::cvtColor(edge_img, edge_img, CV_GRAY2BGR);

	// cmからピクセル単位への変換
	max_distance = std::abs(camMat(0,0) * find_distance / estimatedPose(2,3));

	// 3Dモデルのエッジ点との対応点の探索
	for (int i = 0; i < static_cast<int>(correspondPoints.size()); ++i)
	{
		float model_normal_x = correspondPoints[i].normal2D.x;
		float model_normal_y = correspondPoints[i].normal2D.y;

		// 探索方向(3Dモデルのエッジの法線方向)
		float find_vec_rad = std::atan2f(model_normal_y, model_normal_x);
		float dx = cos(find_vec_rad);
		float dy = sin(find_vec_rad);

		// 3Dモデルの2次元点の位置
		int x = static_cast<int>(correspondPoints[i].model2D.x);
		int y = static_cast<int>(correspondPoints[i].model2D.y);

		correspondPoints[i].normalUnit = cv::Point2f(dx, dy);
		correspondPoints[i].dist = max_distance;

		// 法線方向を探索
		for (int j = 0; j < static_cast<int>(max_distance); ++j)
		{
			///// 正方向への探索 /////
			float point_x = correspondPoints[i].model2D.x + dx * (float)j;
			float point_y = correspondPoints[i].model2D.y + dy * (float)j;
			x = static_cast<int>(point_x + 0.5);
			y = static_cast<int>(point_y + 0.5);

			// 範囲内かつ入力画像のエッジ点であれば
			if ( x >= 0 && x < windowWidth && y >= 0 && y < windowHeight && cannyImg.at<uchar>(y,x) > uchar(50) )
			{
				float input_edge_x = sobelImgX.at<float>(y,x);
				float input_edge_y = sobelImgY.at<float>(y,x);

				// 入力画像のエッジの勾配方向と3Dモデルの法線の内積
				float edge_dot = input_edge_x * model_normal_x + input_edge_y * model_normal_y;
				// ノルムの積
				float edge_norm = std::sqrtf((input_edge_x*input_edge_x + input_edge_y*input_edge_y) * (model_normal_x*model_normal_x + model_normal_y*model_normal_y));
				// コサイン類似度
				float cos_theta = edge_dot / edge_norm;

				// 勾配方向が90°以内に収まれば検出
				if ( cos_theta >= 0.7071 || cos_theta <= -0.7071)
				{
					correspondPoints[i].dist = std::sqrt( (double)j*dx*(double)j*dx + (double)j*dy*(double)j*dy );
					correspondPoints[i].imageEdge2D = cv::Point2f(point_x,point_y);
					break;
				}
			}

			// 0の時は以下の処理をスキップ
			if(j == 0) {
				continue;
			}

			///// 負方向への探索 /////
			point_x = correspondPoints[i].model2D.x - dx * (float)j;
			point_y = correspondPoints[i].model2D.y - dy * (float)j;
			x = static_cast<int>(point_x + 0.5);
			y = static_cast<int>(point_y + 0.5);

			// 範囲内かつ入力画像のエッジ点であれば
			if ( x >= 0 && x < windowWidth && y >= 0 && y < windowHeight && cannyImg.at<uchar>(y,x) > uchar(50) )
			{
				float input_edge_x = sobelImgX.at<float>(y,x);
				float input_edge_y = sobelImgY.at<float>(y,x);

				// 入力画像のエッジの勾配方向と3Dモデルの法線の内積
				float edge_dot = input_edge_x * model_normal_x + input_edge_y * model_normal_y;
				// ノルムの積
				float edge_norm = std::sqrtf((input_edge_x*input_edge_x + input_edge_y*input_edge_y) * (model_normal_x*model_normal_x + model_normal_y*model_normal_y));
				// コサイン類似度
				float cos_theta = edge_dot / edge_norm;			

				// 勾配方向が90°以内に収まれば検出
				if ( cos_theta >= 0.7071 || cos_theta <= -0.7071)
				{
					correspondPoints[i].dist = std::sqrt( (double)j*dx*(double)j*dx + (double)j*dy*(double)j*dy );
					correspondPoints[i].imageEdge2D = cv::Point2f(point_x,point_y);

					// 法線の単位ベクトルの反転
					correspondPoints[i].normalUnit = -correspondPoints[i].normalUnit;
					break;
				}
			}
		}

		// 確認用
		cv::circle(edge_img, correspondPoints[i].model2D, 3, cv::Scalar(0,0,200), -1, CV_AA);

		if (correspondPoints[i].dist < max_distance) {
			cv::line(inputImg, correspondPoints[i].model2D, correspondPoints[i].imageEdge2D, cv::Scalar(200,0,0), 2, CV_AA);

			// 確認用
			cv::line(edge_img, correspondPoints[i].model2D, correspondPoints[i].imageEdge2D, cv::Scalar(200,0,0), 2, CV_AA);
		}
	}
}


// ヤコビアンの計算
TooN::Vector<6> TrackingThread::calcJacobian(const cv::Point3f& pts3, const cv::Point2f& pts2, const cv::Point2f& ptsnv, double ptsd, const TooN::SE3& E)
{
	TooN::Vector<4> vpts3;	// 3次元点
	TooN::Vector<3> vpts2;	// 2次元点
	TooN::Vector<2> vptsn;	// 法線の単位ベクトル
	TooN::Vector<6> J;
	TooN::Matrix<2,2> ja_;

	// 初期化
	vpts3 = pts3.x, pts3.y, pts3.z, 1.0;
	vpts2 = pts2.x, pts2.y, 1.0;
	vptsn = ptsnv.x, ptsnv.y;

	ja_[0][0] = -static_cast<double>(camMat(0,0)); ja_[0][1] = 0.0;
	ja_[1][1] = static_cast<double>(camMat(1,1)); ja_[1][0] = 0.0;

	for(int i = 0; i < 6; i++)
	{
		TooN::Vector<4> cam_coord = E * vpts3;
		TooN::Vector<4> temp = E * TooN::SE3::generator_field(i, vpts3);
		TooN::Vector<2> temp2 = temp.slice<0,2>() / cam_coord[2] - cam_coord.slice<0,2>() * (temp[2]/cam_coord[2]/cam_coord[2]);
		J[i] = vptsn*ja_*temp2;
	}

	return J;
}


// IRLSによる姿勢推定
void TrackingThread::estimatePoseIRLS()
{
	double m_prev[4][4];
	for(int r = 0; r < 4; ++r) {
		for(int c = 0; c < 4; ++c) {
			m_prev[r][c] = estimatedPose(r,c);
		}
	}

	TooN::Matrix<4> M(m_prev);
	TooN::SE3 se3_prev;
	se3_prev = M;

	TooN::WLS<6> wls;
	for (int i = 0; i< (int)correspondPoints.size(); ++i)
	{
		if(correspondPoints[i].dist < max_distance)
		{
			wls.add_df( correspondPoints[i].dist,
						calcJacobian(correspondPoints[i].model3D, correspondPoints[i].model2D, correspondPoints[i].normalUnit, correspondPoints[i].dist, se3_prev),
						1/((1.0 + abs(correspondPoints[i].dist))));
		}
	}

	wls.compute();

	TooN::Vector<6> mu = wls.get_mu();
	TooN::Matrix<6> inv_cov = (wls.get_C_inv()) ;

	// 初期化
	cv::Mat cov = cv::Mat::eye(6, 6, CV_32F)*0.1;
	for(int i = 0; i < 6; ++i) {
		for(int j = 0; j < 6; ++j){
			covariance(i,j) = inv_cov(i,j);
		}
	}

	// 逆行列
	covariance = covariance.inv();
	cv::add(covariance, cov, covariance);

	TooN::SE3 se3_cur;
	se3_cur = se3_prev * TooN::SE3::exp(mu);

	TooN::Matrix<3> rot = se3_cur.getRot();
	TooN::Vector<3> trans = se3_cur.getTrans();


	// 回転
	glm::mat4 rotation( rot(0,0), rot(1,0), rot(2,0), 0.0f,
						rot(0,1), rot(1,1), rot(2,1), 0.0f,
						rot(0,2), rot(1,2), rot(2,2), 0.0f,
						0.f, 0.f, 0.f, 1.f);


	///// 予測フィルタ //////
	// 新しい位置姿勢の格納
	timer.stop();
	predict_point->addData(timer.MSec(), cv::Point3f(trans[0],trans[1],trans[2]));
	predict_quat->addData(timer.MSec(), glm::quat_cast(rotation));

	// 投影予測位置
	cv::Point3f predict_point2 = predict_point->calcYValue(timer.MSec()+trackingTime);

	// 遅延補償した位置姿勢
	glm::mat4 predictPose = glm::mat4_cast(predict_quat->calcYValue(timer.MSec()+trackingTime));
	predictPose[3][0] = predict_point2.x;
	predictPose[3][1] = predict_point2.y;
	predictPose[3][2] = predict_point2.z;

	// 予測フィルタ使用時
	if (critical_section->predict_flag && !firstTime)
	{
		// 姿勢の更新
		estimatedPose(0, 0) = predictPose[0][0];
		estimatedPose(0, 1) = predictPose[1][0];
		estimatedPose(0, 2) = predictPose[2][0];
		estimatedPose(0, 3) = predictPose[3][0];
		estimatedPose(1, 0) = predictPose[0][1];
		estimatedPose(1, 1) = predictPose[1][1];
		estimatedPose(1, 2) = predictPose[2][1];
		estimatedPose(1, 3) = predictPose[3][1];
		estimatedPose(2, 0) = predictPose[0][2];
		estimatedPose(2, 1) = predictPose[1][2];
		estimatedPose(2, 2) = predictPose[2][2];
		estimatedPose(2, 3) = predictPose[3][2];
		estimatedPose(3, 0) = 0.0f;
		estimatedPose(3, 1) = 0.0f;
		estimatedPose(3, 2) = 0.0f;
		estimatedPose(3, 3) = 1.0f;
	}
	else
	{
		// 姿勢の更新
		estimatedPose(0, 0) = static_cast<float>(rot(0,0));
		estimatedPose(0, 1) = static_cast<float>(rot(0,1));
		estimatedPose(0, 2) = static_cast<float>(rot(0,2));
		estimatedPose(0, 3) = static_cast<float>(trans[0]);
		estimatedPose(1, 0) = static_cast<float>(rot(1,0));
		estimatedPose(1, 1) = static_cast<float>(rot(1,1));
		estimatedPose(1, 2) = static_cast<float>(rot(1,2));
		estimatedPose(1, 3) = static_cast<float>(trans[1]);
		estimatedPose(2, 0) = static_cast<float>(rot(2,0));
		estimatedPose(2, 1) = static_cast<float>(rot(2,1));
		estimatedPose(2, 2) = static_cast<float>(rot(2,2));
		estimatedPose(2, 3) = static_cast<float>(trans[2]);
		estimatedPose(3, 0) = 0.0f;
		estimatedPose(3, 1) = 0.0f;
		estimatedPose(3, 2) = 0.0f;
		estimatedPose(3, 3) = 1.0f;
	}

	firstTime = true;

	///// 位置姿勢更新 /////
	glm::mat4 trackingPose( estimatedPose(0,0), estimatedPose(1,0), estimatedPose(2,0), 0,
							estimatedPose(0,1), estimatedPose(1,1), estimatedPose(2,1), 0, 
							estimatedPose(0,2), estimatedPose(1,2), estimatedPose(2,2), 0, 
							estimatedPose(0,3), estimatedPose(1,3), estimatedPose(2,3), 1);

	
	critical_section->setTrackingPoseMatrix(trackingPose);


	///// 遅延補償 /////	

	// 新しい位置姿勢の格納
	timer.stop();
	compensate_point->addData(timer.MSec(), cv::Point3f(trans[0],trans[1],trans[2]));
	compensate_quat->addData(timer.MSec(), glm::quat_cast(rotation));

	// 投影予測位置
	cv::Point3f compensate_point2 = compensate_point->calcYValue(timer.MSec()+delayTime);

	// 遅延補償した位置姿勢
	glm::mat4 compensatePose = glm::mat4_cast(compensate_quat->calcYValue(timer.MSec()+delayTime));
	compensatePose[3][0] = compensate_point2.x;
	compensatePose[3][1] = compensate_point2.y;
	compensatePose[3][2] = compensate_point2.z;

	critical_section->setCompensatePoseMatrix(compensatePose);
}


// エラー率の推定
void TrackingThread::calcError()
{
	float sum = 0.f;
	float max_error = max_distance * (int)correspondPoints.size();

	// 総和
	for (int i = 0; i < (int)correspondPoints.size(); ++i){
		sum += correspondPoints[i].dist;
	}

	// エラー率が閾値以上になったらトラッキング終了
	if ( sum / max_error > error_th || sum < 0.00001) {
		critical_section->tracking_flag = false;
		critical_section->tracking_success_flag = false;
	}
	else if(critical_section->detect_tracking_flag){
		critical_section->tracking_flag = true;
		critical_section->tracking_success_flag = true;
	}
}

// 描画シーン(元モデル)
void TrackingThread::scene_orig(int width, int height)
{	
	///// 元のモデル用 /////
	glEnable(GL_POLYGON_OFFSET_FILL);	// 僅かに奥にずらす
	glPolygonOffset(1.f, 1.f);

	glUseProgram(program_orig);

	// カメラ座標系へ変換
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// モデルの座標変換
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// 内部パラメータを使う場合
	if(critical_section->use_calib_flag)
	{
		MVP = cameraMat * ViewMatrix * ModelMatrix;
	}

	// Uniform変数に行列を送る
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, critical_section->getLightPower());

	// モデルのレンダリング
	orig_mesh.render();
	glDisable(GL_POLYGON_OFFSET_FILL);
}


// 描画シーン(トラッキングモデル)
void TrackingThread::scene_point(int width, int height)
{	
	///// トラッキングモデル用 /////
	glUseProgram(program_point);

	// カメラ座標系へ変換
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// モデルの座標変換
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// 内部パラメータを使う場合
	if(critical_section->use_calib_flag)
	{
		MVP = cameraMat * ViewMatrix * ModelMatrix;
	}

	glUniformMatrix4fv(pointMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(pointModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(pointViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);


	// ポイントスプライトによるレンダリング
	// シェーダの変数に頂点情報を対応付ける
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// VBOの指定
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO[0]);
	// 頂点情報の格納場所と書式の指定
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// 頂点

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[1]);
	// 頂点情報の格納場所と書式の指定
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);			// 法線

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[2]);
	// 頂点情報の格納場所と書式の指定
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);			// インデックス

	// IBOの指定
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pointIBO);

	glDrawElements(GL_POINTS, pointCloud.size(), GL_UNSIGNED_INT, (void*)0);

	// 頂点の対応付けの解除
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

	//　バインドしたものをもどす
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



// 描画シーン(入力画像)
void TrackingThread::scene_background(int width, int height)
{
	// テクスチャの更新
	input_mesh.textureUpdate(edge_img);
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)windowWidth, (float)windowHeight, 0.f, -100.f, 1.f);		// farクリップ面近くで描画
	input_mesh.changeMatrix(OrthoMatrix);

	// 描画
	input_mesh.draw();
}




// 頂点の描画
void TrackingThread::draw_point(const std::vector<glm::vec3>& point_cloud, int width, int height)
{
	glUseProgram(program_edge);

	// カメラ座標系へ変換
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// モデルの座標変換
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// 内部パラメータを使う場合
	if(critical_section->use_calib_flag)
	{
		MVP = cameraMat * ViewMatrix * ModelMatrix;
	}

	glUniformMatrix4fv(pMatrixID, 1, GL_FALSE, &MVP[0][0]);


	std::vector<unsigned int> index_vect(point_cloud.size());
	for (int i = 0; i < point_cloud.size(); ++i){
		index_vect[i] = i;
	}

	// VBOの生成
    glBindBuffer(GL_ARRAY_BUFFER, pVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*point_cloud.size(), &point_cloud[0], GL_DYNAMIC_DRAW);

	// IBOの生成
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*index_vect.size(), &index_vect[0], GL_STATIC_DRAW);

	// シェーダの変数に頂点情報を対応付ける
	glEnableVertexAttribArray(0);

	// 頂点情報の格納場所と書式の指定
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// 頂点

	// IBOの指定
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIBO);

	glDrawElements(GL_POINTS, point_cloud.size(), GL_UNSIGNED_INT, 0);

	// 頂点の対応付けの解除
    glDisableVertexAttribArray(0);

	//　バインドしたものをもどす
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// 初期姿勢のセット
void TrackingThread::setInitPose()
{
	// モデルの座標変換
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	// GLMからCVへの変換
	estimatedPose = cv::Matx44f(  ModelMatrix[0][0], ModelMatrix[1][0], ModelMatrix[2][0], ModelMatrix[3][0]
								, ModelMatrix[0][1], ModelMatrix[1][1], ModelMatrix[2][1], ModelMatrix[3][1]
								, ModelMatrix[0][2], ModelMatrix[1][2], ModelMatrix[2][2], ModelMatrix[3][2]
								, ModelMatrix[0][3], ModelMatrix[1][3], ModelMatrix[2][3], ModelMatrix[3][3]);
}


// 2次元平面への投影
void TrackingThread::projection3Dto2D(const cv::Vec3f& point3D, const cv::Vec3f& normal3D, cv::Vec2f& point2D, cv::Vec2f& normal2D)
{
	// 3次元位置
	cv::Vec3f point3Dnow;
	point3Dnow[0] = estimatedPose(0,0)*point3D[0] + estimatedPose(0,1)*point3D[1] + estimatedPose(0,2)*point3D[2] + estimatedPose(0,3);
	point3Dnow[1] = estimatedPose(1,0)*point3D[0] + estimatedPose(1,1)*point3D[1] + estimatedPose(1,2)*point3D[2] + estimatedPose(1,3);
	point3Dnow[2] = estimatedPose(2,0)*point3D[0] + estimatedPose(2,1)*point3D[1] + estimatedPose(2,2)*point3D[2] + estimatedPose(2,3);

	// 2次元位置
	point2D[0] = -camMat(0,0) * point3Dnow[0] / point3Dnow[2] + camMat(0,2);
	point2D[1] = camMat(1,1) * point3Dnow[1] / point3Dnow[2] + camMat(1,2);

	// 3次元法線
	cv::Vec3f normal3Dnow;
	normal3Dnow[0] = estimatedPose(0,0)*normal3D[0] + estimatedPose(0,1)*normal3D[1] + estimatedPose(0,2)*normal3D[2] + estimatedPose(0,3);
	normal3Dnow[1] = estimatedPose(1,0)*normal3D[0] + estimatedPose(1,1)*normal3D[1] + estimatedPose(1,2)*normal3D[2] + estimatedPose(1,3);
	normal3Dnow[2] = estimatedPose(2,0)*normal3D[0] + estimatedPose(2,1)*normal3D[1] + estimatedPose(2,2)*normal3D[2] + estimatedPose(2,3);

	// 2次元法線
	normal2D[0] = -camMat(0,0) * normal3Dnow[0] / normal3Dnow[2] + camMat(0,2);
	normal2D[1] = camMat(1,1) * normal3Dnow[1] / normal3Dnow[2] + camMat(1,2);


	if (point3Dnow[2] == 0.f) {
		point2D[0] = point2D[1] = std::numeric_limits<float>::infinity();
	} else {
		float th_max = 10000.0;
		float th_min = -10000.0;
		if (point2D[0] < th_min || point2D[0] > th_max) {
			point2D[0] = std::numeric_limits<float>::infinity();
		} 
		if (point2D[1] < th_min || point2D[1] > th_max) {
			point2D[1] = std::numeric_limits<float>::infinity();
		} 
	}
}



/**
 * @brief   点群の読み込み(PLY形式　面無し)
 * 
 * @param   fileName[in]			ファイル名
 * @param   point_cloud[in,out]		3次元点群
 * @param   point_normal[in,out]	3次元点群の法線
 */
void TrackingThread::loadPointCloudPly(const std::string& fileName, std::vector<glm::vec3>& point_cloud, std::vector<glm::vec3>& point_normal)
{
	std::ifstream ifs(fileName);
	std::string buf;
	unsigned int i;

	// ファイルオープン
	if(!ifs.is_open()){
		std::cerr << "ファイルを開けません："<< fileName << std::endl;
		return;
	}

	// ヘッダの読み込み
	int vertex_size = 0;
	while(ifs >> buf)
	{
		if(buf =="vertex"){
			ifs >> vertex_size;
			if(ifs.fail()){
				std::cerr <<"error! vertices_num is not int"<<std::endl;
			}
		}
		if(buf == "end_header"){
			break;
		}
	
	}

	if(vertex_size == 0){
		std::cerr << "vertices_num is 0" << std::endl;
		return;
	}

	std::getline(ifs,buf);
	point_cloud.resize(vertex_size);
	point_normal.resize(vertex_size);

	// 頂点と法線を読み込む
	for (i = 0; i < vertex_size; ++i)
	{
		std::getline(ifs,buf);
		std::vector<std::string> offset = split(buf, ' ');

		point_cloud[i].x = (float)atof(offset[0].c_str());
		point_cloud[i].y = (float)atof(offset[1].c_str());
		point_cloud[i].z = (float)atof(offset[2].c_str());
		point_normal[i].x = (float)atof(offset[3].c_str());
		point_normal[i].y = (float)atof(offset[4].c_str());
		point_normal[i].z = (float)atof(offset[5].c_str());
	}
}


// 区切り文字による分割
std::vector<std::string> TrackingThread::split(const std::string &str, char sep)
{
	std::vector<std::string> v;
	std::stringstream ss(str);
    std::string buffer;
    while( std::getline(ss, buffer, sep) ) {
        v.push_back(buffer);
    }
    return v;
}


// カラーバッファを入れ替えてイベントを取り出す
void TrackingThread::swapBuffers()
{
	// カラーバッファを入れ替える
	glfwSwapBuffers(window);

	glfwPollEvents();
}