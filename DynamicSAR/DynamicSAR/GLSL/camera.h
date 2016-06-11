#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>


/**
 * @brief   カメラクラス
 *
 * @note	モデルビュー行列と透視投影行列を扱う
 */
class Camera
{

public:
	Camera()
		: FoV (45.0f)
		, Near (0.1f)
		, Far (100.0f)
		, horizontalAngle (3.1415f)
		, verticalAngle (0.0f)
	{
		eyePosition = glm::vec3( 0, 0, 5 );
		eyeVector = glm::vec3( 0, 0, 0);
		eyeUp = glm::vec3( 0, 1, 0);

		updateViewMatrix(eyePosition, eyeVector, eyeUp);
	}

	// 値の代入
	inline void setEyePosition(glm::vec3 eyePos) { eyePosition = eyePos; }
	inline void setEyeVector(glm::vec3 eyeVec) { eyeVector = eyeVec; }
	inline void setEyeUp(glm::vec3 eyeUP) { eyeUp = eyeUP; }
	inline void setFov(float fov) { FoV = fov; }
	inline void setNear(float n) { Near = n; }
	inline void setFar(float f) { Far = f; }

	// 値の取得
	inline glm::vec3 getEyePosition() { return eyePosition; }
	inline glm::vec3 getEyeVector() { return eyeVector; }
	inline glm::vec3 getEyeUp() { return eyeUp; }
	inline float getFov() { return FoV; }
	inline float getNear() { return Near; }
	inline float getFar() { return Far; }
	inline glm::mat4 getViewMatrix(){ return ViewMatrix; }
	inline glm::mat4 getProjectionMatrix(){	return ProjectionMatrix; }


	// 透視投影変換行列の更新
	inline glm::mat4 updateProjectionMatrix(float fov, float n, float f, int width, int height)
	{
		return ProjectionMatrix = glm::perspective(glm::radians(fov), (float)width / (float)height, n, f);
	}

	// カメラ位置姿勢の更新
	inline glm::mat4 updateViewMatrix(glm::vec3 position, glm::vec3 vector, glm::vec3 up)
	{
		glm::vec3 const f(glm::normalize(vector - position));
		glm::vec3 s0(glm::cross(f, up));
		if (s0 == glm::vec3(0.0f,0.0f,0.0f))
		{
			s0 = glm::vec3(0.0f,0.0f,-1.0f);
		}
		glm::vec3 const s(glm::normalize(s0));
		glm::vec3 const u(glm::cross(s, f));

		ViewMatrix[0][0] = s.x;
		ViewMatrix[1][0] = s.y;
		ViewMatrix[2][0] = s.z;
		ViewMatrix[0][1] = u.x;
		ViewMatrix[1][1] = u.y;
		ViewMatrix[2][1] = u.z;
		ViewMatrix[0][2] =-f.x;
		ViewMatrix[1][2] =-f.y;
		ViewMatrix[2][2] =-f.z;
		ViewMatrix[3][0] =-glm::dot(s, position);
		ViewMatrix[3][1] =-glm::dot(u, position);
		ViewMatrix[3][2] = glm::dot(f, position);

		return ViewMatrix;
	}


	// マウスの移動量から視点の回転
	void computeRotationFromInputs(double difX, double difY, float speed)
	{
		horizontalAngle += speed * (float)difX;
		verticalAngle += speed * (float)difY;

		// 球上での方向
		glm::vec3 direction( cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle) );

		// 右方向のベクトル
		glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.1415f/2.0f), 0, cos(horizontalAngle - 3.1415f/2.0f)	);

		// 上方向のベクトル
		eyeUp = glm::cross( right, direction );

		// 視線ベクトル
		eyeVector = eyePosition + direction;

		// カメラの姿勢
		ViewMatrix = glm::lookAt( eyePosition, eyePosition + direction, eyeUp);
	}



	// カメラの位置
	glm::vec3 eyePosition;
	// カメラの視線ベクトル
	glm::vec3 eyeVector;
	// カメラの上方向
	glm::vec3 eyeUp;
	// Field of View
	float FoV;
	// カメラのnear
	float Near;
	// カメラのfar
	float Far;

	glm::mat4 ViewMatrix;			// カメラの位置姿勢
	glm::mat4 ProjectionMatrix;		// 透視投影変換行列

	float horizontalAngle;			// 水平方向の回転
	float verticalAngle;			// 垂直方向の回転
};

#endif