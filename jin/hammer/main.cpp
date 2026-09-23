#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>

#include <SDL2/SDL.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include "appframework/AppFramework.h"
#include "tier0/dbg.h"
#include "vstdlib/cvar.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"
#include "istudiorender.h"
#include "filesystem_init.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "vphysics_interface.h"
#include "tier0/icommandline.h"
#include "appframework/ilaunchermgr.h"
#include "mathlib/vmatrix.h"

// Explicit include for input system interface mapping
#include "inputsystem/iinputsystem.h"

//-----------------------------------------------------------------------------
// Global systems
//-----------------------------------------------------------------------------
IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

extern void *CreateSDLMgr();

//-----------------------------------------------------------------------------
// The application object
//-----------------------------------------------------------------------------
class CHammerApp : public CAppSystemGroup
{
public:
	virtual bool Create();
	virtual bool PreInit();
	virtual int Main();
	virtual void PostShutdown();
	virtual void Destroy();
};

CHammerApp g_ApplicationObject;

int main(int argc, char *argv[])
{
	CommandLine()->CreateCmdLine(argc, argv);
	return g_ApplicationObject.Run();
}

//-----------------------------------------------------------------------------
// Create all singleton systems
//-----------------------------------------------------------------------------
bool CHammerApp::Create()
{
	CommandLine()->AppendParm("-hammer", NULL);

	IAppSystem *pSystem;

	AppModule_t cvarModule = LoadModule(VStdLib_GetICVarFactory());
	pSystem = AddSystem(cvarModule, CVAR_INTERFACE_VERSION);
	if (!pSystem)
		return false;

	bool bSteam;
	char pFileSystemDLL[MAX_PATH];
	if (FileSystem_GetFileSystemDLLName(pFileSystemDLL, MAX_PATH, bSteam) != FS_OK)
		return false;

	AppModule_t fileSystemModule = LoadModule(pFileSystemDLL);
	g_pFileSystem = (IFileSystem *)AddSystem(fileSystemModule, FILESYSTEM_INTERFACE_VERSION);

	FileSystem_SetBasePaths(g_pFileSystem);

	AppSystemInfo_t appSystems[] =
		{
			{"materialsystem.dll", MATERIAL_SYSTEM_INTERFACE_VERSION},
			{"inputsystem.dll", INPUTSYSTEM_INTERFACE_VERSION},
			{"studiorender.dll", STUDIO_RENDER_INTERFACE_VERSION},
			{"vphysics.dll", VPHYSICS_INTERFACE_VERSION},
			{"datacache.dll", DATACACHE_INTERFACE_VERSION},
			{"datacache.dll", MDLCACHE_INTERFACE_VERSION},
			{"datacache.dll", STUDIO_DATA_CACHE_INTERFACE_VERSION},
			{"", ""}};

	AddSystem((IAppSystem *)CreateSDLMgr(), SDLMGR_INTERFACE_VERSION);

	if (!AddSystems(appSystems))
		return false;

	g_pMaterialSystem = (IMaterialSystem *)FindSystem(MATERIAL_SYSTEM_INTERFACE_VERSION);
	g_pDataCache = (IDataCache *)FindSystem(DATACACHE_INTERFACE_VERSION);
	g_pInputSystem = (IInputSystem *)FindSystem(INPUTSYSTEM_INTERFACE_VERSION);
	g_pStudioRender = (IStudioRender *)FindSystem(STUDIO_RENDER_INTERFACE_VERSION);
	g_pMDLCache = (IMDLCache *)FindSystem(MDLCACHE_INTERFACE_VERSION);

	g_pMaterialSystem->SetShaderAPI("shaderapidx9.dll");

	return true;
}

void CHammerApp::Destroy()
{
	g_pMaterialSystem = NULL;
	g_pFileSystem = NULL;
	g_pDataCache = NULL;
	g_pInputSystem = NULL;
	g_pStudioRender = NULL;
	g_pMDLCache = NULL;
}

