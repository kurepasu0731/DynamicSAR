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
	// カメラ座標での法線
	vec3 n = normalize( Normal_cameraspace );
	
	// 出力値(-1～1を0～1へ変換)
	color = (n + 1.0) / 2.0;

}