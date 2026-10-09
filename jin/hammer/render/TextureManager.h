#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <GL/gl.h>

extern GLuint g_texChecker;
extern GLuint g_texBrick;
extern GLuint g_texFloor;

void CreatePlaceholderTextures();
GLuint GetBrushTexture(int id);

#endif // TEXTURE_MANAGER_H