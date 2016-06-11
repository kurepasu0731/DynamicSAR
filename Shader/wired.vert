#version 330 core

// ���_���ɈقȂ�l�����
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;


// CPU�����l�Ȓl�Ƃ��ē���
uniform mat4 MVP;


void main()
{

	// �N���b�v���W�ł̒��_�ʒu : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
}

