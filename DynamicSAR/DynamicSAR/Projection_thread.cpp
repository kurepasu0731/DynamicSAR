#include "Projection_thread.h"


/**
 * @brief   初期処理
 *
 * @param	modelFile[in]	モデルファイル名
 */
void ProjectionThread::init(const std::string& modelFile)
{
	// 背景色
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// デプステスト
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	// ポイントスプライトの設定
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);


	// オフスクリーンレンダリングの初期化
	offscreen.init(offscreen.offscreen_width, offscreen.offscreen_height);

	/////////////// 対象物体のモデル //////////////////

	//	プログラムオブジェクトを作成する
	init_GLSL( &program, "../../Shader/shading_tex.vert", "../../Shader/shading_tex.frag");
	glUseProgram(program);

	// uniform変数の場所を取得
	MatrixID = glGetUniformLocation(program, "MVP");
	ViewMatrixID = glGetUniformLocation(program, "V");
	ModelMatrixID = glGetUniformLocation(program, "M");

	// 光源位置の取得
	LightID = glGetUniformLocation(program, "LightPosition_worldspace");
	LightPowerID = glGetUniformLocation(program, "LightPower");

	// モデルの読み込み
	mesh.loadMesh(modelFile);


	/////////////// 背景 //////////////////

	// カメラ画像
	cv::Mat Black(windowHeight, windowWidth, CV_8UC3, cv::Scalar(0.0, 0.0, 0.0));

	// 背景の初期設定
	projection_background.init(Black, "../../Shader/background.vert", "../../Shader/background.frag", windowWidth, windowHeight);


	/////////////// チェッカーボード点群用 //////////////////
	init_GLSL( &program_point, "../../Shader/point.vert", "../../Shader/point.frag");
	glUseProgram(program_point);

	// uniform変数の場所を取得
	PointMatrixID = glGetUniformLocation(program_point, "MVP");
	
	// 点の大きさ
	glUseProgram(program_point);
	GLuint pointSizeID = glGetUniformLocation(program_point, "point_size");
	GLfloat p_size = 10.f;
	glUniform1f(pointSizeID, p_size);

	// VBOの生成
    glGenBuffers(1, &PointVBO);
    glGenBuffers(1, &PointIBO);

	// カメラの確認用
	cv::Mat ir_img;
	irCamDev->getImage(ir_img);
	camera_background.init(ir_img, "../../Shader/background.vert", "../../Shader/background.frag", ir_img.cols, ir_img.rows);
	camera_offscreen.offscreen_width = ir_img.cols;
	camera_offscreen.offscreen_height = ir_img.rows;
	camera_offscreen.init(ir_img.cols, ir_img.rows);
}



/**
 * @brief   毎フレーム行う処理
 */
