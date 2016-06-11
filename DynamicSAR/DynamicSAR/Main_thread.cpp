#include "Main_thread.h"


// ��������
void MainThread::init(const std::string& modelFile, const std::string& trackingFile)
{
	// �w�i�F
	glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);

	// �f�v�X�e�X�g
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);


	/////////////// �Ώە��̂̃��f�� //////////////////

	//	�v���O�����I�u�W�F�N�g���쐬����
	init_GLSL( &program, "../../Shader/shading.vert", "../../Shader/shading.frag");
	glUseProgram(program);

	// uniform�ϐ��̏ꏊ���擾
	MatrixID = glGetUniformLocation(program, "MVP");
	ViewMatrixID = glGetUniformLocation(program, "V");
	ModelMatrixID = glGetUniformLocation(program, "M");

	// �����ʒu�̎擾
	LightID = glGetUniformLocation(program, "LightPosition_worldspace");
	LightPowerID = glGetUniformLocation(program, "LightPower");

	// ���f���̓ǂݍ���
	modelFilePath = modelFile;
	mesh.loadMesh(modelFilePath);

	// �ǐ՗p�̃t�@�C��
	trackingFilePath = trackingFile;

	/////////////// �w�i //////////////////

	// �J�����摜
	cv::Mat camera_image;
	irCamDev->getImage(camera_image);

	// �w�i�̏����ݒ�
	camera_background.init(camera_image, "../../Shader/background.vert", "../../Shader/background.frag", windowWidth, windowHeight);
}


// �I������
void MainThread::end()
{
	// �I���t���O
	projThread_end_flag = true;
	detectThread_end_flag = true;
	trackingThread_end_flag = true;

	// �X���b�h�I���̑ҋ@
	projThread.join();
	detectThread.join();
	trackingThread.join();
}


// ���t���[���s������
void MainThread::display()
{
	// �E�B���h�E����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	// �w�i�F
	glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);

	// �J�����ƏƖ��̈ʒu�����킹��
	lightPos = camera.eyePosition;

	// ���o��
	if (detection_flag || (critical_section->detect_tracking_flag && !critical_section->tracking_success_flag))
	{
		std::vector<glm::mat4> detectPose;
		critical_section->getDetectPoseMatrix(detectPose);

		// ���o�������`��
		for(int i = 0; i < detectPose.size(); ++i)
		{
			draw_detectPose(detectPose[i]);
		}
	}
	else //�g���b�L���O��?
	{
		// ���f���̕`��
		draw_model();
	}

	// �w�i(�J�����摜)�̕`��
	draw_background();

	// GUI�̕`��
	TwDraw();

	// �J���[�o�b�t�@�����ւ�,�C�x���g���擾
	swapBuffers();
}


// ���f���̕`��
void MainThread::draw_model()
{
	// �V�F�[�_�v���O�����̎g�p�J�n
	glUseProgram(program);

	// �J�������W�n�֕ϊ�
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), windowWidth, windowHeight);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// ���f���̍��W�ϊ�
	glm::mat4 RotationMatrix = glm::mat4_cast(glm::normalize(model_pose));
	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), model_trans); 
	glm::mat4 ScalingMatrix = scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 ModelMatrix = ViewMatrix * TranslationMatrix * RotationMatrix * ScalingMatrix;

	// �g���b�L���O��
	if (critical_section->tracking_flag)
	{
		ModelMatrix = critical_section->getTrackingPoseMatrix();
	}


	glm::mat4 MVP = ProjectionMatrix * ModelMatrix;

	// �����p�����[�^���g���ꍇ
	if(use_calib_flag)
	{
		MVP = cameraMat * ModelMatrix;
	}


	// Uniform�ϐ��ɍs��𑗂�
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, lightPower);

	// ���f���̃����_�����O
	mesh.render();

	critical_section->setModelMatrix( ModelMatrix);
	critical_section->setLightPower( lightPower);
}


// ���o���ʂ̕`��
void MainThread::draw_detectPose(const glm::mat4 &poseMatrix)
{
	// �V�F�[�_�v���O�����̎g�p�J�n
	glUseProgram(program);

	glm::mat4 ProjectionMatrix = cameraMat;
	glm::mat4 ViewMatrix = glm::mat4(1.f);
	glm::mat4 ModelMatrix = poseMatrix;

	glm::mat4	MVP = cameraMat * ModelMatrix;
	

	// Uniform�ϐ��ɍs��𑗂�
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, lightPower);

	// ���f���̃����_�����O
	mesh.render();

	// �g���b�L���O�Ɏ��s���Ă��Ȃ�������
	if ( (critical_section->detect_tracking_flag && !critical_section->tracking_success_flag) || detection_flag)
	{
		critical_section->setModelMatrix( ModelMatrix);
		critical_section->setLightPower(lightPower);
	}
}


