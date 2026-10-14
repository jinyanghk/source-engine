#ifndef MATERIAL_PREVIEW_H
#define MATERIAL_PREVIEW_H

#include <string>
#include <map>
#include <GL/gl.h>

// ★ 1. Create a complete asset resource tracker
struct MaterialTextureResource_t
{
    GLuint thumbnailTexID = 0; // Lightweight mip for ImGui drop-downs
    GLuint worldRenderTexID = 0;// Full-resolution mip for 3D brush polygons
    int nativeWidth = 64;       // Saved for planar world-space UV math
    int nativeHeight = 64;
};

class MaterialPreview
{
public:
    static MaterialPreview& Instance();

    // ★ 2. Update public access methods
    MaterialTextureResource_t GetMaterialResource(const std::string& materialPath);

    // 拿到材质的 OpenGL 纹理 ID（0 = 失败）
    // materialPath 是 "BRICK/BRICKWALL001A" 格式（大写，无 materials/ 前缀，无 .vmt）
    GLuint GetPreviewTexture(const std::string& materialPath);

    void ClearCache();

private:
    MaterialPreview() = default;
    ~MaterialPreview();
    MaterialPreview(const MaterialPreview&) = delete;
    MaterialPreview& operator=(const MaterialPreview&) = delete;

    MaterialTextureResource_t LoadMaterialResource(const std::string& materialPath);
    std::string ParseBaseTextureFromVMT(const std::string& materialPath);

    std::map<std::string, MaterialTextureResource_t> m_cache;
};

#endif // MATERIAL_PREVIEW_H