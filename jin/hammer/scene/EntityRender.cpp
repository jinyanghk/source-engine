#include "scene/EntityRender.h"

#include <cstring>
#include "mathlib/mathlib.h"
#include "mathlib/vmatrix.h"
#include "datacache/imdlcache.h"
#include "istudiorender.h"

extern IMDLCache *g_pMDLCache;
extern IStudioRender *g_pStudioRender;

//-----------------------------------------------------------------------------
void BuildRotationMatrixFromQAngle(const QAngle& angles, VMatrix& outMatrix)
{
    matrix3x4_t mat;
    AngleMatrix(angles, mat);
    outMatrix.Identity();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            outMatrix.m[i][j] = mat.m_flMatVal[i][j];
}

//-----------------------------------------------------------------------------
void UpdateEntityBBox(CEntity& ent)
{
    if (ent.m_iType == ENTITY_PLAYER_START)
    {
        Vector halfSize(16, 16, 36);
        ent.m_vecBBoxMins = ent.m_vecPos - halfSize;
        ent.m_vecBBoxMaxs = ent.m_vecPos + halfSize;
        return;
    }
    if (ent.m_hMdl == MDLHANDLE_INVALID) return;
    studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(ent.m_hMdl);
    if (!pStudioHdr) return;

    VMatrix matModelTranslation;
    matModelTranslation.Identity();
    matModelTranslation.m[0][3] = ent.m_vecPos.x;
    matModelTranslation.m[1][3] = ent.m_vecPos.y;
    matModelTranslation.m[2][3] = ent.m_vecPos.z;
    VMatrix matModelRotation;
    BuildRotationMatrixFromQAngle(ent.m_angRot, matModelRotation);
    VMatrix matModel;
    MatrixMultiply(matModelTranslation, matModelRotation, matModel);

    matrix3x4_t matModel3x4;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 4; c++)
            matModel3x4.m_flMatVal[r][c] = matModel.m[r][c];

    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
    if (!pBoneArray) return;
    int numBones = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
    Vector localMins(1e9f, 1e9f, 1e9f);
    Vector localMaxs(-1e9f, -1e9f, -1e9f);
    matrix3x4_t poseBones[MAXSTUDIOBONES] = {};
    for (int i = 0; i < numBones; i++)
    {
        Vector bonePos = pBoneArray[i].pos;
        Quaternion boneQuat = pBoneArray[i].quat;
        QuaternionMatrix(boneQuat, bonePos, poseBones[i]);
        int parentIdx = pBoneArray[i].parent;
        if (parentIdx >= 0 && parentIdx < numBones)
        {
            matrix3x4_t temp; MatrixCopy(poseBones[i], temp);
            ConcatTransforms(poseBones[parentIdx], temp, poseBones[i]);
        }
    }
    for (int i = 0; i < numBones; i++)
    {
        matrix3x4_t finalBone;
        ConcatTransforms(matModel3x4, poseBones[i], finalBone);
        MatrixCopy(finalBone, poseBones[i]);
    }
    for (int i = 0; i < numBones; i++)
    {
        Vector bonePosModel(poseBones[i].m_flMatVal[0][3], poseBones[i].m_flMatVal[1][3], poseBones[i].m_flMatVal[2][3]);
        localMins.x = std::min(localMins.x, bonePosModel.x);
        localMins.y = std::min(localMins.y, bonePosModel.y);
        localMins.z = std::min(localMins.z, bonePosModel.z);
        localMaxs.x = std::max(localMaxs.x, bonePosModel.x);
        localMaxs.y = std::max(localMaxs.y, bonePosModel.y);
        localMaxs.z = std::max(localMaxs.z, bonePosModel.z);
    }
    localMins -= Vector(15.0f, 15.0f, 6.0f);
    localMaxs += Vector(15.0f, 15.0f, 14.0f);
    ent.m_vecBBoxMins = localMins;
    ent.m_vecBBoxMaxs = localMaxs;
}