void ProjectionThread::display()
{

	///// グレイコード投影 /////
	if (critical_section->graycode_flag)
	{
		std::cout << "グレイコード投影" << std::endl;

		// 白色投影
		cv::Mat white(windowHeight, windowWidth, CV_8UC3, cv::Scalar(255.0, 255.0, 255.0));
		img_projection(white);
		Sleep(400);

		// カメラの撮影
		cv::Mat ir_img, rgb_img;
		irCamDev->getImage(ir_img);
		rgbCamDev->captureImage(rgb_img);
		rgbCamDev->captureImage(rgb_img);
		rgbCamDev->captureImage(rgb_img);

		// グレイコードを投影して対応点を取得
		calib->runGetCorrespond(ir_img, rgb_img, this);

		critical_section->graycode_flag = false;
	}

	///// キャリブレーションフラグ /////
	else if (critical_section->calib_flag)
	{
		if(calib->calib_count > 2)
		{
			std::cout << "キャリブレーション中…" << std::endl;

			// キャリブレーション
			cv::Mat ir_img;
			irCamDev->getImage(ir_img);
			calib->proCamCalibration(calib->myWorldPoints, calib->myCameraPoints, calib->myProjectorPoints, cv::Size(ir_img.cols, ir_img.rows), cv::Size(windowWidth, windowHeight));
			
			loadProjectorParam();		// プロジェクタパラメータの読み込み
		} else {
			std::cout << "対応点数が足りません" << std::endl;
		}
		critical_section->calib_flag = false;
	}

	///// キャリブレーションファイルの読み込み /////
	else if (critical_section->load_calib_flag)
	{
		cv::Mat ir_img;
		irCamDev->getImage(ir_img);
		calib->loadCalibParam(saveCalibFolder+"/calibration.xml");

		loadProjectorParam();			// プロジェクタパラメータの読み込み

		critical_section->load_calib_flag = false;
	}

	///// チェッカーボードへ再投影 /////
	else if (critical_section->run_reprojection_flag)
	{
		runCheckerReprojection();
	}
	///// 検出＆追跡 /////
	else if (critical_section->detect_tracking_flag)
	{
		///// オフスクリーン //////
		offscreen.startRender();

		// ウィンドウを消去する
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClearColor(0.0, 0.0, 0.0, 1.0);

		// カメラと照明の位置を合わせる
		lightPos = camera.eyePosition;

		// トラッキング成功時のみ
		if (critical_section->tracking_success_flag)
		{
			// モデルの描画
			draw_model();
		}

		// レンダリング画像
		cv::Mat renderImg;
		offscreen.getRenderRGB(renderImg);
		offscreen.endRender();

		// 画像反転
		cv::flip(renderImg, renderImg, 0);

		// 歪み補正
		cv::Mat map1, map2;
		critical_section->getProjUndistortMap(map1, map2);
		if (critical_section->use_projCalib_flag && map1.cols > 0)
		{
			cv::remap(renderImg, renderImg, map1, map2, cv::INTER_LINEAR);
		}

		// 背景のレンダリング
		img_projection(renderImg);
	}
	///// 白の投影 /////
	else if (critical_section->project_white_flag)
	{
		cv::Mat white(windowHeight, windowWidth, CV_8UC3, cv::Scalar(255, 255, 255));
		
		// 背景のレンダリング
		img_projection(white);
	}
	else
	{
		///// オフスクリーン //////
		offscreen.startRender();

		// ウィンドウを消去する
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClearColor(0.0, 0.0, 0.0, 1.0);

		// カメラと照明の位置を合わせる
		lightPos = camera.eyePosition;

		// モデルの描画
		draw_model();

		// レンダリング画像
		cv::Mat renderImg;
		offscreen.getRenderRGB(renderImg);
		offscreen.endRender();

		// 画像反転
		cv::flip(renderImg, renderImg, 0);

		// 歪み補正
		cv::Mat map1, map2;
		critical_section->getProjUndistortMap(map1, map2);
		if (critical_section->use_projCalib_flag && map1.cols > 0)
		{
			cv::remap(renderImg, renderImg, map1, map2, cv::INTER_LINEAR);
		}

		// 背景のレンダリング
		img_projection(renderImg);
	}
}


/**
 * @brief   モデルの描画
 */
void ProjectionThread::draw_model()
{
	// シェーダプログラムの使用開始
	glUseProgram(program);

	// カメラ座標系へ変換
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), windowWidth, windowHeight);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// モデルの座標変換
	glm::mat4 RotationMatrix = glm::mat4_cast(glm::normalize(quatOrientation));
	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), glm::vec3(0.0,0.0,0.0)); 
	glm::mat4 ScalingMatrix = scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
	//glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	// 遅延補償
	if(critical_section->tracking_flag && critical_section->compensation_delay_flag)
	{
		ModelMatrix = critical_section->getCompensatePoseMatrix();
	}


	glm::mat4 MVP = ProjectionMatrix * ModelMatrix;

	// キャリブレーション済みであれば
	if (calib->calib_flag)
	{
		ProjectionMatrix = proj_intrinsicMat;
		ModelMatrix = proj_extrinsicMat * ModelMatrix;
		MVP = ProjectionMatrix * ModelMatrix;
	}

	// Uniform変数に行列を送る
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, critical_section->getLightPower()+1.0f);

	// モデルのレンダリング
	mesh.render();
}



/**
 * @brief   画像の投影
 * 
 * @param	projMat[in]		投影画像
 */
void ProjectionThread::img_projection(const cv::Mat& projMat)
{
	// ウィンドウを消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	// 背景色
	glClearColor(0.0, 0.0, 0.0, 1.0);


	// 背景のレンダリング
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)windowWidth, (float)windowHeight, 0.f, -100.f, 1.f);		// farクリップ面近くで描画
	projection_background.changeMatrix(OrthoMatrix);
	projection_background.textureUpdate(projMat);
	projection_background.draw();

	// カラーバッファを入れ替え,イベントを取得
	swapBuffers();
}


/**
 * @brief   頂点の描画
 * 
 * @param	point_cloud[in]		頂点配列
 */
void ProjectionThread::draw_point(const std::vector<glm::vec3>& point_cloud)
{
	std::vector<unsigned int> index_vect(point_cloud.size());
	for (int i = 0; i < point_cloud.size(); ++i){
		index_vect[i] = i;
	}

	// VBOの生成
    glBindBuffer(GL_ARRAY_BUFFER, PointVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*point_cloud.size(), &point_cloud[0], GL_DYNAMIC_DRAW);

	// IBOの生成
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PointIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*index_vect.size(), &index_vect[0], GL_STATIC_DRAW);

	// シェーダの変数に頂点情報を対応付ける
	glEnableVertexAttribArray(0);

	// 頂点情報の格納場所と書式の指定
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// 頂点

	// IBOの指定
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PointIBO);

	glDrawElements(GL_POINTS, point_cloud.size(), GL_UNSIGNED_INT, 0);

	// 頂点の対応付けの解除
    glDisableVertexAttribArray(0);

	//　バインドしたものをもどす
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



