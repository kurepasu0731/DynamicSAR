#ifndef PROJECTION_THREAD_H
#define PROJECTION_THREAD_H

#include <memory>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "GLSL/window.h"
#include "GLSL/background.h"
#include "GLSL/camera.h"
#include "GLSL/mesh.h"
#include "GLSL/texture.h"
#include "GLSL/offscreenRender.h"

#include "Calibration/Calibration.h"

#include "CriticalSection.h"

// �S��ʕ\���p
namespace Monitor
{

	typedef struct disp_prop
	{
		int index;
		int x,y,width,height;
	} Disp_Prop;

	static int dispCount=-1;
	static std::vector<Disp_Prop> Disps_Prop;

	//�f�B�X�v���C�̏�����
	inline BOOL CALLBACK DispEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData ) 
	{
		Disp_Prop di;
		di.index = dispCount++;
		di.x = lprcMonitor->left;
		di.y = lprcMonitor->top;
		di.width = lprcMonitor->right - di.x;
		di.height = lprcMonitor->bottom - di.y;
		Disps_Prop.push_back(di);

		return TRUE; // TRUE�͒T���p���CFALSE�ŏI��
	}

	//�f�B�X�v���C���o
	inline void SearchDisplay(void) 
	{
		// ��x�������s����
		if (dispCount == -1) {
			dispCount = 0;
			Disps_Prop = std::vector<Disp_Prop>();
			EnumDisplayMonitors(NULL, NULL, DispEnumProc, 0);
			Sleep(200);
		}
	}
}



// ���e�X���b�h�N���X
class ProjectionThread : public Window
{

public:
	// �R���X�g���N�^
	ProjectionThread(CriticalSection *cs, CamDev::CameraLinkCamera *cameraDevice, CamDev::Camera *RGBcameraDevice = NULL, bool _use_chessboard=false, int _cornerCol=4, int _cornerRow=11, float _cornerInterval_m=0.0225f, double _delay=80.0, std::string _saveCalibFolder=".", int width=640, int height=480, const char *title = "Projection")
		: Window( width, height, title)
		, calib_able_flag (false)
		, lightPower(0.7f)
	{
		// �f�o�C�X�̎󂯓n��
		irCamDev = cameraDevice;
		rgbCamDev = RGBcameraDevice;

		// �ۑ��t�H���_
		saveCalibFolder = _saveCalibFolder;

		// �J�������ڑ�����Ă�����L�����u���[�V�����\
		if (rgbCamDev != NULL)
		{
			cv::Mat img;
			if (rgbCamDev->captureImage(img)) 
			{
				calib = std::unique_ptr<Calibration> (new Calibration(rgbCamDev, width, height, img.cols, img.rows, _use_chessboard, _cornerCol, _cornerRow, _cornerInterval_m, _delay, saveCalibFolder));
				calib_able_flag = true;
			}		
		}

		// �X���b�h�ԋ��L�N���X
		critical_section = cs;

		// �p���̏�����
		quatOrientation = glm::quat(1.0, 0.0, 0.0, 0.0);

		// �Ɩ��̏����ʒu
		lightPos = glm::vec3(0.0, 0.0, 1.0);

		// �I�t�X�N���[�������ݒ�
		offscreen.offscreen_width = width;
		offscreen.offscreen_height = height;

		// �E�B���h�E�̐�������off
		glfwSwapInterval(0);

		// �J�����̍X�V
		camera.setEyePosition(glm::vec3(0.0f, 0.0f, 1.5f));
		camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);

		// �C���X�^���X��this�|�C���^���L�^
		glfwSetWindowUserPointer(window, this);
	}

	// �f�X�g���N�^
	virtual ~ProjectionThread(){}

	// ��������
	void init(const std::string& modelFile);

	// ���t���[���s������
	void display();

	// ���f���̕`��
	void draw_model();

	// �摜�̓��e
	void img_projection(const cv::Mat& projMat);

	// ���_�̕`��
	void draw_point(const std::vector<glm::vec3>& point_cloud);

	// �L�����u���[�V�������ʂ�p���ă`�F�b�J�[�{�[�h�ɍē��e
	void runCheckerReprojection();

	// �v���W�F�N�^�p�����[�^�̓ǂݍ���
	void loadProjectorParam();

	// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
	void swapBuffers();



	/***** �����o�ϐ� *****/

	///// CAD���f���p /////
	GLuint program;					// �v���O�����I�u�W�F�N�g
	// �V�F�[�_��uniform�ϐ���ID
	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint LightID;
	GLuint LightPowerID;
	Mesh mesh;						// ���f���̃��b�V��

	glm::mat4 ProjectionMatrix;		// �������e
	glm::mat4 ViewMatrix;			// �J�����s��
	glm::quat quatOrientation;		// ���f���̎p��


	///// ���e�摜�`��p /////
	Background projection_background;		


	///// �L�����u���[�V�����m�F�p�̓_�Q /////
	GLuint program_point;					// ���_�̂ݕ`��p
	std::vector<glm::vec3> checkerPoint;	// ���_
	std::vector<glm::vec3> checkerNormal;	// �@��
	GLuint PointMatrixID;
	GLuint PointVBO;						// �_�̕`��p
	GLuint PointIBO;						// �_�̕`��p
	Background camera_background;			// �J�����m�F�p
	OffscreenRender camera_offscreen;


	///// GL�I�u�W�F�N�g�p /////
	Camera camera;					// �J����
	glm::vec3 lightPos;				// �����ʒu
	float lightPower;				// �����̋���

	OffscreenRender offscreen;		// �I�t�X�N���[�������_�����O


	///// �v���W�F�N�^�p�����[�^�p /////
	glm::mat4 proj_extrinsicMat;			// �v���W�F�N�^�̈ʒu�p��
	glm::mat4 proj_intrinsicMat;			// �J�����̓����p�����[�^
	cv::Matx33f proj_intrinsic;				// �J�����̓����p�����[�^
	glm::mat4 projection;


	///// ProCam�L�����u���[�V�����p /////
	std::unique_ptr<Calibration> calib;
	std::string saveCalibFolder;				// �L�����u���[�V�����t�H���_
	CamDev::CameraLinkCamera *irCamDev;			// IR�J����
	CamDev::Camera *rgbCamDev;					// ���e���B�e�p�f�o�C�X
	bool calib_able_flag;						// �L�����u���[�V�������\������


	///// �X���b�h�Ԃ̋��L�N���X�p //////
	CriticalSection* critical_section;
};



#endif