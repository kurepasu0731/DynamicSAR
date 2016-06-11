#include "Main_thread.h"


// 初期処理
void MainThread::init(const std::string& modelFile, const std::string& trackingFile)
{
	// 背景色
	glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);

	// デプステスト
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);


	/////////////// 対象物体のモデル //////////////////

	//	プログラムオブジェクトを作成する
	init_GLSL( &program, "../../Shader/shading.vert", "../../Shader/shading.frag");
	glUseProgram(program);

	// uniform変数の場所を取得
	MatrixID = glGetUniformLocation(program, "MVP");
	ViewMatrixID = glGetUniformLocation(program, "V");
	ModelMatrixID = glGetUniformLocation(program, "M");

	// 光源位置の取得
	LightID = glGetUniformLocation(program, "LightPosition_worldspace");
	LightPowerID = glGetUniformLocation(program, "LightPower");

	// モデルの読み込み
	modelFilePath = modelFile;
	mesh.loadMesh(modelFilePath);

	// 追跡用のファイル
	trackingFilePath = trackingFile;

	/////////////// 背景 //////////////////

	// カメラ画像
	cv::Mat camera_image;
	irCamDev->getImage(camera_image);

	// 背景の初期設定
	camera_background.init(camera_image, "../../Shader/background.vert", "../../Shader/background.frag", windowWidth, windowHeight);
}


// 終了処理
void MainThread::end()
{
	// 終了フラグ
	projThread_end_flag = true;
	detectThread_end_flag = true;
	trackingThread_end_flag = true;

	// スレッド終了の待機
	projThread.join();
	detectThread.join();
	trackingThread.join();
}


// 毎フレーム行う処理
void MainThread::display()
{
	// ウィンドウを消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	// 背景色
	glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);

	// カメラと照明の位置を合わせる
	lightPos = camera.eyePosition;

	// 検出時
	if (detection_flag || (critical_section->detect_tracking_flag && !critical_section->tracking_success_flag))
	{
		std::vector<glm::mat4> detectPose;
		critical_section->getDetectPoseMatrix(detectPose);

		// 検出数だけ描画
		for(int i = 0; i < detectPose.size(); ++i)
		{
			draw_detectPose(detectPose[i]);
		}
	}
	else //トラッキング時?
	{
		// モデルの描画
		draw_model();
	}

	// 背景(カメラ画像)の描画
	draw_background();

	// GUIの描画
	TwDraw();

	// カラーバッファを入れ替え,イベントを取得
	swapBuffers();
}


// モデルの描画
void MainThread::draw_model()
{
	// シェーダプログラムの使用開始
	glUseProgram(program);

	// カメラ座標系へ変換
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), windowWidth, windowHeight);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// モデルの座標変換
	glm::mat4 RotationMatrix = glm::mat4_cast(glm::normalize(model_pose));
	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), model_trans); 
	glm::mat4 ScalingMatrix = scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 ModelMatrix = ViewMatrix * TranslationMatrix * RotationMatrix * ScalingMatrix;

	// トラッキング時
	if (critical_section->tracking_flag)
	{
		ModelMatrix = critical_section->getTrackingPoseMatrix();
	}


	glm::mat4 MVP = ProjectionMatrix * ModelMatrix;

	// 内部パラメータを使う場合
	if(use_calib_flag)
	{
		MVP = cameraMat * ModelMatrix;
	}


	// Uniform変数に行列を送る
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, lightPower);

	// モデルのレンダリング
	mesh.render();

	critical_section->setModelMatrix( ModelMatrix);
	critical_section->setLightPower( lightPower);
}


// 検出結果の描画
void MainThread::draw_detectPose(const glm::mat4 &poseMatrix)
{
	// シェーダプログラムの使用開始
	glUseProgram(program);

	glm::mat4 ProjectionMatrix = cameraMat;
	glm::mat4 ViewMatrix = glm::mat4(1.f);
	glm::mat4 ModelMatrix = poseMatrix;

	glm::mat4	MVP = cameraMat * ModelMatrix;
	

	// Uniform変数に行列を送る
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, lightPower);

	// モデルのレンダリング
	mesh.render();

	// トラッキングに失敗していなかったら
	if ( (critical_section->detect_tracking_flag && !critical_section->tracking_success_flag) || detection_flag)
	{
		critical_section->setModelMatrix( ModelMatrix);
		critical_section->setLightPower(lightPower);
	}
}