//-----------------------------------------------------------------------------
void RenderEntityModel(CEntity& ent, IMatRenderContext *pRenderContext)
{
    if (ent.m_iType == ENTITY_PLAYER_START) return;
    if (ent.m_hMdl == MDLHANDLE_INVALID) return;
    studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(ent.m_hMdl);
    studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(ent.m_hMdl);
    if (!pStudioHdr || !pHardwareData) return;

    VMatrix matModelTranslation;
    matModelTranslation.Identity();
    matModelTranslation.m[0][3] = ent.m_vecPos.x;
    matModelTranslation.m[1][3] = ent.m_vecPos.y;
    matModelTranslation.m[2][3] = ent.m_vecPos.z;
    VMatrix matModelRotation;
    BuildRotationMatrixFromQAngle(ent.m_angRot, matModelRotation);
    VMatrix matModel;
    MatrixMultiply(matModelTranslation, matModelRotation, matModel);

    pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);
    pRenderContext->MatrixMode(MATERIAL_MODEL);
    pRenderContext->LoadIdentity();

    g_pStudioRender->BeginFrame();
    ::StudioRenderConfig_t studioCfg;
    memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
    studioCfg.drawEntities = 1;
    studioCfg.bNoSoftware = true;
    g_pStudioRender->UpdateConfig(studioCfg);
    g_pStudioRender->ForcedMaterialOverride(nullptr);
    g_pStudioRender->SetAlphaModulation(1.0f);
    g_pStudioRender->SetColorModulation(Vector(1.0f, 1.0f, 1.0f).Base());
    pRenderContext->OverrideDepthEnable(true, true);

    DrawModelInfo_t drawInfo;
    drawInfo.m_pStudioHdr = pStudioHdr;
    drawInfo.m_pHardwareData = pHardwareData;
    drawInfo.m_Decals = STUDIORENDER_DECAL_INVALID;
    drawInfo.m_Skin = drawInfo.m_Body = drawInfo.m_HitboxSet = drawInfo.m_Lod = 0;
    drawInfo.m_pColorMeshes = nullptr;

    matrix3x4_t poseBones[MAXSTUDIOBONES] = {};
    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
    if (pBoneArray)
    {
        int numBones = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
        for (int i = 0; i < numBones; i++)
        {
            Vector bonePos = pBoneArray[i].pos;
            Quaternion boneQuat = pBoneArray[i].quat;
            QuaternionMatrix(boneQuat, bonePos, poseBones[i]);
            int parentIdx = pBoneArray[i].parent;
            if (parentIdx >= 0 && parentIdx < numBones)
            {
                matrix3x4_t temp; MatrixCopy(poseBones[i], temp);
                ConcatTransforms(poseBones[parentIdx], temp, poseBones[i]);
            }
        }
        matrix3x4_t matModel3x4;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 4; c++)
                matModel3x4.m_flMatVal[r][c] = matModel.m[r][c];
        for (int i = 0; i < numBones; i++)
        {
            matrix3x4_t finalBone;
            ConcatTransforms(matModel3x4, poseBones[i], finalBone);
            MatrixCopy(finalBone, poseBones[i]);
        }
        g_pStudioRender->LockBoneMatrices(numBones);
        g_pStudioRender->UnlockBoneMatrices();
    }
    float pFlexWeights[MAXSTUDIOFLEXDESC] = {0.0f};
    float pFlexDelayedWeights[MAXSTUDIOFLEXDESC] = {0.0f};
    DrawModelResults_t modelResults;
    memset(&modelResults, 0, sizeof(DrawModelResults_t));
    g_pStudioRender->DrawModel(&modelResults, drawInfo, poseBones, pFlexWeights, pFlexDelayedWeights, Vector(0, 0, 0), STUDIORENDER_DRAW_ENTIRE_MODEL);
    g_pStudioRender->EndFrame();
    pRenderContext->OverrideDepthEnable(false, false);
}