//-----------------------------------------------------------------------------
SpewRetval_t HammerSpewFunc(SpewType_t type, tchar const *pMsg)
{
	if (type == SPEW_ASSERT)
	{
		return SPEW_DEBUGGER;
	}
	else if (type == SPEW_ERROR)
	{
		Msg("Hammer Error %s\n", pMsg);
		return SPEW_ABORT;
	}
	else
	{
		return SPEW_CONTINUE;
	}
}

//-----------------------------------------------------------------------------
// Init, shutdown
//-----------------------------------------------------------------------------
bool CHammerApp::PreInit()
{
	SpewOutputFunc(HammerSpewFunc);

	CFSSearchPathsInit initInfo;
	initInfo.m_pFileSystem = g_pFileSystem;
	initInfo.m_pDirectoryName = "hl2";

	if (FileSystem_LoadSearchPaths(initInfo) != FS_OK)
	{
		Error("Unable to load search paths!\n");
	}

	if (g_pFileSystem)
	{
		g_pFileSystem->AddSearchPath("hl2/hl2_textures.vpk", "GAME");
		g_pFileSystem->AddSearchPath("hl2/hl2_misc.vpk", "GAME");
		g_pFileSystem->AddSearchPath("hl2", "GAME");
	}

	g_pMaterialSystem->EnableEditorMaterials();

	g_pMaterialSystem->SetAdapter(0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE);

	return true;
}

void CHammerApp::PostShutdown() {}

//-----------------------------------------------------------------------------
// Main application loop
//-----------------------------------------------------------------------------
// 1. Add the official example matrix helpers right at the top of your function scope
void HammerDemoLookAt(const float *eye, const float *at, const float *up, float *m16)
{
	float X[3], Y[3], Z[3], tmp[3];
	tmp[0] = eye[0] - at[0];
	tmp[1] = eye[1] - at[1];
	tmp[2] = eye[2] - at[2];

	float lenZ = 1.f / (sqrtf(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]) + 1e-5f);
	Z[0] = tmp[0] * lenZ;
	Z[1] = tmp[1] * lenZ;
	Z[2] = tmp[2] * lenZ;

	float lenY = 1.f / (sqrtf(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]) + 1e-5f);
	Y[0] = up[0] * lenY;
	Y[1] = up[1] * lenY;
	Y[2] = up[2] * lenY;

	tmp[0] = Y[1] * Z[2] - Y[2] * Z[1];
	tmp[1] = Y[2] * Z[0] - Y[0] * Z[2];
	tmp[2] = Y[0] * Z[1] - Y[1] * Z[0];
	float lenX = 1.f / (sqrtf(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]) + 1e-5f);
	X[0] = tmp[0] * lenX;
	X[1] = tmp[1] * lenX;
	X[2] = tmp[2] * lenX;

	Y[0] = Z[1] * X[2] - Z[2] * X[1];
	Y[1] = Z[2] * X[0] - Z[0] * X[2];
	Y[2] = Z[0] * X[1] - Z[1] * X[0];
	float lenY2 = 1.f / (sqrtf(Y[0] * Y[0] + Y[1] * Y[1] + Y[2] * Y[2]) + 1e-5f);
	Y[0] *= lenY2;
	Y[1] *= lenY2;
	Y[2] *= lenY2;

	m16[0] = X[0];
	m16[1] = Y[0];
	m16[2] = Z[0];
	m16[3] = 0.0f;
	m16[4] = X[1];
	m16[5] = Y[1];
	m16[6] = Z[1];
	m16[7] = 0.0f;
	m16[8] = X[2];
	m16[9] = Y[2];
	m16[10] = Z[2];
	m16[11] = 0.0f;
	m16[12] = -(X[0] * eye[0] + X[1] * eye[1] + X[2] * eye[2]);
	m16[13] = -(Y[0] * eye[0] + Y[1] * eye[1] + Y[2] * eye[2]);
	m16[14] = -(Z[0] * eye[0] + Z[1] * eye[1] + Z[2] * eye[2]);
	m16[15] = 1.0f;
}