/**
 * @brief   キャリブレーション結果を用いてチェッカーボードに再投影(OpenGL基準)
 */
void ProjectionThread::runCheckerReprojection()
{
	if(calib->calib_flag)
	{
		std::cout << "チェッカーパターンへの再投影中…" << std::endl;

		// キャプチャ
		cv::Mat ir_img;
		irCamDev->getImage(ir_img);
					
		// チェッカーパターンの交点を検出(カメラ)
		cv::Mat ir_undistort = ir_img.clone();
		// 歪み補正
		if(critical_section->use_calib_flag)
		{
			cv::Mat map1, map2;
			critical_section->getUndistortMap(map1, map2);
			cv::remap(ir_img, ir_undistort, map1, map2, cv::INTER_LINEAR);
		}

		std::vector<cv::Point3f> worldPoint;
		std::vector<cv::Point2f> imagePoint;
		std::vector<cv::Point2f> projPoint;
		bool detect_flag = calib->getCorners(imagePoint, ir_undistort, ir_undistort);		// チェッカーパターン検出

		cv::Mat proj_img = cv::Mat(windowHeight, windowWidth, CV_8UC3, cv::Scalar(0, 0, 0));

		if(detect_flag) 
		{
			// カメラを中心としたチェッカーパターンの位置取得
			calib->getCameraWorldPoint(worldPoint, imagePoint);

			// カメラ座標への投影(歪み除去後のため歪みパラメータは用いない)
			std::vector<cv::Point2f> cam_projection;
			cv::Mat cam_R = cv::Mat::eye(3, 3, CV_64F);
			cv::Mat cam_T = cv::Mat::zeros(3, 1, CV_64F);


			///// カメラ画像のオフスクリーンレンダリング /////
			camera_offscreen.startRender();

			// ウィンドウを消去する
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, ir_undistort.cols, ir_undistort.rows);
			glClearColor(0.0, 0.0, 0.0, 1.0);

			// 格子点の描画
			glUseProgram(program_point);

			// カメラ座標系へ変換
			glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), ir_undistort.cols, ir_undistort.rows);

			// モデルの座標変換
			glm::mat4 RotationMatrix(1.f);
			glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), glm::vec3(cam_T.at<double>(0), cam_T.at<double>(1), cam_T.at<double>(2))); 
			glm::mat4 GL2CV = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// CV座標系への変換
			glm::mat4 CV2GL = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// GL座標系への変換

			glm::mat4 ModelMatrix = CV2GL * TranslationMatrix * RotationMatrix * GL2CV;
			glm::mat4 MVP = ProjectionMatrix * ModelMatrix;

			// 内部パラメータを使う場合
			if(critical_section->use_calib_flag)
			{
				MVP = critical_section->getCameraIntrinsicMatrix() * ModelMatrix;
			}

			// Uniform変数に行列を送る
			glUniformMatrix4fv(PointMatrixID, 1, GL_FALSE, &MVP[0][0]);

			// 格子点
			std::vector<glm::vec3> checker_point;
			for (int i = 0; i < worldPoint.size(); ++i)
			{
				checker_point.emplace_back(glm::vec3(worldPoint[i].x, -worldPoint[i].y, -worldPoint[i].z));					// GL座標点
			}
			draw_point(checker_point);

			// 背景のレンダリング
			glm::mat4 Ortho = glm::ortho(0.f, (float)ir_undistort.cols, (float)ir_undistort.rows, 0.f, -100.f, 1.f);		// farクリップ面近くで描画
			camera_background.changeMatrix(Ortho);
			camera_background.textureUpdate(ir_undistort);
			camera_background.draw();

			// レンダリング画像
			cv::Mat renderImg;
			camera_offscreen.getRenderRGB(renderImg);

			// 反転
			cv::flip(renderImg, renderImg, 0);

			cv::imshow("IR_camera", renderImg);
			cv::waitKey(1);

			camera_offscreen.endRender();

			// カメラの再投影誤差
			cv::projectPoints(worldPoint, cam_R, cam_T, calib->cam_K, cv::Mat(), cam_projection);
			float cam_error = 0;
			for(int i=0; i<imagePoint.size(); ++i)
			{
				cam_error += std::sqrt((imagePoint[i].x - cam_projection[i].x)*(imagePoint[i].x - cam_projection[i].x) + (imagePoint[i].y - cam_projection[i].y)*(imagePoint[i].y - cam_projection[i].y));
			}

			std::cout << "カメラの再投影誤差：" << cam_error/imagePoint.size() << std::endl;


			///// プロジェクタ画像のオフスクリーンレンダリング //////
			offscreen.startRender();

			// ウィンドウを消去する
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, windowWidth, windowHeight);
			glClearColor(0.0, 0.0, 0.0, 1.0);

			// 格子点の描画
			glUseProgram(program_point);

			// モデルの座標変換
			// 内部パラメータと外部パラメータ
			glm::mat4 MVP2 = proj_intrinsicMat * proj_extrinsicMat;

			// Uniform変数に行列を送る
			glUniformMatrix4fv(PointMatrixID, 1, GL_FALSE, &MVP2[0][0]);

			// 格子点
			draw_point(checker_point);

			// レンダリング画像
			offscreen.getRenderRGB(proj_img);
			offscreen.endRender();

			// 反転
			cv::flip(proj_img, proj_img, 0);

			// 歪み補正
			cv::Mat map1, map2;
			critical_section->getProjUndistortMap(map1, map2);
			if (critical_section->use_projCalib_flag && map1.cols > 0)
			{
				cv::remap(proj_img, proj_img, map1, map2, cv::INTER_LINEAR);
			}
		}

		// 映像の投影
		img_projection(proj_img);
	}
	else 
	{
		std::cerr << "キャリブレーションを行ってください" << std::endl;
	}
}


