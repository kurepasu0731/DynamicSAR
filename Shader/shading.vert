#version 330 core

// 頂点毎に異なる値を入力
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

// フラグメントシェーダへ出力
out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

// CPUから一様な値として入力
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;

void main()
{

	// クリップ座標での頂点位置 : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// ワールド座標での頂点位置 : M * position
	Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	
	// カメラ座標での頂点位置
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	// カメラ座標におけるカメラ位置は (0,0,0)
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// カメラ座標における光源位置 (Mは単位行列のため，省いた)
	vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	
	// カメラ座標における法線
	Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; 
	
	// 頂点におけるUV座標
	UV = vertexUV;
}

