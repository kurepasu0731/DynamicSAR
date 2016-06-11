#version 330 core

// 頂点毎に異なる値を入力
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;


// CPUから一様な値として入力
uniform mat4 MVP;


void main()
{

	// クリップ座標での頂点位置 : MVP * position
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
}