/**
 * @brief   プロジェクタパラメータの読み込み		
 */
void ProjectionThread::loadProjectorParam()
{
	// プロジェクタの外部パラメータ
	proj_extrinsicMat[0][0] = calib->R.at<double>(0,0);
	proj_extrinsicMat[1][0] = calib->R.at<double>(0,1);
	proj_extrinsicMat[2][0] = calib->R.at<double>(0,2);
	proj_extrinsicMat[3][0] = calib->T.at<double>(0,0);
	proj_extrinsicMat[0][1] = calib->R.at<double>(1,0);
	proj_extrinsicMat[1][1] = calib->R.at<double>(1,1);
	proj_extrinsicMat[2][1] = calib->R.at<double>(1,2);
	proj_extrinsicMat[3][1] = calib->T.at<double>(1,0);
	proj_extrinsicMat[0][2] = calib->R.at<double>(2,0);
	proj_extrinsicMat[1][2] = calib->R.at<double>(2,1);
	proj_extrinsicMat[2][2] = calib->R.at<double>(2,2);
	proj_extrinsicMat[3][2] = calib->T.at<double>(2,0);
	proj_extrinsicMat[0][3] = 0.0;
	proj_extrinsicMat[1][3] = 0.0;
	proj_extrinsicMat[2][3] = 0.0;
	proj_extrinsicMat[3][3] = 1.0;

	glm::mat4 GL2CV = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// CV座標系への変換
	glm::mat4 CV2GL = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// GL座標系への変換

	// 外部パラメータはCV座標系
	proj_extrinsicMat = CV2GL * proj_extrinsicMat * GL2CV;

	// プロジェクタの内部パラメータ
	proj_intrinsic = calib->proj_K;

	// OpenGL形式に変換
	glm::mat4 intrinsic(2.0*proj_intrinsic(0,0)/windowWidth, 0, 0, 0,
                proj_intrinsic(0,1), 2.0*proj_intrinsic(1,1)/windowHeight, 0, 0,
                (windowWidth-2.0*proj_intrinsic(0,2))/windowWidth, (2.0*proj_intrinsic(1,2)-windowHeight)/windowHeight, -(camera.getFar()+camera.getNear())/(camera.getFar()-camera.getNear()), -1,
                0, 0,  -2*camera.getFar()*camera.getNear()/(camera.getFar()-camera.getNear()), 0);


	// OpenGLのプロジェクション行列
	proj_intrinsicMat = intrinsic;


	// 歪み補正マップ
	cv::Mat map1, map2, projMat2;

	cv::initUndistortRectifyMap(calib->proj_K, -calib->proj_dist, cv::Mat(), calib->proj_K, cv::Size(windowWidth , windowHeight), CV_32FC1, map1, map2);

	critical_section->setProjUndistortMap(map1, map2);
	critical_section->use_projCalib_flag = true;
}


// カラーバッファを入れ替えてイベントを取り出す
void ProjectionThread::swapBuffers()
{
	// カラーバッファを入れ替える
	glfwSwapBuffers(window);

	// イベントを取り出す
	glfwPollEvents();
}
