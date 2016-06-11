#version 330 core

// ���_�V�F�[�_����̓���
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

// �o�̓f�[�^
out vec3 color;

// CPU�����l�Ȓl�Ƃ��ē���
uniform sampler2D myTextureSampler;		// �_�~�[�i�e�N�X�`���ԍ�0�j
uniform vec3 LightPosition_worldspace;
uniform float LightPower = 10.f;

void main()
{
	// �J�������W�ł̖@��
	vec3 n = normalize( Normal_cameraspace );
	
	// �o�͒l(-1�`1��0�`1�֕ϊ�)
	color = (n + 1.0) / 2.0;

}