#version 330 core

layout(location = 0) in vec3 vertex;
out vec2 pos;			// �t���O�����g�V�F�[�_�ɓn����f�̈ʒu

uniform mat4 MVP;

void main(void)
{

	// 2�����̉摜�̈ʒu
	pos = vec2(vertex.xy);

	// �摜�̈ʒu�����̂܂܏o��
	gl_Position =  MVP * vec4(vertex,1);
}
