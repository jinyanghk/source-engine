#ifndef MATERIAL_PREVIEW_H
#define MATERIAL_PREVIEW_H

#include <string>
#include <map>
#include <GL/gl.h>

class MaterialPreview
{
public:
    static MaterialPreview& Instance();

    // 拿到材质的 OpenGL 纹理 ID（0 = 失败）
    // materialPath 是 "BRICK/BRICKWALL001A" 格式（大写，无 materials/ 前缀，无 .vmt）
    GLuint GetPreviewTexture(const std::string& materialPath);

    void ClearCache();

private:
    MaterialPreview() = default;
    ~MaterialPreview();
    MaterialPreview(const MaterialPreview&) = delete;
    MaterialPreview& operator=(const MaterialPreview&) = delete;

    GLuint LoadMaterialPreview(const std::string& materialPath);
    std::string ParseBaseTextureFromVMT(const std::string& materialPath);

    std::map<std::string, GLuint> m_cache;
};

#endif // MATERIAL_PREVIEW_H