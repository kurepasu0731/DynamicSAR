#version 330 core

// 頂点毎に異なる値を入力
layout(location = 0) in vec3 vertexPosition_modelspace;


// CPUから一様な値として入力
uniform mat4 MVP;
uniform float point_size = 1.0f;

void main()
{
	// クリップ座標での頂点位置 : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	
	// 頂点の大きさを変更する
	gl_PointSize = point_size / gl_Position.w;  
}

