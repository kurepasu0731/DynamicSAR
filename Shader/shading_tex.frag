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

	// �����̃v���p�e�B
	vec3 LightColor = vec3(1,1,1);
	
	// �}�e���A���̃v���p�e�B
	vec3 MaterialDiffuseColor = texture2D( myTextureSampler, UV ).rgb;
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.1,0.1,0.1);

	// �����̋���
	float distance = length( LightPosition_worldspace - Position_worldspace );

	// �J�������W�ł̖@��
	vec3 n = normalize( Normal_cameraspace );
	// �����̕���
	vec3 l = normalize( LightDirection_cameraspace );
	// ���̖̂@���ƌ��������Ԃ�cosine
	// 0�ȏ�ɌŒ�
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	// �����x�N�g��
	vec3 E = normalize(EyeDirection_cameraspace);
	// �������˂������
	vec3 R = reflect(-l,n);
	// �����x�N�g���Ɣ��˕�����cosine
	// 0�ȏ�ɌŒ�
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	// �o�͂����F
	color = 
		// Ambient
		MaterialAmbientColor +
		// Diffuse
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		// Specular
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);

}