// 背景の描画
void MainThread::draw_background()
{
	// カメラ画像の更新
	cv::Mat cam_img;
	irCamDev->getImage(cam_img);

	// 歪み補正
	if(critical_section->use_calib_flag)
	{
		cv::Mat map1, map2;
		critical_section->getUndistortMap(map1, map2);
		cv::remap(cam_img, cam_img, map1, map2, cv::INTER_LINEAR);
	}

	// 背景のレンダリング
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)windowWidth, (float)windowHeight, 0.f, -100.f, 1.f);		// farクリップ面近くで描画
	camera_background.changeMatrix(OrthoMatrix);
	camera_background.textureUpdate(cam_img);
	camera_background.draw();
}


// 投影用スレッドの生成
void MainThread::createProjectionThread()
{
	// モニターを検出
	Monitor::SearchDisplay();		
	if (disp_num > Monitor::Disps_Prop.size()-1) 
	{
		std::cerr << "ディスプレイ" << disp_num << "が見つかりません" << std::endl;
		exit(0);
	}

	Monitor::Disp_Prop di = Monitor::Disps_Prop[disp_num];		// ディスプレイを指定

	// 枠無し描画
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	ProjectionThread proj_thread(critical_section.get(), irCamDev, rgbCamDev, use_chessboard, cornerCol, cornerRow, cornerInterval_m, graycode_delay, saveCalibFolder, di.width, di.height);
	glfwSetWindowPos(proj_thread.getWindowID(), di.x, di.y);	// ウィンドウ位置
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);

	// 初期処理
	proj_thread.init(modelFilePath);

	// 繰り返し処理
	while (!projThread_end_flag)
	{
		// 描画
		proj_thread.display();
	}
}


// 検出用スレッドの生成
void MainThread::createDetectionThread()
{
	DetectionThread detect_thread(critical_section.get(), irCamDev);


	// 繰り返し処理
	while (!detectThread_end_flag)
	{
		if (detection_flag || critical_section->detect_tracking_flag)
		{
			// 初期処理
			if (!critical_section->ready_detect_flag)
			{
				detect_thread.init(fernsParamFile, lerningDataFolder);
			}

			// 検出処理
			if (critical_section->use_calib_flag)
			{
				detect_thread.detection();
			} 
			else
			{
				std::cerr << "カメラパラメータを読み込んでください" << std::endl;
			}
		}
		cv::waitKey(1);
	}
}


// 追跡スレッドの生成
void MainThread::createTrackingThread()
{
	// カメラパラメータが読み込まれるまで待機
	while (!trackingThread_end_flag){
		if (critical_section->use_calib_flag){
			break;
		}
	}

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);	// ウィンドウサイズ固定
	TrackingThread tracking_thread(critical_section.get(), irCamDev, windowWidth, windowHeight);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// 繰り返し処理
	while (!trackingThread_end_flag)
	{	
		if (!ready_tracking_flag) {
			// 初期処理
			tracking_thread.init(modelFilePath, trackingFilePath, critical_section->getCameraIntrinsicCV(), critical_section->getCameraIntrinsicMatrix());
			ready_tracking_flag = true;
		}

		tracking_thread.display();
	}
}


// カメラパラメータの読み込み
bool MainThread::loadCameraParam(const std::string& camFile)
{
	// xmlファイルの読み込み
	cv::FileStorage cvfs(camFile, cv::FileStorage::READ);

	cv::Mat camMat0;
	cvfs["cameraMatrix"] >> camMat0;
	double width = static_cast<double>(cvfs["imageWidth"]); 
	double height = static_cast<double>(cvfs["imageHeight"]); 

	if(camMat0.empty())
	{
		std::cerr << "内部パラメータを読み込めませんでした" << std::endl;
		return false;
	}

	camMat = camMat0;

	// OpenGL形式に変換
	glm::mat4 intrinsic(2.0*camMat(0,0)/width, 0, 0, 0,
                camMat(0,1), 2.0*camMat(1,1)/height, 0, 0,
                (width-2.0*camMat(0,2))/width, (2.0*camMat(1,2)-height)/height, -(camera.getFar()+camera.getNear())/(camera.getFar()-camera.getNear()), -1,
                0, 0,  -2*camera.getFar()*camera.getNear()/(camera.getFar()-camera.getNear()), 0);


	// OpenGLのプロジェクション行列
	cameraMat = intrinsic;

	// 歪み補正マップの生成
	cv::Mat map1, map2, distort;
	cvfs["distCoeffs"] >> distort;
	cv::initUndistortRectifyMap(camMat0, distort, cv::Mat(), camMat0, cv::Size(width , height), CV_32FC1, map1, map2);

	critical_section->setCameraIntrinsicMatrix(cameraMat);
	critical_section->setCameraIntrinsicCV(camMat);
	critical_section->setUndistortMap(map1, map2);
	critical_section->use_calib_flag = true;

	return true;
}



