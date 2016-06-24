#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H


#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/opencv.hpp>


/**
 * @brief	�}���`�X���b�h�Ԃ̕ϐ��̋��L�N���X\n
 * 			�N���e�B�J���Z�N�V�����̊m��
 */
class CriticalSection
{
public:

	CriticalSection()
		: graycode_flag (false)
		, calib_flag (false)
		, load_calib_flag (false)
		, run_reprojection_flag (false)
		, use_calib_flag (false)
		, use_projCalib_flag (false)
		, project_white_flag (false)
		, ready_detect_flag (false)
		, tracking_flag (false)
		, predict_flag (true)
		, detect_tracking_flag (false)
		, tracking_success_flag (false)
		, compensation_delay_flag (true)
		, lightPower (0.7f)
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
		, edge_th1 (60.f)
		, edge_th2 (100.f)
		, find_distance (0.05f)
		, error_th (0.75f)
		, trackingTime (24.0)
		, delayTime (120.0)
		//**written by fujisawa**//
		, movie_flag(false)
		, movieFileName("movie02.avi")
		, reloadMovie_flag(false)
		//, textureFileName("test.png") //Debug�p
		//***********************//

	{}

	virtual ~CriticalSection(){}

	// ���f���̍��W�ϊ��̃Z�b�g
	inline void setModelMatrix(const glm::mat4& _model_matrix)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(model_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		model_matrix = _model_matrix;
	}

	// �����̋����̃Z�b�g
	inline void setLightPower(const float& _lightPower)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(light_power_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		lightPower = _lightPower;
	}


	// �J�����̓����p�����[�^�̃Z�b�g(CV)
	inline void setCameraIntrinsicCV(const cv::Matx33f& _camera_matrix)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(camera_intrinsic_cv_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		cameraIntrinsicCV = _camera_matrix;
	}

	// �J�����̓����p�����[�^�̃Z�b�g
	inline void setCameraIntrinsicMatrix(const glm::mat4& _camera_matrix)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(camera_intrinsic_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		cameraIntrinsicMatrix = _camera_matrix;
	}

	// �J�����c�ݕ␳�}�b�v�̃Z�b�g
	inline void setUndistortMap(const cv::Mat& map1, const cv::Mat& map2)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(undistortMap_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		undistortMap1 = map1.clone();
		undistortMap2 = map2.clone();
	}

	// �v���W�F�N�^�c�ݕ␳�}�b�v�̃Z�b�g
	inline void setProjUndistortMap(const cv::Mat& map1, const cv::Mat& map2)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(projUndistortMap_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		projUndistortMap1 = map1.clone();
		projUndistortMap2 = map2.clone();
	}

	// ���o�����ʒu�p���̃Z�b�g
	inline void setDetectPoseMatrix(const std::vector<glm::mat4>& _detectPose)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(detectPose_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		
		detectPose.clear();
		detectPose.assign(_detectPose.begin(), _detectPose.end());
	}

	// ���o�ɗp����p�����[�^�̃Z�b�g
	inline void setDetectParams(float _min_distance1, float _max_distance1, int _distance_step, float _min_scale2, float _max_scale2, int _scale_step,
								int _detect_width, int _detect_height, float _grad_th, int _meanshift_radius, int _win_th, double _likelihood_th)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(detect_params_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		min_distance1 = _min_distance1;
		max_distance1 = _max_distance1;
		distance_step = _distance_step;
		min_scale2 = _min_scale2;
		max_scale2 = _max_scale2;
		scale_step = _scale_step;
		detect_width = _detect_width;
		detect_height = _detect_height;
		grad_th = _grad_th;
		meanshift_radius = _meanshift_radius;
		win_th = _win_th;
		likelihood_th = _likelihood_th;
	}

	// �ǐՒ��̈ʒu�p���̃Z�b�g
	inline void setTrackingPoseMatrix(const glm::mat4& _trackingPose)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(trackingPose_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		
		trackingPose = _trackingPose;
	}

