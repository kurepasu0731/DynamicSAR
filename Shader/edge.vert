#version 330 core

// ���_���ɈقȂ�l�����
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec3 index;

// CPU�����l�Ȓl�Ƃ��ē���
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform float point_size = 1.0f;

flat out vec3 model_index;		// ���_�̃C���f�b�N�X

void main()
{
	model_index = index;

	// �J�������W�ł̒��_�ʒu
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	// �J�������W�ɂ�����J�����ʒu�� (0,0,0)
	vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// �J�������W�ł̖@��
	vec3 n = normalize( (V * M * vec4(vertexNormal_modelspace,0)).xyz );
	// �����x�N�g��
	vec3 E = normalize( EyeDirection_cameraspace );
	// ���̖̂@���Ǝ����x�N�g����cosine (0�ȏ�ɌŒ�)
	float cosAlpha = dot( n,E );

	//if ( cosAlpha > -0.06 && cosAlpha < 0.06) {
		// �N���b�v���W�ł̒��_�ʒu : MVP * position
		gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	//}else {
	//	gl_Position = vec4(-100,0,0,0);
	//}

	// ���_�̑傫����ύX����
	gl_PointSize = point_size / gl_Position.w;  
}

