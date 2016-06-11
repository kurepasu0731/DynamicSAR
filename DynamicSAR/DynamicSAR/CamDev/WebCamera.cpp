#include "WebCamera.h"


namespace CamDev
{
	WebCamera::WebCamera(int cameraIndex, int width, int height)
	{
		cap = new cv::VideoCapture(cameraIndex);

		if (width != -1)
		{
			// ‰ð‘œ“x‚ÌÝ’è
			cap->set(CV_CAP_PROP_FRAME_WIDTH, width);
			cap->set(CV_CAP_PROP_FRAME_HEIGHT, height);
		}
	}

	WebCamera::~WebCamera()
	{
		delete cap;
	}

	bool WebCamera::getImage(cv::Mat &dst)
	{
		if(!cap->isOpened()) {
			return false;
		}

		*cap >> dst;
		dst = dst.clone();

		return true;
	}
}