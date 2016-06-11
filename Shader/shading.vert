#version 330 core

// ���_���ɈقȂ�l�����
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

// �t���O�����g�V�F�[�_�֏o��
out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

// CPU�����l�Ȓl�Ƃ��ē���
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;

void main()
{

	// �N���b�v���W�ł̒��_�ʒu : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// ���[���h���W�ł̒��_�ʒu : M * position
	Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	
	// �J�������W�ł̒��_�ʒu
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	// �J�������W�ɂ�����J�����ʒu�� (0,0,0)
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// �J�������W�ɂ���������ʒu (M�͒P�ʍs��̂��߁C�Ȃ���)
	vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	
	// �J�������W�ɂ�����@��
	Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; 
	
	// ���_�ɂ�����UV���W
	UV = vertexUV;
}