	// �x���⏞�����ʒu�p���̃Z�b�g
	inline void setCompensatePoseMatrix(const glm::mat4& _predicctPose)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(compensatePose_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		
		compensatePose = _predicctPose;
	}

	// �ǐՂɗp����p�����[�^�̃Z�b�g
	inline void setTrackingParams(float _find_distance, float _error_th, float _edge_th1, float _edge_th2, double _trackingTime, double _delayTime)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(tracking_params_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		find_distance = _find_distance;
		edge_th1 = _edge_th1;
		edge_th1 = _edge_th1;
		edge_th2 = _edge_th2;
		trackingTime = _trackingTime;
		delayTime = _delayTime;
	}


	// ���f���̍��W�ϊ��̎擾
	inline glm::mat4 getModelMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(model_matrix_mtx);
		return model_matrix;
	}

	// �����̋����̎擾
	inline float getLightPower()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(light_power_mtx);
		return lightPower;
	}

	// �J�����̓����p�����[�^�̎擾
	inline cv::Matx33f getCameraIntrinsicCV()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(camera_intrinsic_cv_mtx);
		return cameraIntrinsicCV;
	}

	// �J�����̓����p�����[�^�̎擾
	inline glm::mat4 getCameraIntrinsicMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(camera_intrinsic_matrix_mtx);
		return cameraIntrinsicMatrix;
	}

	// �J�����c�ݕ␳�}�b�v�̎擾
	inline void getUndistortMap(cv::Mat& map1, cv::Mat& map2)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(undistortMap_mtx);
		map1 = undistortMap1.clone();
		map2 = undistortMap2.clone();
	}

	// �J�����c�ݕ␳�}�b�v�̎擾
	inline void getProjUndistortMap(cv::Mat& map1, cv::Mat& map2)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(projUndistortMap_mtx);
		map1 = projUndistortMap1.clone();
		map2 = projUndistortMap2.clone();
	}

	// ���o�����ʒu�p���̎擾
	inline void getDetectPoseMatrix(std::vector<glm::mat4>& _detectPose)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(detectPose_matrix_mtx);
		_detectPose.clear();
		_detectPose.assign(detectPose.begin(), detectPose.end());
	}

	// ���o�ɗp����p�����[�^�̎擾
	inline void getDetectParams(float& _min_distance1, float& _max_distance1, int& _distance_step, float& _min_scale2, float& _max_scale2, int& _scale_step,
								int& _detect_width, int& _detect_height, float& _grad_th, int& _meanshift_radius, int& _win_th, double& _likelihood_th)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(detect_params_mtx);
		_min_distance1 = min_distance1;
		_max_distance1 = max_distance1;
		_distance_step = distance_step;
		_min_scale2 = min_scale2;
		_max_scale2 = max_scale2;
		_scale_step = scale_step;
		_detect_width = detect_width;
		_detect_height = detect_height;
		_grad_th = grad_th;
		_meanshift_radius = meanshift_radius;
		_win_th = win_th;
		_likelihood_th = likelihood_th;
	}

	// �ǐՒ��̈ʒu�p���̎擾
	inline glm::mat4 getTrackingPoseMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(trackingPose_matrix_mtx);
		return trackingPose;
	}

	// �x���⏞�����ʒu�p���̎擾
	inline glm::mat4 getCompensatePoseMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(compensatePose_matrix_mtx);
		return compensatePose;
	}

	// �ǐՂɗp����p�����[�^�̎擾
	inline void getTrackingParams(float& _find_distance, float& _error_th, float& _edge_th1, float& _edge_th2, double& _trackingTime, double& _delayTime)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(tracking_params_mtx);
		_find_distance = find_distance;
		_edge_th1 = edge_th1;
		_edge_th1 = edge_th1;
		_edge_th2 = edge_th2;
		_trackingTime = trackingTime;
		_delayTime = delayTime;
	}



	///// �t���O /////

	// �L�����u���[�V����
	boost::atomic<bool> graycode_flag;			// �O���C�R�[�h���e�t���O
	boost::atomic<bool> calib_flag;				// �L�����u���[�V�����t���O
	boost::atomic<bool> load_calib_flag;		// �L�����u���[�V�����t�@�C���̓ǂݍ��݃t���O
	boost::atomic<bool> run_reprojection_flag;	// �`�F�b�J�[�{�[�h�ɍē��e����t���O
	boost::atomic<bool> use_calib_flag;			// �J�����L�����u���[�V�����ς݂��m�F�t���O
	boost::atomic<bool> use_projCalib_flag;		// �v���W�F�N�^�L�����u���[�V�����ς݂��m�F�t���O
	boost::atomic<bool> project_white_flag;		// ���𓊉e����t���O

	// ���o
	boost::atomic<bool> ready_detect_flag;		// ���o�̏����m�F�t���O

	// �ǐ�
	boost::atomic<bool> tracking_flag;			// �ǐՊJ�n�t���O
	boost::atomic<bool> predict_flag;			// �\���t�B���^�t���O

	// ���o���ǐ�
	boost::atomic<bool> detect_tracking_flag;	// ���o���ǐՃt���O
	boost::atomic<bool> tracking_success_flag;	// �ǐՐ����t���O

	// �x���⏞
	boost::atomic<bool> compensation_delay_flag;	// �x���⏞�t���O

	//**written by fujisawa***************************************************//
	// ����
	boost::atomic<bool> movie_flag;				//����Đ�����/���Ȃ�
	boost::atomic<const char*> movieFileName;	//����t�@�C���ꏊ
	boost::atomic<bool> reloadMovie_flag;		//����̍ēǂݍ��݂���/���Ȃ�
	//boost::atomic<const char*> textureFileName; //�`�F���W����e�N�X�`���t�@�C���ꏊ //Debug�p
	//************************************************************************//

