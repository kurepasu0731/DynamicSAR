#ifndef MAIN_THREAD_H
#define MAIN_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/atomic.hpp>


#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <AntTweakBar.h>

#include "GLSL/glsl_setup.h"
#include "GLSL/window.h"
#include "GLSL/camera.h"
#include "GLSL/mesh.h"
#include "GLSL/texture.h"
#include "GLSL/background.h"
#include "GLSL/myTimer.h"

#include "CamDev/WebCamera.h"
#include "CamDev/CameraLinkCamera.h"
#include "CamDev/UndistortedCamera.h"

#include "CriticalSection.h"
#include "Projection_thread.h"
#include "Detection_thread.h"
#include "Tracking_thread.h"

// ���C���X���b�h�N���X
class MainThread : public Window
{

public:
	// �R���X�g���N�^
	MainThread(CamDev::CameraLinkCamera *cameraDevice, CamDev::Camera *RGBCameraDevice = NULL, int _disp_num = 0, bool _use_chessboard=false, int _cornerCol=4, int _cornerRow=11, float _cornerInterval_m=0.0225f, double _graycode_delay=80.0,
				std::string _saveCalibFolder=".", std::string _fernsParamFile="ferns_param.csv", std::string _lerningDataFolder=".", int width = 640, int height = 480, const char *title = "Main Thread")
		: Window( width, height, title)
		, mousePress_flag ( false )
		, lightPower(0.7f)
		, model_trans (0.0, 0.0, 0.0)
		, model_pose (1.0, 0.0, 0.0, 0.0)
		, disp_num (_disp_num)
		, projThread_end_flag (false)
		, use_chessboard (_use_chessboard)
		, cornerCol (_cornerCol)
		, cornerRow (_cornerRow)
		, cornerInterval_m (_cornerInterval_m)
		, graycode_delay (_graycode_delay)
		, saveCalibFolder (_saveCalibFolder)
		, use_calib_flag (false)
		, detection_flag (false)
		, detectThread_end_flag (false)
		, min_distance1 (1.0)
		, max_distance1 (1.3)
		, distance_step (1)
		, min_scale2 (0.9)
		, max_scale2 (1.1)
		, scale_step (2)
		, detect_width (200)
		, detect_height (200)
		, grad_th (200.f)
		, meanshift_radius (64)
		, win_th (80)
		, likelihood_th (0.8)
		, ready_tracking_flag (false)
		, trackingThread_end_flag (false)
		, edge_th1 (60.f)
		, edge_th2 (100.f)
		, find_distance (0.05f)
		, error_th (0.75f)
		, trackingTime (24.0)
		, delayTime (120.0)
	{
		// �X���b�h�ԋ��L�N���X
		critical_section = std::unique_ptr<CriticalSection> (new CriticalSection);

		// �f�o�C�X�̎󂯓n��
		irCamDev = cameraDevice;
		rgbCamDev = RGBCameraDevice;


		///// ���e�X���b�h�̊J�n /////
		projThread = boost::thread( &MainThread::createProjectionThread, this);

		///// ���o�X���b�h�̊J�n /////
		detectThread = boost::thread( &MainThread::createDetectionThread, this);

		///// �ǐՃX���b�h�̊J�n /////
		trackingThread = boost::thread( &MainThread::createTrackingThread, this);


		// GUI�̏�����
		TwInit(TW_OPENGL_CORE, NULL);
		TwWindowSize(width, height);
		myGUI = TwNewBar("GUI");
		TwDefine(" GLOBAL help='This example shows a model loader' ");
		TwDefine(" GUI size='200 400' color='41 126 231' ");
		TwAddVarRW(myGUI, "Background color", TW_TYPE_COLOR3F, &background_color, NULL);
		TwAddVarCB(myGUI, "Quaternion", TW_TYPE_QUAT4F, SetQuatNormalizeCB, GetQuatNormalizeCB, &model_pose, "showval=true opened=true");
		TwAddSeparator(myGUI, NULL, " group='Camera' ");
		TwAddVarRW(myGUI, "Position", TW_TYPE_DIR3F, &camera.eyePosition, " group='Camera' ");
		TwAddVarRW(myGUI, "Vector", TW_TYPE_DIR3F, &camera.eyeVector, " group='Camera' ");
		TwAddVarRW(myGUI, "Up", TW_TYPE_DIR3F, &camera.eyeUp, " group='Camera' ");
		TwAddVarRW(myGUI, "FoV", TW_TYPE_FLOAT, &camera.FoV, " group='Camera' ");
		TwAddSeparator(myGUI, NULL, " group='Light' ");
		TwAddVarRW(myGUI, "Light pos", TW_TYPE_DIR3F, &lightPos, " group='Light' ");
		TwAddVarRW(myGUI, "Power", TW_TYPE_FLOAT, &lightPower, " group='Light' min=0.0 max=100.0 step=0.01");

		// �L�����u���[�V�����pGUI
		calibGUI = TwNewBar("Calibration");
		TwDefine(" Calibration position ='430 10' size='200 140' color='41 250 100' ");
		TwAddVarRW(calibGUI, "Project graycode", TW_TYPE_BOOLCPP, &critical_section->graycode_flag, NULL);
		TwAddVarRW(calibGUI, "Run calibration", TW_TYPE_BOOLCPP, &critical_section->calib_flag, NULL);
		TwAddVarRW(calibGUI, "Run reprojection", TW_TYPE_BOOLCPP, &critical_section->run_reprojection_flag, NULL);
		TwAddVarRW(calibGUI, "Load projector calibration", TW_TYPE_BOOLCPP, &critical_section->load_calib_flag, NULL);
		TwAddVarCB(calibGUI, "Load camera calibration", TW_TYPE_BOOLCPP, SetCalibCB, GetCalibCB, this, NULL);
		TwAddVarRW(calibGUI, "Project white", TW_TYPE_BOOLCPP, &critical_section->project_white_flag, NULL);

		// ���o�pGUI
		detectGUI = TwNewBar("Detectioin");
		TwDefine(" Detectioin position ='430 160' size='200 140' color='241 50 100' ");
		TwAddVarRW(detectGUI, "DetectAndTrack", TW_TYPE_BOOLCPP, &critical_section->detect_tracking_flag, NULL);
		TwAddVarRW(detectGUI, "Detect", TW_TYPE_BOOLCPP, &detection_flag, NULL);
		TwAddButton(detectGUI, "Update parameter", SetDetectParamCB, this, NULL);
		TwAddVarRW(detectGUI, "min_distance1", TW_TYPE_FLOAT, &min_distance1, "min=0.5 max=2.0 step=0.1");
		TwAddVarRW(detectGUI, "max_distance1", TW_TYPE_FLOAT, &max_distance1, "min=0.5 max=2.0 step=0.1");
		TwAddVarRW(detectGUI, "distance_step", TW_TYPE_INT32, &distance_step, "min=1 max=8 step=1");
		TwAddVarRW(detectGUI, "min_scale2", TW_TYPE_FLOAT, &min_scale2, "min=0.5 max=1.6 step=0.1");
		TwAddVarRW(detectGUI, "max_scale2", TW_TYPE_FLOAT, &max_scale2, "min=0.5 max=1.6 step=0.1");
		TwAddVarRW(detectGUI, "scale_step", TW_TYPE_INT32, &scale_step, "min=1 max=8 step=1");
		TwAddVarRW(detectGUI, "detect_width", TW_TYPE_INT32, &detect_width, "min=20 max=400 step=10");
		TwAddVarRW(detectGUI, "detect_height", TW_TYPE_INT32, &detect_height, "min=20 max=400 step=10");
		TwAddVarRW(detectGUI, "gradient threashold", TW_TYPE_FLOAT, &grad_th, "min=0.0 max=4000.0 step=1.0");
		TwAddVarRW(detectGUI, "meanshift_radius", TW_TYPE_INT32, &meanshift_radius, "min=10 max=100 step=1");
		TwAddVarRW(detectGUI, "nearest neighbor threshold", TW_TYPE_INT32, &win_th, "min=10 max=200 step=1");
		TwAddVarRW(detectGUI, "likelihood threshold", TW_TYPE_DOUBLE, &likelihood_th, "min=0.1 max=1.0 step=0.01");

		// �ǐ՗pGUI
		trackingGUI = TwNewBar("Tracking");
		TwDefine(" Tracking position ='430 310' size='200 140' color='101 50 200' ");
		TwAddVarRW(trackingGUI, "Track", TW_TYPE_BOOLCPP, &critical_section->tracking_flag, NULL);
		TwAddVarRW(trackingGUI, "Predict filter", TW_TYPE_BOOLCPP, &critical_section->predict_flag, NULL);
		TwAddVarRW(trackingGUI, "Compensation delay", TW_TYPE_BOOLCPP, &critical_section->compensation_delay_flag, NULL);
		TwAddButton(trackingGUI, "Update parameter", SetTrackingParamCB, this, NULL);
		TwAddVarRW(trackingGUI, "find distance", TW_TYPE_FLOAT, &find_distance, "min=0.01 max=0.2 step=0.01");
		TwAddVarRW(trackingGUI, "error threshold", TW_TYPE_FLOAT, &error_th, "min=0.05 max=1.0 step=0.05");
		TwAddVarRW(trackingGUI, "edge threshold1", TW_TYPE_FLOAT, &edge_th1, "min=0.0 max=400.0 step=1.0");
		TwAddVarRW(trackingGUI, "edge threshold2", TW_TYPE_FLOAT, &edge_th2, "min=0.0 max=400.0 step=1.0");
		TwAddVarRW(trackingGUI, "tracking time", TW_TYPE_DOUBLE, &trackingTime, "min=0.0 max=1000.0 step=1.0");
		TwAddVarRW(trackingGUI, "System delay time", TW_TYPE_DOUBLE, &delayTime, "min=0.0 max=1000.0 step=1.0");

		// �Ɩ��̏����ʒu
		lightPos = glm::vec3(0.0, 0.0, 1.0);

		// �E�B���h�E�̐�������off
		glfwSwapInterval(0);

		// �E�B���h�E�̔w�i�F
		background_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		// �J�����̍X�V
		camera.setEyePosition(glm::vec3(0.0f, 0.0f, 1.5f));
		camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);

