#version 330 core

// 頂点シェーダからの入力
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

// 出力データ
out vec3 color;

// CPUから一様な値として入力
uniform sampler2D myTextureSampler;		// ダミー（テクスチャ番号0）
uniform vec3 LightPosition_worldspace;
uniform float LightPower = 10.f;

void main()
{

	// 光源のプロパティ
	vec3 LightColor = vec3(1,1,1);
	
	// マテリアルのプロパティ
	vec3 MaterialDiffuseColor = texture2D( myTextureSampler, UV ).rgb;
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.1,0.1,0.1);

	// 光源の距離
	float distance = length( LightPosition_worldspace - Position_worldspace );

	// カメラ座標での法線
	vec3 n = normalize( Normal_cameraspace );
	// 光源の方向
	vec3 l = normalize( LightDirection_cameraspace );
	// 物体の法線と光源方向間のcosine
	// 0以上に固定
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	// 視線ベクトル
	vec3 E = normalize(EyeDirection_cameraspace);
	// 光が反射する方向
	vec3 R = reflect(-l,n);
	// 視線ベクトルと反射方向のcosine
	// 0以上に固定
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	// 出力される色
	color = 
		// Ambient
		MaterialAmbientColor +
		// Diffuse
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		// Specular
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);

}