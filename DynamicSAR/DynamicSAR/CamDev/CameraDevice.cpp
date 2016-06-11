#include "CameraDevice.h"


namespace CamDev
{
	Camera::~Camera()
	{
	}

	bool Camera::captureImage(cv::Mat &dst)
	{
		return getImage(dst);
	}
}