private:

	///// �~���[�e�b�N�X /////
	boost::shared_mutex model_matrix_mtx;
	boost::shared_mutex light_power_mtx;
	boost::shared_mutex camera_intrinsic_cv_mtx;
	boost::shared_mutex camera_intrinsic_matrix_mtx;
	boost::shared_mutex undistortMap_mtx;
	boost::shared_mutex projUndistortMap_mtx;
	boost::shared_mutex detectPose_matrix_mtx;
	boost::shared_mutex detect_params_mtx;
	boost::shared_mutex trackingPose_matrix_mtx;
	boost::shared_mutex compensatePose_matrix_mtx;
	boost::shared_mutex tracking_params_mtx;


	///// ���L�ϐ� /////
	glm::mat4 model_matrix;				// 3D���f���̍��W�ϊ�
	float lightPower;					// �����̋���

	// �L�����u���[�V����
	cv::Matx33f cameraIntrinsicCV;		// IR�J�����̓����p�����[�^(CV�p)
	glm::mat4 cameraIntrinsicMatrix;	// IR�J�����̓����p�����[�^
	cv::Mat undistortMap1;				// IR�J�����̘c�ݕ␳�}�b�v
	cv::Mat undistortMap2;				// IR�J�����̘c�ݕ␳�}�b�v
	cv::Mat projUndistortMap1;			// �v���W�F�N�^�̘c�ݕ␳�}�b�v
	cv::Mat projUndistortMap2;			// �v���W�F�N�^�̘c�ݕ␳�}�b�v

	// ���o
	std::vector<glm::mat4> detectPose;	// ���o�����ʒu�p��
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

	// �ǐ�
	glm::mat4 trackingPose;				// �ǐՒ��̈ʒu�p��
	glm::mat4 compensatePose;			// �x�����ԕ⏞�����ʒu�p��
	float find_distance;				// �T�����鋗��(m)	
	float error_th;						// �G���[��臒l
	float edge_th1;						// �G�b�W��臒l1
	float edge_th2;						// �G�b�W��臒l2
	double trackingTime;				// �ǐՏ�������
	double delayTime;					// �V�X�e���x������
};



#endif