void HammerDemoPerspective(float fovy, float aspect, float znear, float zfar, float *m16)
{
	float ymax = znear * tanf(fovy * 3.141592f / 360.0f);
	float xmax = ymax * aspect;
	float left = -xmax, right = xmax, bottom = -ymax, top = ymax;
	float temp = 2.0f * znear;
	float temp2 = right - left;
	float temp3 = top - bottom;
	float temp4 = zfar - znear;
	m16[0] = temp / temp2;
	m16[1] = 0.0f;
	m16[2] = 0.0f;
	m16[3] = 0.0f;
	m16[4] = 0.0f;
	m16[5] = temp / temp3;
	m16[6] = 0.0f;
	m16[7] = 0.0f;
	m16[8] = (right + left) / temp2;
	m16[9] = (top + bottom) / temp3;
	m16[10] = -(zfar + znear) / temp4;
	m16[11] = -1.0f;
	m16[12] = 0.0f;
	m16[13] = 0.0f;
	m16[14] = -(temp * zfar) / temp4;
	m16[15] = 0.0f;
}

int CHammerApp::Main()
{
	// 1. Initial Window Handle Lookups & Layout Configurations
	SDL_Window *pWindow = SDL_GL_GetCurrentWindow();
	int w = 1280;
	int h = 720;
	if (pWindow)
	{
		SDL_GetWindowSize(pWindow, &w, &h);
		if (w == 0 || h == 0)
		{
			w = 1280;
			h = 720;
		}
	}

	MaterialVideoMode_t mode;
	mode.m_Width = w;
	mode.m_Height = h;
	mode.m_Format = IMAGE_FORMAT_RGBA8888;
	mode.m_RefreshRate = 60;

	MaterialSystem_Config_t config;
	config.m_VideoMode = mode;
	config.SetFlag(MATSYS_VIDCFG_FLAGS_WINDOWED, true);

	if (!g_pMaterialSystem->SetMode((void *)pWindow, config))
	{
		Warning("[HAMMER] Material System SetMode tracking failure.\n");
	}

	pWindow = SDL_GL_GetCurrentWindow();
	SDL_ShowWindow(pWindow);
	SDL_RaiseWindow(pWindow);
	SDL_SetWindowSize(pWindow, w, h);

	SDL_GLContext glContext = SDL_GL_GetCurrentContext();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	//io.DisplaySize = ImVec2((float)w, (float)h);
	//io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.IniFilename = nullptr; // disable .ini file

	ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
	ImGui_ImplOpenGL3_Init("#version 130");

	MDLHandle_t hMdl = g_pMDLCache->FindMDL("models/alyx.mdl");
	IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();

	bool bRunning = true;
	SDL_Event event;

	// Camera Workspace Properties (Restored to Clean Front-Facing System Defaults)
	float flCameraPitch = 0.0f;
	float flCameraYaw = 90.0f;
	float flZoomScale = 1.8f;
	float flPanX = 0.0f;
	float flPanY = 0.0f;
	float flPanZ = 35.0f;

	// Standard transformation array data tracks for ImGuizmo position vectors
	static float objectMatrix[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 35.f, 1.f // Anchors the handles precisely at her chest bone level
	};
	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

	float flAnimCycle = 0.0f;
	uint32_t lastTicks = SDL_GetTicks();

	while (bRunning)
	{
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type)
			{
			case SDL_QUIT:
				bRunning = false;
				break;

			case SDL_KEYDOWN:
				if (!io.WantCaptureKeyboard && event.key.keysym.sym == SDLK_ESCAPE)
					bRunning = false;
				break;
			}
		}

		uint32_t currentTicks = SDL_GetTicks();
		float frameTime = (currentTicks - lastTicks) / 1000.0f;
		if (frameTime == 0.0f)
			frameTime = 0.01f;
		lastTicks = currentTicks;

		flAnimCycle += frameTime * 0.4f;
		if (flAnimCycle > 1.0f)
			flAnimCycle -= 1.0f;

		// Interactive Camera input tracking passes
		if (!io.WantCaptureMouse)
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				flCameraYaw += io.MouseDelta.x * 0.25f;
				flCameraPitch += io.MouseDelta.y * 0.25f;
				if (flCameraPitch > 89.0f)
					flCameraPitch = 89.0f;
				if (flCameraPitch < -89.0f)
					flCameraPitch = -89.0f;
			}
			else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
			{
				float radYaw = flCameraYaw * (M_PI / 180.0f);
				flPanX -= (std::sin(radYaw) * io.MouseDelta.x) * 0.05f * flZoomScale;
				flPanY += (std::cos(radYaw) * io.MouseDelta.x) * 0.05f * flZoomScale;
				flPanZ += io.MouseDelta.y * 0.05f * flZoomScale;
			}

			if (io.MouseWheel != 0.0f)
			{
				flZoomScale -= io.MouseWheel * 0.15f * (flZoomScale * 0.4f);
				if (flZoomScale < 0.1f)
					flZoomScale = 0.1f;
				if (flZoomScale > 15.0f)
					flZoomScale = 15.0f;
			}
		}

		g_pMaterialSystem->BeginFrame(frameTime);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		pRenderContext = g_pMaterialSystem->GetRenderContext();
		if (pRenderContext)
		{
			pRenderContext->ClearColor3ub(51, 51, 51);
			pRenderContext->ClearBuffers(true, true, true);

			pRenderContext->Viewport(0, 0, w, h);
			pRenderContext->DepthRange(0.0f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LEQUAL);

			pRenderContext->Flush(false);

			// Re-inject pristine 6-sided volumetric lighting cube array
			Vector4D ambientCube[6];
			for (int side = 0; side < 6; side++)
			{
				ambientCube[side].Init(1.0f, 1.0f, 1.0f, 1.0f);
			}
			pRenderContext->SetAmbientLightCube(ambientCube);

			// Setup Projection Matrix natively via pure engine calls
			pRenderContext->MatrixMode(MATERIAL_PROJECTION);
			pRenderContext->LoadIdentity();
			double aspect = (double)w / (double)h;
			pRenderContext->PerspectiveX(45.0, aspect, 1.0, 2000.0);

			VMatrix matProj;
			pRenderContext->GetMatrix(MATERIAL_PROJECTION, &matProj);

			// Setup View Matrix natively via pure engine calls
			pRenderContext->MatrixMode(MATERIAL_VIEW);
			pRenderContext->LoadIdentity();

			float radPitch = flCameraPitch * (M_PI / 180.0f);
			float radYaw = flCameraYaw * (M_PI / 180.0f);
			float distance = 50.0f * flZoomScale;
			Vector vecEye(distance * std::cos(radPitch) * std::cos(radYaw), distance * std::cos(radPitch) * std::sin(radYaw), distance * std::sin(radPitch));
			Vector vecAt(flPanX, flPanY, flPanZ);
			vecEye += vecAt;
			Vector vecUp(0, 0, 1);
			Vector forward = vecAt - vecEye;
			VectorNormalize(forward);
			Vector left;
			CrossProduct(vecUp, forward, left);
			VectorNormalize(left);
			Vector up;
			CrossProduct(forward, left, up);
			VMatrix matView;
			matView.Init(left.x, left.y, left.z, -DotProduct(left, vecEye), up.x, up.y, up.z, -DotProduct(up, vecEye), -forward.x, -forward.y, -forward.z, DotProduct(forward, vecEye), 0.0f, 0.0f, 0.0f, 1.0f);
			pRenderContext->LoadMatrix(matView);

			// 3D character model rendering pass
			if (hMdl != MDLHANDLE_INVALID)
			{
				studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(hMdl);
				studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(hMdl);
				if (pStudioHdr && pHardwareData)
				{
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
					if (pBoneArray != nullptr)
					{
						int numBonesToProcess = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
						for (int i = 0; i < numBonesToProcess; i++)
						{
							Vector bonePos = pBoneArray[i].pos;
							Quaternion boneQuat = pBoneArray[i].quat;
							if (pBoneArray[i].parent != -1 && (strstr(pBoneArray[i].pszName(), "Spine2") || strstr(pBoneArray[i].pszName(), "Spine4")))
							{
								boneQuat.x += std::sin(flAnimCycle * M_PI * 2.0f) * 0.015f;
							}
							QuaternionMatrix(boneQuat, bonePos, poseBones[i]);
							int parentIdx = pBoneArray[i].parent;
							if (parentIdx >= 0 && parentIdx < numBonesToProcess)
							{
								matrix3x4_t temporaryTransform;
								MatrixCopy(poseBones[i], temporaryTransform);
								ConcatTransforms(poseBones[parentIdx], temporaryTransform, poseBones[i]);
							}
						}
						g_pStudioRender->LockBoneMatrices(numBonesToProcess);
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
			}

			// Render ImGui workspace console panel
			ImGui::Begin("ImGuizmo Operator Console");
			ImGui::Text("Widget Modes Selection:");
			if (ImGui::RadioButton("Translate Arrows (W)", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
				mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
			if (ImGui::RadioButton("Rotate Rings (E)", mCurrentGizmoOperation == ImGuizmo::ROTATE))
				mCurrentGizmoOperation = ImGuizmo::ROTATE;
			if (ImGui::RadioButton("Scale Blocks (R)", mCurrentGizmoOperation == ImGuizmo::SCALE))
				mCurrentGizmoOperation = ImGuizmo::SCALE;
			ImGui::Separator();
			ImGui::Text("Gizmo Target Coordinates:");
			ImGui::Text("X: %.2f | Y: %.2f | Z: %.2f", objectMatrix[12], objectMatrix[13], objectMatrix[14]);
			ImGui::End();

			// --- THE MAGIC BRIDGE: CONVERT AND TRANSPOSE SOURCE MATRICES TO COLUMN-MAJOR ---
			// We convert the row-major engine matrices to column-major float arrays [j][i] 
			// and apply a vertical inversion on the projection matrix to align perfectly with the UI flipper!
			float rawView[16];
			float rawProj[16];
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					// Transpose the View Matrix
					rawView[j * 4 + i] = matView.m[i][j];
					
					// Transpose the Projection Matrix
					rawProj[j * 4 + i] = matProj.m[i][j];
				}
			}

			// THE ULTIMATE ALIGNMENT FLIP:
			// Invert the vertical scaling components inside the Column-Major Projection matrix.
			// This forces ImGuizmo's lines to flip right-side up along with our 2D vertex flipper system!
			rawProj[5] = -rawProj[5];   // Invert Y scale
			rawProj[9] = -rawProj[9];   // Invert Y translation offset

			// Execute ImGuizmo manipulation right over the unified coordinate bounds
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(0, 0, (float)w, (float)h);
			
			ImGuizmo::Manipulate(
				rawView, 
				rawProj, 
				mCurrentGizmoOperation, 
				mCurrentGizmoMode, 
				objectMatrix
			);

			ImGui::Render();

			// Flipper System (Inverts 2D UI layouts and 3D arrow vertex positions upright safely together)
			ImDrawData *draw_data = ImGui::GetDrawData();
			if (draw_data)
			{
				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					ImDrawList *cmd_list = draw_data->CmdLists[n];
					for (int v_idx = 0; v_idx < cmd_list->VtxBuffer.Size; v_idx++)
					{
						ImDrawVert &vertex = cmd_list->VtxBuffer.Data[v_idx];
						vertex.pos.y = (float)h - vertex.pos.y;
					}
					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
						float clip_rect_h = pcmd->ClipRect.w - pcmd->ClipRect.y;
						pcmd->ClipRect.y = (float)h - pcmd->ClipRect.w;
						pcmd->ClipRect.w = pcmd->ClipRect.y + clip_rect_h;
					}
				}
			}
			ImGui_ImplOpenGL3_RenderDrawData(draw_data);
			pRenderContext->Flush(true);
		}
		g_pMaterialSystem->EndFrame();
		g_pMaterialSystem->SwapBuffers();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	_exit(0);
	return 0;
}