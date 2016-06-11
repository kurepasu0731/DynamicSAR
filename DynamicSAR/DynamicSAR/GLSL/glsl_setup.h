#ifndef GLSL_SETUP
#define GLSL_SETUP

#include <stdio.h>

#include <gl/glew.h>
#include <gl/freeglut.h>
#pragma comment (lib,"glew32.lib")


/***** GLSLのセットアップ(頂点シェーダのみ) *****/
void init_GLSL(GLuint *program, const char *vertexFile);

/***** GLSLのセットアップ(頂点シェーダ, フラグメントシェーダ) *****/
void init_GLSL(GLuint *program, const char *vertexFile, const char *fragmentFile);

#endif