		// ���o�p�t�@�C��
		fernsParamFile = _fernsParamFile;
		lerningDataFolder = _lerningDataFolder;

		// �R�[���o�b�N
		glfwSetFramebufferSizeCallback(window, ResizeCB);
		glfwSetMouseButtonCallback(window, MouseButtonCB);
		glfwSetCursorPosCallback(window , MousePosCB);
		glfwSetScrollCallback(window, MouseScrollCB);
		glfwSetKeyCallback(window, KeyFunCB);

		// �C���X�^���X��this�|�C���^���L�^
		glfwSetWindowUserPointer(window, this);
	}

	// �f�X�g���N�^
	virtual ~MainThread()
	{
		// GUI�̏I��
		TwTerminate();
	}

	// ��������
	void init(const std::string& modelFile, const std::string& trackingFile);

	// �I������
	void end();

	// ���t���[���s������
	void display();

	// ���f���̕`��
	void draw_model();

	// ���o���ʂ̕`��
	void draw_detectPose(const glm::mat4 &poseMatrix);

	// �w�i�̕`��
	void draw_background();

	// ���e�p�X���b�h�̐���
	void createProjectionThread();

	// ���o�p�X���b�h�̐���
	void createDetectionThread();

	// �ǐ՗p�X���b�h�̐���
	void createTrackingThread();

	// �J�����p�����[�^�̓ǂݍ���
	bool loadCameraParam(const std::string& camFile);

	// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
	void swapBuffers();

	// �R�[���o�b�N����
	static void ResizeCB(GLFWwindow *const window, int width, int height);
	static void MouseButtonCB(GLFWwindow *const window, int button, int action, int mods);
	static void MousePosCB(GLFWwindow *const window, double x, double y);
	static void MouseScrollCB(GLFWwindow *window, double x, double y);
	static void KeyFunCB(GLFWwindow *window, int key, int scancode, int action, int mods);

	// ����R�[���o�b�N�֐�
	static void TW_CALL SetQuatNormalizeCB(const void *value, void *clientData);
	static void TW_CALL GetQuatNormalizeCB(void *value, void *clientData);
	static void TW_CALL SetCalibCB(const void *value, void* clientData);
	static void TW_CALL GetCalibCB(void *value, void* clientData);
	static void TW_CALL SetDetectParamCB(void* clientData);
	static void TW_CALL SetTrackingParamCB(void* clientData);

	/***** �����o�ϐ� *****/

	///// GUI�p /////
	TwBar *myGUI;					// GUI
	TwBar *calibGUI;				
	TwBar *detectGUI;	
	TwBar *trackingGUI;	



	///// CAD���f���p /////
	GLuint program;					// �v���O�����I�u�W�F�N�g
	// �V�F�[�_��uniform�ϐ���ID
	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint LightID;
	GLuint LightPowerID;
	Mesh mesh;						// ���f���̃��b�V��

	glm::vec3 model_trans;			// �����ʒu
	glm::quat model_pose;			// �����p��
	std::string modelFilePath;		// ���f���̃t�@�C����


	///// �J�����摜�`��p /////
	Background camera_background;	// �w�i�̕`��


	///// GL�I�u�W�F�N�g�p /////
	Camera camera;					// �J����
	glm::vec4 background_color;		// �w�i�F
	glm::vec3 lightPos;				// �����ʒu
	float lightPower;			// �����̋���

	bool use_calib_flag;		// �����p�����[�^�̎g�p�t���O
	cv::Matx33f camMat;			// �J�����̓����p�����[�^
	glm::mat4 cameraMat;		// �J�����̓����p�����[�^

	// �O��̃}�E�X�ʒu
	double lastMousePosX;
	double lastMousePosY;
	bool mousePress_flag;			// �}�E�X��������Ă��邩

	MyTimer FPS;					// fps�v��


	///// �J�����f�o�C�X /////
	CamDev::CameraLinkCamera *irCamDev;		// IR�J����
	CamDev::Camera *rgbCamDev;				// �v���W�F�N�^�L�����u���[�V�����p�̃J����


	///// �X���b�h�Ԃ̋��L�N���X�p //////
	std::unique_ptr<CriticalSection> critical_section;

	///// ���e�X���b�h�p /////
	boost::thread projThread;
	int disp_num;
	bool use_chessboard;
	int cornerCol;
	int cornerRow;
	float cornerInterval_m;
	double graycode_delay;
	std::string saveCalibFolder;					// �L�����u���[�V�������ʂ�ۑ�����t�H���_
	boost::atomic<bool> projThread_end_flag;		// �X���b�h�I���t���O


	///// ���o�X���b�h�p /////
	boost::thread detectThread;
	boost::atomic<bool> detection_flag;				// ���o�J�n�t���O
	boost::atomic<bool> detectThread_end_flag;		// �X���b�h�I���t���O
	std::string fernsParamFile;						// Hough Ferns�̃p�����[�^�t�@�C��
	std::string lerningDataFolder;					// �w�K�f�[�^�̂���t�H���_��
	
	// ���o�p�����[�^
	float min_distance1;				// 1�w�ڂ̍l������ŏ�����
	float max_distance1;				// 1�w�ڂ̍l������ő勗��
	int distance_step;					// 1�w�ڂ̋����ω��̃X�e�b�v��
	float min_scale2;					// 2�w�ڂ̍l������ŏ��T�C�Y
	float max_scale2;					// 2�w�ڂ̍l������ő�T�C�Y
	int scale_step;						// 2�w�ڂ̃X�P�[���̃X�e�b�v��
	int detect_width;					// 2�w�ڂ̌��o�����`�̈�
	int detect_height;					// 2�w�ڂ̌��o�����`�̈�
	float grad_th;						// �P�x���z��臒l
	int meanshift_radius;				// Mean Shift�̃J�[�l����
	int win_th;							// Nearest Neighbor�œ�������T�C�Y
	double likelihood_th;				// ���[�ɂ��ޓx��臒l



	///// �ǐՃX���b�h�p /////
	boost::thread trackingThread;
	boost::atomic<bool> ready_tracking_flag;		// �������������t���O
	boost::atomic<bool> trackingThread_end_flag;	// �X���b�h�I���t���O
	std::string trackingFilePath;					// �ǐ՗p�̃��f���t�@�C��

	// �ǐՃp�����[�^
	float find_distance;				// �T�����鋗��(m)	
	float error_th;						// �G���[��臒l
	float edge_th1;						// �G�b�W��臒l1
	float edge_th2;						// �G�b�W��臒l2
	double trackingTime;				// �ǐՏ�������
	double delayTime;					// �V�X�e���x������
};


#endif