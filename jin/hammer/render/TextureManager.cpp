#include "render/TextureManager.h"

GLuint g_texChecker = 0;
GLuint g_texBrick = 0;
GLuint g_texFloor = 0;

void CreatePlaceholderTextures()
{
    const int SIZE = 128;
    unsigned char data[SIZE * SIZE * 4];

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
        {
            int c = ((x / 16) + (y / 16)) % 2;
            unsigned char v = c ? 220 : 120;
            int idx = (y * SIZE + x) * 4;
            data[idx+0] = v; data[idx+1] = v; data[idx+2] = v; data[idx+3] = 255;
        }
    glGenTextures(1, &g_texChecker);
    glBindTexture(GL_TEXTURE_2D, g_texChecker);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
        {
            int row = y / 16;
            int mortarX = (x + (row % 2) * 16) % 32;
            int mortarY = y % 16;
            int idx = (y * SIZE + x) * 4;
            bool isMortar = (mortarX < 2) || (mortarY < 2);
            if (isMortar) { data[idx+0]=100; data[idx+1]=100; data[idx+2]=100; }
            else
            {
                int rand = ((x * 7 + y * 13) % 30) - 15;
                data[idx+0] = (unsigned char)(160 + rand);
                data[idx+1] = (unsigned char)(70 + rand / 2);
                data[idx+2] = (unsigned char)(50 + rand / 2);
            }
            data[idx+3] = 255;
        }
    glGenTextures(1, &g_texBrick);
    glBindTexture(GL_TEXTURE_2D, g_texBrick);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
        {
            int c = ((x / 8) + (y / 8)) % 2;
            unsigned char v = c ? 180 : 140;
            int idx = (y * SIZE + x) * 4;
            data[idx+0] = v;
            data[idx+1] = (unsigned char)(v * 0.9f);
            data[idx+2] = (unsigned char)(v * 0.7f);
            data[idx+3] = 255;
        }
    glGenTextures(1, &g_texFloor);
    glBindTexture(GL_TEXTURE_2D, g_texFloor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

GLuint GetBrushTexture(int id)
{
    if (id == 0) return g_texChecker;
    if (id == 1) return g_texBrick;
    return g_texFloor;
}