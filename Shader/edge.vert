#version 330 core

// 頂点毎に異なる値を入力
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec3 index;

// CPUから一様な値として入力
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform float point_size = 1.0f;

flat out vec3 model_index;		// 頂点のインデックス

void main()
{
	model_index = index;

	// カメラ座標での頂点位置
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	// カメラ座標におけるカメラ位置は (0,0,0)
	vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// カメラ座標での法線
	vec3 n = normalize( (V * M * vec4(vertexNormal_modelspace,0)).xyz );
	// 視線ベクトル
	vec3 E = normalize( EyeDirection_cameraspace );
	// 物体の法線と視線ベクトルのcosine (0以上に固定)
	float cosAlpha = dot( n,E );

	//if ( cosAlpha > -0.06 && cosAlpha < 0.06) {
		// クリップ座標での頂点位置 : MVP * position
		gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	//}else {
	//	gl_Position = vec4(-100,0,0,0);
	//}

	// 頂点の大きさを変更する
	gl_PointSize = point_size / gl_Position.w;  
}

