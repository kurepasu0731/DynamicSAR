#ifndef TRACKING_THREAD_H
#define TRACKING_THREAD_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <TooN/TooN.h>
#include <TooN/SVD.h>
#include <TooN/so3.h>
#include <TooN/se3.h>
#include <TooN/wls.h>

#include "GLSL/glsl_setup.h"
#include "GLSL/window.h"
#include "GLSL/camera.h"
#include "GLSL/mesh.h"
#include "GLSL/texture.h"
#include "GLSL/background.h"
#include "GLSL/offscreenRender.h"
#include "GLSL/image_rect.h"
#include "GLSL/myTimer.h"

#include "CriticalSection.h"

#include "CamDev/WebCamera.h"
#include "CamDev/CameraLinkCamera.h"
#include "CamDev/UndistortedCamera.h"

#include "LSM/LSMPoint3f.h"
#include "LSM/LSMQuatd.h"

/**
 * @brief   �g���b�L���O�X���b�h
 *			�I�t�X�N���[�������_�����O�ŃG�b�W���o
 */
class TrackingThread : public Window
{
public:

	// 3D���f���Ɠ��͉摜�Ƃ̑Ή�
	struct Correspond
	{
		cv::Point3f model3D;		// 3D���f����3�����_�̈ʒu
		cv::Point2f model2D;		// 3D���f����2�����_�̈ʒu
		cv::Point2f normal2D;		// 3D���f����2�����_�ɂ�����@���x�N�g��
		cv::Point2f normalUnit;		// 3D���f����2�����_�ɂ�����@���̒P�ʃx�N�g��
		cv::Point2f imageEdge2D;	// �Ή��t����ꂽ���͉摜�̃G�b�W�̈ʒu
		double dist;				// �Ή��_�Ԃ̋���
	};


	// �R���X�g���N�^
	TrackingThread(CriticalSection *cs, CamDev::CameraLinkCamera *cameraDevice, int width = 640, int height = 480, const char *title = "Edge image")
		: Window(width, height, title)
		, windowWidth (width)
		, windowHeight (height)
		, lightPos (0.0, 0.0, 0.0)			// �Ɩ��̏����ʒu(���_)
		, edge_th1 (60.f)
		, edge_th2 (100.f)
		, find_distance (0.03f)
		, error_th (0.45f)
		, trackingTime (14.0)
		, delayTime (100.0)
		, firstTime (true)
	{
		// �f�o�C�X�̎󂯓n��
		irCamDev = cameraDevice;

		// �X���b�h�ԋ��L�N���X
		critical_section = cs;

		// �J�����̏����ʒu(�J���������_)
		camera.setEyePosition(glm::vec3(0.0, 0.0, 0.0));

		// �J�����̎�������
		camera.setEyeVector(glm::vec3(0.0, 0.0, -1.0));

		// �Ɩ��̏����ʒu(���_)
		lightPos = glm::vec3(0.0, 0.0, 0.0);

		// �I�t�X�N���[�������ݒ�
		offscreen1.offscreen_width = width;
		offscreen1.offscreen_height = height;
		offscreen2.offscreen_width = width;
		offscreen2.offscreen_height = height;
		offscreen3.offscreen_width = width;
		offscreen3.offscreen_height = height;
		offscreen4.offscreen_width = width;
		offscreen4.offscreen_height = height;

		// �J�����̍X�V
		camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);

		// �����U�̏�����
		cv::Mat cov = cv::Mat::eye(6, 6, CV_32F);
		covariance = cov;

		// �\���t�B���^
		predict_point = std::unique_ptr<LSMPoint3f> (new LSMPoint3f(LSM));
		predict_quat = std::unique_ptr<LSMQuatd> (new LSMQuatd(LSM));
		predict_point->setForgetFactor(0.6);	// �Y�p�W��
		predict_quat->setForgetFactor(0.6);	// �Y�p�W��

