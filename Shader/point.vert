#version 330 core

// ���_���ɈقȂ�l�����
layout(location = 0) in vec3 vertexPosition_modelspace;


// CPU�����l�Ȓl�Ƃ��ē���
uniform mat4 MVP;
uniform float point_size = 1.0f;

void main()
{
	// �N���b�v���W�ł̒��_�ʒu : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// ���_�̑傫����ύX����
	gl_PointSize = point_size / gl_Position.w;  
}