// �w�i�̕`��
void MainThread::draw_background()
{
	// �J�����摜�̍X�V
	cv::Mat cam_img;
	irCamDev->getImage(cam_img);

	// �c�ݕ␳
	if(critical_section->use_calib_flag)
	{
		cv::Mat map1, map2;
		critical_section->getUndistortMap(map1, map2);
		cv::remap(cam_img, cam_img, map1, map2, cv::INTER_LINEAR);
	}

	// �w�i�̃����_�����O
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)windowWidth, (float)windowHeight, 0.f, -100.f, 1.f);		// far�N���b�v�ʋ߂��ŕ`��
	camera_background.changeMatrix(OrthoMatrix);
	camera_background.textureUpdate(cam_img);
	camera_background.draw();
}


// ���e�p�X���b�h�̐���
void MainThread::createProjectionThread()
{
	// ���j�^�[�����o
	Monitor::SearchDisplay();		
	if (disp_num > Monitor::Disps_Prop.size()-1) 
	{
		std::cerr << "�f�B�X�v���C" << disp_num << "��������܂���" << std::endl;
		exit(0);
	}

	Monitor::Disp_Prop di = Monitor::Disps_Prop[disp_num];		// �f�B�X�v���C���w��

	// �g�����`��
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	ProjectionThread proj_thread(critical_section.get(), irCamDev, rgbCamDev, use_chessboard, cornerCol, cornerRow, cornerInterval_m, graycode_delay, saveCalibFolder, di.width, di.height);
	glfwSetWindowPos(proj_thread.getWindowID(), di.x, di.y);	// �E�B���h�E�ʒu
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);

	// ��������
	proj_thread.init(modelFilePath);

	// �J��Ԃ�����
	while (!projThread_end_flag)
	{
		// �`��
		proj_thread.display();
	}
}


// ���o�p�X���b�h�̐���
void MainThread::createDetectionThread()
{
	DetectionThread detect_thread(critical_section.get(), irCamDev);


	// �J��Ԃ�����
	while (!detectThread_end_flag)
	{
		if (detection_flag || critical_section->detect_tracking_flag)
		{
			// ��������
			if (!critical_section->ready_detect_flag)
			{
				detect_thread.init(fernsParamFile, lerningDataFolder);
			}

			// ���o����
			if (critical_section->use_calib_flag)
			{
				detect_thread.detection();
			} 
			else
			{
				std::cerr << "�J�����p�����[�^��ǂݍ���ł�������" << std::endl;
			}
		}
		cv::waitKey(1);
	}
}


// �ǐՃX���b�h�̐���
void MainThread::createTrackingThread()
{
	// �J�����p�����[�^���ǂݍ��܂��܂őҋ@
	while (!trackingThread_end_flag){
		if (critical_section->use_calib_flag){
			break;
		}
	}

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);	// �E�B���h�E�T�C�Y�Œ�
	TrackingThread tracking_thread(critical_section.get(), irCamDev, windowWidth, windowHeight);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// �J��Ԃ�����
	while (!trackingThread_end_flag)
	{	
		if (!ready_tracking_flag) {
			// ��������
			tracking_thread.init(modelFilePath, trackingFilePath, critical_section->getCameraIntrinsicCV(), critical_section->getCameraIntrinsicMatrix());
			ready_tracking_flag = true;
		}

		tracking_thread.display();
	}
}


// �J�����p�����[�^�̓ǂݍ���
bool MainThread::loadCameraParam(const std::string& camFile)
{
	// xml�t�@�C���̓ǂݍ���
	cv::FileStorage cvfs(camFile, cv::FileStorage::READ);

	cv::Mat camMat0;
	cvfs["cameraMatrix"] >> camMat0;
	double width = static_cast<double>(cvfs["imageWidth"]); 
	double height = static_cast<double>(cvfs["imageHeight"]); 

	if(camMat0.empty())
	{
		std::cerr << "�����p�����[�^��ǂݍ��߂܂���ł���" << std::endl;
		return false;
	}

	camMat = camMat0;

	// OpenGL�`���ɕϊ�
	glm::mat4 intrinsic(2.0*camMat(0,0)/width, 0, 0, 0,
                camMat(0,1), 2.0*camMat(1,1)/height, 0, 0,
                (width-2.0*camMat(0,2))/width, (2.0*camMat(1,2)-height)/height, -(camera.getFar()+camera.getNear())/(camera.getFar()-camera.getNear()), -1,
                0, 0,  -2*camera.getFar()*camera.getNear()/(camera.getFar()-camera.getNear()), 0);


	// OpenGL�̃v���W�F�N�V�����s��
	cameraMat = intrinsic;

	// �c�ݕ␳�}�b�v�̐���
	cv::Mat map1, map2, distort;
	cvfs["distCoeffs"] >> distort;
	cv::initUndistortRectifyMap(camMat0, distort, cv::Mat(), camMat0, cv::Size(width , height), CV_32FC1, map1, map2);

	critical_section->setCameraIntrinsicMatrix(cameraMat);
	critical_section->setCameraIntrinsicCV(camMat);
	critical_section->setUndistortMap(map1, map2);
	critical_section->use_calib_flag = true;

	return true;
}



// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
void MainThread::swapBuffers()
{
	// �J���[�o�b�t�@�����ւ���
	glfwSwapBuffers(window);

	// �C�x���g�����o��
	glfwPollEvents();
}


// ���T�C�Y�̃R�[���o�b�N
void MainThread::ResizeCB(GLFWwindow *const window, int width, int height)
{
	// �E�B���h�E�S�̂��r���[�|�[�g
	glViewport(0, 0, width, height);

	// this�|�C���^���擾
	MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));
	
	if(instance != NULL)
	{
		instance->windowWidth = width;
		instance->windowHeight = height;
		instance->camera.updateProjectionMatrix(instance->camera.getFov(), instance->camera.getNear(), instance->camera.getFar(), width, height);
		instance->camera_background.resizeRectangle(width, height);
	}

	// GUI�̃T�C�Y�ύX
	TwWindowSize(width, height);
}

// �}�E�X�{�^���̃R�[���o�b�N
void MainThread::MouseButtonCB(GLFWwindow *const window, int button, int action, int mods)
{
	if(!TwEventMouseButtonGLFW( button , action ))
	{
		// this�|�C���^�̎擾
		MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));

		if(instance != NULL)
		{
			if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			{
				// ���݂̃}�E�X�ʒu��o�^
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

// �}�E�X����̃R�[���o�b�N
void MainThread::MousePosCB(GLFWwindow *const window, double x, double y)
{
	// this�|�C���^�̎擾
	MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));

	if(!TwEventMousePosGLFW( (int)x, (int)y ))
	{
		if(instance != NULL)
		{
			// ���N���b�N������Ԃł����
			if(instance->mousePress_flag)
			{
				double mousex, mousey;
				glfwGetCursorPos(window, &mousex, &mousey);	

				// x,y���W�̈ړ�
				instance->model_trans.x = instance->model_trans.x + 0.001*(mousex - instance->lastMousePosX);
				instance->model_trans.y = instance->model_trans.y + 0.001*(instance->lastMousePosY - mousey);

				// �O��̃}�E�X�ʒu���X�V
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

// �}�E�X�z�C�[���̃R�[���o�b�N
void MainThread::MouseScrollCB(GLFWwindow *window, double x, double y)
{
	if(!TwEventMouseWheelGLFW( (int)y ))
	{
		// this�|�C���^�̎擾
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

// �L�[�{�[�h�̃R�[���o�b�N
void MainThread::KeyFunCB(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(!TwEventKeyGLFW( key, action) && !TwEventCharGLFW( key, action))
	{
		// �C���X�^���X��this�|�C���^
		MainThread *const instance(static_cast<MainThread *>(glfwGetWindowUserPointer(window)));

		if(instance != NULL)
		{
			// �L�[�̏�Ԃ�ۑ�����
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

// �L�����u���[�V�����f�[�^��p����ۂ̃R�[���o�b�N
void TW_CALL MainThread::SetCalibCB(const void *value, void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);
	render->use_calib_flag = *(bool *)value;

	if(render->use_calib_flag){
		render->use_calib_flag = render->loadCameraParam(render->saveCalibFolder+"/camera.xml");
	}
}

// �L�����u���[�V�����f�[�^��p����ۂ̃R�[���o�b�N
void TW_CALL MainThread::GetCalibCB(void *value, void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);
	*(bool *)value  = render->use_calib_flag;
}


// ���o�p�����[�^�X�V���̃R�[���o�b�N
void TW_CALL MainThread::SetDetectParamCB(void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);

	render->critical_section->setDetectParams(render->min_distance1, render->max_distance1, render->distance_step, render->min_scale2, render->max_scale2, render->scale_step,
									render->detect_width, render->detect_height, render->grad_th, render->meanshift_radius, render->win_th, render->likelihood_th);
}


// �ǐՃp�����[�^�X�V���̃R�[���o�b�N
void TW_CALL MainThread::SetTrackingParamCB(void* clientData)
{
	MainThread *render = static_cast<MainThread *>(clientData);

	render->critical_section->setTrackingParams(render->find_distance, render->error_th, render->edge_th1, render->edge_th2, render->trackingTime, render->delayTime);
}