// カラーバッファを入れ替えてイベントを取り出す
void MainThread::swapBuffers()
{
	// カラーバッファを入れ替える
	glfwSwapBuffers(window);

	// イベントを取り出す
	glfwPollEvents();
}


// リサイズのコールバック
void MainThread::ResizeCB(GLFWwindow *const window, int width, int height)
{
	// ウィンドウ全体をビューポート
	glViewport(0, 0, width, height);

	// thisポインタを取得
	MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));
	
	if(instance != NULL)
	{
		instance->windowWidth = width;
		instance->windowHeight = height;
		instance->camera.updateProjectionMatrix(instance->camera.getFov(), instance->camera.getNear(), instance->camera.getFar(), width, height);
		instance->camera_background.resizeRectangle(width, height);
	}

	// GUIのサイズ変更
	TwWindowSize(width, height);
}

// マウスボタンのコールバック
void MainThread::MouseButtonCB(GLFWwindow *const window, int button, int action, int mods)
{
	if(!TwEventMouseButtonGLFW( button , action ))
	{
		// thisポインタの取得
		MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));

		if(instance != NULL)
		{
			if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			{
				// 現在のマウス位置を登録
				glfwGetCursorPos(window, &instance->lastMousePosX, &instance->lastMousePosY);
				instance->mousePress_flag = true;
			}
			else if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			{
				instance->mousePress_flag = false;
			}
		}
	}
}

// マウス操作のコールバック
void MainThread::MousePosCB(GLFWwindow *const window, double x, double y)
{
	// thisポインタの取得
	MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));

	if(!TwEventMousePosGLFW( (int)x, (int)y ))
	{
		if(instance != NULL)
		{
			// 左クリックした状態であれば
			if(instance->mousePress_flag)
			{
				double mousex, mousey;
				glfwGetCursorPos(window, &mousex, &mousey);	

				// x,y座標の移動
				instance->model_trans.x = instance->model_trans.x + 0.001*(mousex - instance->lastMousePosX);
				instance->model_trans.y = instance->model_trans.y + 0.001*(instance->lastMousePosY - mousey);

				// 前回のマウス位置を更新
				instance->lastMousePosX = mousex;
				instance->lastMousePosY = mousey;
			}
		}
	}
	else
	{
		if(instance != NULL)
		{
			instance->mousePress_flag = false;
		}
	}
}

// マウスホイールのコールバック
void MainThread::MouseScrollCB(GLFWwindow *window, double x, double y)
{
	if(!TwEventMouseWheelGLFW( (int)y ))
	{
		// thisポインタの取得
		MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));

		if(instance != NULL)
		{
			glm::vec3 campos = instance->camera.getEyePosition();
			if(y>0)
				campos.z -= 0.05;
			else
				campos.z += 0.05;
			instance->camera.setEyePosition(campos);
		}
	}
}

// キーボードのコールバック
void MainThread::KeyFunCB(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(!TwEventKeyGLFW( key, action) && !TwEventCharGLFW( key, action))
	{
		// インスタンスのthisポインタ
		MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));

		if(instance != NULL)
		{
			// キーの状態を保存する
			instance->keyStatus = action;
		}
	}
}


void TW_CALL MainThread::SetQuatNormalizeCB(const void *value, void *clientData)
{
	glm::quat *quaternion = static_cast<glm::quat *>(clientData);
	*quaternion = glm::normalize(*(const glm::quat *)value);
}

void TW_CALL MainThread::GetQuatNormalizeCB(void *value, void *clientData)
{
	glm::quat *quaternion = static_cast<glm::quat *>(clientData);
	*(glm::quat *)value = glm::normalize(*quaternion);
}

// キャリブレーションデータを用いる際のコールバック
void TW_CALL MainThread::SetCalibCB(const void *value, void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);
	render->use_calib_flag = *(bool *)value;

	if(render->use_calib_flag){
		render->use_calib_flag = render->loadCameraParam(render->saveCalibFolder+"/camera.xml");
	}
}

// キャリブレーションデータを用いる際のコールバック
void TW_CALL MainThread::GetCalibCB(void *value, void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);
	*(bool *)value  = render->use_calib_flag;
}


// 検出パラメータ更新時のコールバック
void TW_CALL MainThread::SetDetectParamCB(void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);

	render->critical_section->setDetectParams(render->min_distance1, render->max_distance1, render->distance_step, render->min_scale2, render->max_scale2, render->scale_step,
									render->detect_width, render->detect_height, render->grad_th, render->meanshift_radius, render->win_th, render->likelihood_th);
}


// 追跡パラメータ更新時のコールバック
void TW_CALL MainThread::SetTrackingParamCB(void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);

	render->critical_section->setTrackingParams(render->find_distance, render->error_th, render->edge_th1, render->edge_th2, render->trackingTime, render->delayTime);
}
