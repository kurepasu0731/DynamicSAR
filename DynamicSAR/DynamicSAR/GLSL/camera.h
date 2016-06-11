#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>


/**
 * @brief   �J�����N���X
 *
 * @note	���f���r���[�s��Ɠ������e�s�������
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

	// �l�̑��
	inline void setEyePosition(glm::vec3 eyePos) { eyePosition = eyePos; }
	inline void setEyeVector(glm::vec3 eyeVec) { eyeVector = eyeVec; }
	inline void setEyeUp(glm::vec3 eyeUP) { eyeUp = eyeUP; }
	inline void setFov(float fov) { FoV = fov; }
	inline void setNear(float n) { Near = n; }
	inline void setFar(float f) { Far = f; }

	// �l�̎擾
	inline glm::vec3 getEyePosition() { return eyePosition; }
	inline glm::vec3 getEyeVector() { return eyeVector; }
	inline glm::vec3 getEyeUp() { return eyeUp; }
	inline float getFov() { return FoV; }
	inline float getNear() { return Near; }
	inline float getFar() { return Far; }
	inline glm::mat4 getViewMatrix(){ return ViewMatrix; }
	inline glm::mat4 getProjectionMatrix(){	return ProjectionMatrix; }


	// �������e�ϊ��s��̍X�V
	inline glm::mat4 updateProjectionMatrix(float fov, float n, float f, int width, int height)
	{
		return ProjectionMatrix = glm::perspective(glm::radians(fov), (float)width / (float)height, n, f);
	}

	// �J�����ʒu�p���̍X�V
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


	// �}�E�X�̈ړ��ʂ��王�_�̉�]
	void computeRotationFromInputs(double difX, double difY, float speed)
	{
		horizontalAngle += speed * (float)difX;
		verticalAngle += speed * (float)difY;

		// ����ł̕���
		glm::vec3 direction( cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle) );

		// �E�����̃x�N�g��
		glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.1415f/2.0f), 0, cos(horizontalAngle - 3.1415f/2.0f)	);

		// ������̃x�N�g��
		eyeUp = glm::cross( right, direction );

		// �����x�N�g��
		eyeVector = eyePosition + direction;

		// �J�����̎p��
		ViewMatrix = glm::lookAt( eyePosition, eyePosition + direction, eyeUp);
	}



	// �J�����̈ʒu
	glm::vec3 eyePosition;
	// �J�����̎����x�N�g��
	glm::vec3 eyeVector;
	// �J�����̏����
	glm::vec3 eyeUp;
	// Field of View
	float FoV;
	// �J������near
	float Near;
	// �J������far
	float Far;

	glm::mat4 ViewMatrix;			// �J�����̈ʒu�p��
	glm::mat4 ProjectionMatrix;		// �������e�ϊ��s��

	float horizontalAngle;			// ���������̉�]
	float verticalAngle;			// ���������̉�]
};

#endif