		// �x���⏞
		compensate_point = std::unique_ptr<LSMPoint3f> (new LSMPoint3f(LSM));
		compensate_quat = std::unique_ptr<LSMQuatd> (new LSMQuatd(LSM));
		compensate_point->setForgetFactor(0.6);	// �Y�p�W��
		compensate_quat->setForgetFactor(0.6);	// �Y�p�W��

	}


	// �f�X�g���N�^
	virtual ~TrackingThread(){}


	// ��������
	void init(const std::string& modelFile, const std::string& trackingFile, const cv::Matx33f& _camMat, const glm::mat4& _cameraMat);

	// ���t���[���s������
	void display();

	// ���f���x�[�X�̃g���b�L���O
	void tracking();

	// �I�t�X�N���[���ł̕`��
	void offscreen_render(int width, int height);

	// 3D���f���̗֊s���̌��o
	void extractModelEdge();

	// ���͉摜�̃G�b�W���o
	void extractInputEdge();

	// ���͉摜�Ƃ̑Ή��𐄒�
	void estimateCorrespondence();

	// ���R�r�A���̌v�Z
	TooN::Vector<6> calcJacobian(const cv::Point3f& pts3, const cv::Point2f& pts2, const cv::Point2f& ptsnv, double ptsd, const TooN::SE3& E);

	// IRLS�ɂ��p������
	void estimatePoseIRLS();

	// �G���[���̐���
	void calcError();

	// �`��V�[��(�����f��)
	void scene_orig(int width, int height);

	// �`��V�[��(�g���b�L���O���f��)
	void scene_point(int width, int height);

	// �`��V�[��(���͉摜)
	void scene_background(int width, int height);

	// ���_�̕`��
	void draw_point(const std::vector<glm::vec3>& point_cloud, int width, int height);

	// �����p���̃Z�b�g
	void setInitPose();

	// 2�������ʂւ̓��e
	void projection3Dto2D(const cv::Vec3f& point3D, const cv::Vec3f& normal3D, cv::Vec2f& point2D, cv::Vec2f& normal2D);

	// PLY�`���œ_�Q�̓ǂݍ���
	void loadPointCloudPly(const std::string& fileName, std::vector<glm::vec3>& point_cloud, std::vector<glm::vec3>& point_normal);

	// �����񕪊�
	std::vector<std::string> split(const std::string &str, char sep);

	// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
	void swapBuffers();


	/***** �����o�ϐ� *****/

	///// ����CAD���f���p /////
	GLuint program_orig;		// �v���O�����I�u�W�F�N�g

	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint LightID;
	GLuint LightPowerID;

	Mesh orig_mesh;				// ���f���̃��b�V��


	///// CAD���f���G�b�W���o�p /////
	GLuint program_CADedge;		// �v���O�����I�u�W�F�N�g
	GLint edge_locW;				// �V�F�[�_�p�̉摜�̕�
	GLint edge_locH;				// �V�F�[�_�p�̉摜�̍���
	GLuint edge_MatrixID;
	GLuint edge_texID;				// �e�N�X�`��ID
	ImageRect edge_rect;		// �V�F�[�_�G�t�F�N�g�p�̃��b�V��


	///// �g���b�L���O���f���p /////
	GLuint program_point;		// �v���O�����I�u�W�F�N�g
	GLuint pointMatrixID;
	GLuint pointViewMatrixID;
	GLuint pointModelMatrixID;
	GLuint pointVBO[3];
	GLuint pointIBO;

	// �|�C���g�N���E�h
	std::vector<glm::vec3> pointCloud;		// ���_
	std::vector<glm::vec3> pointNormal;		// �@��


	///// �V�F�[�_�G�t�F�N�g�p /////
	GLuint program_image;		// �v���O�����I�u�W�F�N�g
	GLint locW;					// �V�F�[�_�p�̉摜�̕�
	GLint locH;					// �V�F�[�_�p�̉摜�̍���
	GLuint imageMatrixID;
	GLuint tex0ID;				// �e�N�X�`��ID
	GLuint tex1ID;				// �e�N�X�`��ID
	GLuint tex2ID;				// �e�N�X�`��ID
	GLuint tex3ID;				// �e�N�X�`��ID

	cv::Mat indexImg;			// �C���f�b�N�X�ԍ����i�[����Ă���
	ImageRect image_rect;		// �V�F�[�_�G�t�F�N�g�p�̃��b�V��


	///// �G�b�W�`��p /////
	std::vector<glm::vec3> pointEdge;
	GLuint program_edge;
	GLuint pMatrixID;
	GLuint psizeID;
	GLuint pVBO;
	GLuint pIBO;


	///// GL�I�u�W�F�N�g /////
	int windowWidth;
	int windowHeight;
	Camera camera;				// �J����
	glm::vec3 lightPos;			// �����ʒu
	float lightPower;			// �����̋���

	cv::Matx33f camMat;			// �J�����̓����p�����[�^
	glm::mat4 cameraMat;		// �J�����̓����p�����[�^

	OffscreenRender offscreen1;	// �I�t�X�N���[�������_�����O
	OffscreenRender offscreen2;	// �I�t�X�N���[�������_�����O
	OffscreenRender offscreen3;	// �I�t�X�N���[�������_�����O
	OffscreenRender offscreen4;	// �I�t�X�N���[�������_�����O

	///// �g���b�L���O�p /////
	cv::Matx44f estimatedPose;	// ���茋��
	cv::Matx66f covariance;		// �����U
	float find_distance;		// �T�����鋗��(m)
	double max_distance;		// �l������ő�̋���
	float error_th;				// �G���[��臒l

	std::vector<Correspond> correspondPoints;		// �Ή��_

	///// �\���t�B���^�p /////
	double trackingTime;		// �g���b�L���O�ɂ����鎞��
	std::unique_ptr<LSMPoint3f> predict_point;	// �ʒu�̍ŏ����@
	std::unique_ptr<LSMQuatd> predict_quat;		// �p���̍ŏ����@
	bool firstTime;								// 1��ڂ��ǂ���

	///// �x���⏞�p /////
	MyTimer timer;				// �N�����Ԃ���̎��Ԍv��
	double delayTime;			// �V�X�e���̒x������
	std::unique_ptr<LSMPoint3f> compensate_point;	// �ʒu�̍ŏ����@
	std::unique_ptr<LSMQuatd> compensate_quat;		// �p���̍ŏ����@


	///// ���͉摜�p /////
	CamDev::CameraLinkCamera *irCamDev;		// IR�J����
	cv::Mat inputImg;			// ���͉摜
	cv::Mat cannyImg;			// �G�b�W�摜
	cv::Mat edge_img;			// ���͉摜�ƃ��f���̃G�b�W�̑Ή��摜
	float edge_th1;				// �G�b�W��臒l1
	float edge_th2;				// �G�b�W��臒l2
	cv::Mat sobelImgX;			// sobel���o
	cv::Mat sobelImgY;			// sobel���o

	Background input_mesh;		// ���͉摜�p�̃��b�V��

	///// �X���b�h�Ԃ̋��L�N���X�p //////
	CriticalSection* critical_section;
};


#endif