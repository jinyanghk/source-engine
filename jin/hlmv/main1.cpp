#include <cmath>
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>
#include <unistd.h> // For getcwd tracking natively on Linux

#include <SDL2/SDL.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

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
#include "tier1/KeyValues.h"

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

// Globals for dynamic model tracking
std::vector<std::string> g_ModelList;
std::string g_CurrentModelPath = "";
std::string g_GameFolder = "hl2";

//-----------------------------------------------------------------------------
// PURE VIRTUAL VPK ARCHIVE RECURSIVE SCANNER
//-----------------------------------------------------------------------------
void ScanModelsDirectoryRecursive(const std::string &path)
{
	FileFindHandle_t findHandle;
	std::string searchFilter = path + "/*";
	const char *pFileName = g_pFileSystem->FindFirstEx(searchFilter.c_str(), "GAME", &findHandle);

	while (pFileName)
	{
		if (pFileName != '.')
		{
			std::string fullPath = path + "/" + pFileName;

			if (g_pFileSystem->FindIsDirectory(findHandle))
			{
				ScanModelsDirectoryRecursive(fullPath);
			}
			else if (fullPath.size() > 4 && fullPath.substr(fullPath.size() - 4) == ".mdl")
			{
				size_t modelsPos = fullPath.find("models/");
				if (modelsPos != std::string::npos)
				{
					std::string fixedPath = fullPath.substr(modelsPos);
					std::replace(fixedPath.begin(), fixedPath.end(), '\\', '/');

					if (std::find(g_ModelList.begin(), g_ModelList.end(), fixedPath) == g_ModelList.end())
					{
						g_ModelList.push_back(fixedPath);
					}
				}
			}
		}
		pFileName = g_pFileSystem->FindNext(findHandle);
	}
	g_pFileSystem->FindClose(findHandle);
}

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
	printf("[DEBUG] Entered global main() execution block.\n");
	CommandLine()->CreateCmdLine(argc, argv);

	printf("[DEBUG] Invoking g_ApplicationObject.Run()...\n");
	int nRet = g_ApplicationObject.Run();

	printf("[DEBUG] Application shutting down safely with return code: %d\n", nRet);
	return nRet;
}

//-----------------------------------------------------------------------------
// Subsystem registration step
//-----------------------------------------------------------------------------
bool CHammerApp::Create()
{
	printf("[DEBUG] Inside CHammerApp::Create() stage.\n");
	CommandLine()->AppendParm("-hlmv", NULL);

	IAppSystem *pSystem;
	AppModule_t cvarModule = LoadModule(VStdLib_GetICVarFactory());
	pSystem = AddSystem(cvarModule, CVAR_INTERFACE_VERSION);
	if (!pSystem)
	{
		printf("[DEBUG ERROR] Failed to bind CVAR module interface.\n");
		return false;
	}

	char pFileSystemDLL[MAX_PATH];
	bool bSteam;
	if (FileSystem_GetFileSystemDLLName(pFileSystemDLL, MAX_PATH, bSteam) != FS_OK)
	{
		printf("[DEBUG ERROR] Failed to query FileSystem DLL filename pointer mappings.\n");
		return false;
	}

	AppModule_t fileSystemModule = LoadModule(pFileSystemDLL);
	g_pFileSystem = (IFileSystem *)AddSystem(fileSystemModule, FILESYSTEM_INTERFACE_VERSION);
	if (!g_pFileSystem)
	{
		printf("[DEBUG ERROR] Failed to load structural FileSystem module factory.\n");
		return false;
	}

	printf("[DEBUG] Setting FileSystem baseline directory path trackers...\n");
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

	printf("[DEBUG] Injecting standard CreateSDLMgr system interface hook...\n");
	void *pSdlMgr = CreateSDLMgr();
	if (!pSdlMgr)
	{
		printf("[DEBUG WARNING] CreateSDLMgr() returned an empty nullptr handle. Framework might crash next.\n");
	}
	AddSystem((IAppSystem *)pSdlMgr, SDLMGR_INTERFACE_VERSION);

	printf("[DEBUG] Adding standard engine library system arrays...\n");
	if (!AddSystems(appSystems))
	{
		printf("[DEBUG ERROR] AddSystems framework batch registration encountered a major symbol failure.\n");
		return false;
	}

	g_pMaterialSystem = (IMaterialSystem *)FindSystem(MATERIAL_SYSTEM_INTERFACE_VERSION);
	g_pDataCache = (IDataCache *)FindSystem(DATACACHE_INTERFACE_VERSION);
	g_pInputSystem = (IInputSystem *)FindSystem(INPUTSYSTEM_INTERFACE_VERSION);
	g_pStudioRender = (IStudioRender *)FindSystem(STUDIO_RENDER_INTERFACE_VERSION);
	g_pMDLCache = (IMDLCache *)FindSystem(MDLCACHE_INTERFACE_VERSION);

	g_pMaterialSystem->SetShaderAPI("shaderapidx9.dll");
	printf("[DEBUG] CHammerApp::Create() completed successfully.\n");
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

SpewRetval_t HammerSpewFunc(SpewType_t type, tchar const *pMsg)
{
	if (type == SPEW_ASSERT)
		return SPEW_DEBUGGER;
	else if (type == SPEW_ERROR)
	{
		printf("[SPEW CRITICAL ERROR] %s\n", pMsg);
		return SPEW_ABORT;
	}
	else
	{
		printf("[ENGINE SPEW] %s", pMsg);
		return SPEW_CONTINUE;
	}
}

//-----------------------------------------------------------------------------
// DYNAMIC PREINIT DIAGNOSTIC
//-----------------------------------------------------------------------------
bool CHammerApp::PreInit()
{
	printf("[DEBUG] Inside CHammerApp::PreInit() setup block pass.\n");
	SpewOutputFunc(HammerSpewFunc);

	g_GameFolder = CommandLine()->ParmValue("-game", "hl2");
	printf("[DEBUG] Extracted parsed -game command argument string data token: '%s'\n", g_GameFolder.c_str());

	char szCurrentDirBuffer[MAX_PATH] = {0};
	if (getcwd(szCurrentDirBuffer, sizeof(szCurrentDirBuffer)))
	{
		printf("[DEBUG] Shell active working execution directory base tracker context: %s\n", szCurrentDirBuffer);
	}

	CFSSearchPathsInit initInfo;
	initInfo.m_pFileSystem = g_pFileSystem;
	initInfo.m_pDirectoryName = g_GameFolder.c_str();

	std::string directGameInfo = g_GameFolder + "/gameinfo.txt";
	std::string siblingGameInfo = "../" + g_GameFolder + "/gameinfo.txt";

	printf("[DEBUG] Checking physical file locations: direct='%s' sibling='%s'\n", directGameInfo.c_str(), siblingGameInfo.c_str());

	FILE *pFile = fopen(directGameInfo.c_str(), "r");
	if (pFile)
	{
		fclose(pFile);
		initInfo.m_pDirectoryName = g_GameFolder.c_str();
		printf("[DEBUG] Direct local game folder layout verified successfully.\n");
	}
	else
	{
		FILE *pSibFile = fopen(siblingGameInfo.c_str(), "r");
		if (pSibFile)
		{
			fclose(pSibFile);
			g_GameFolder = "../" + g_GameFolder;
			initInfo.m_pDirectoryName = g_GameFolder.c_str();
			printf("[DEBUG] Sibling game target tree layout verified. Shifting mount origin to: %s\n", g_GameFolder.c_str());
		}
		else
		{
			printf("[DEBUG WARNING] gameinfo.txt could not be found via local or sibling disk checks.\n");
		}
	}

	printf("[DEBUG] Injecting calculated search path map target: %s\n", initInfo.m_pDirectoryName);
	g_pFileSystem->AddSearchPath(initInfo.m_pDirectoryName, "GAME");
	g_pFileSystem->AddSearchPath("hl2", "GAME"); // Mount base hl2 components as support fallbacks

	printf("[DEBUG] Executing final FileSystem_LoadSearchPaths() pass metrics configuration...\n");
	int nLoadResult = FileSystem_LoadSearchPaths(initInfo);
	printf("[DEBUG] FileSystem_LoadSearchPaths return code output state evaluated to: %d (FS_OK = 0)\n", nLoadResult);

	if (nLoadResult != FS_OK)
	{
		printf("[DEBUG WARNING] Standard loader failed to establish search target arrays. Injecting absolute canonical manual overlays...\n");
		std::string absoluteFallbackPath = std::string(szCurrentDirBuffer) + "/" + g_GameFolder;
		g_pFileSystem->AddSearchPath(absoluteFallbackPath.c_str(), "GAME");
		initInfo.m_pDirectoryName = absoluteFallbackPath.c_str();
	}

	g_pMaterialSystem->EnableEditorMaterials();
	g_pMaterialSystem->SetAdapter(0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE);
	printf("[DEBUG] CHammerApp::PreInit() completed successfully, moving to window initialization...\n");
	return true;
}

void CHammerApp::PostShutdown() {}

int CHammerApp::Main()
{
	Msg("[DEBUG SUCCESS] Successfully breached CHammerApp::Main() entry loop block boundary!\n");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		printf("[DEBUG ERROR] Standalone SDL window framework failed to spin up.\n");
		return -1;
	}

	SDL_Window *pWindow = SDL_CreateWindow(
		"Source Engine Model Viewer | Standalone HLMV Loop",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (!pWindow)
	{
		printf("[DEBUG ERROR] Standalone SDL window instantiation handler failed.\n");
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GLContext glContext = SDL_GL_CreateContext(pWindow);
	if (!glContext)
	{
		printf("[DEBUG ERROR] Hardware accelerated context mapping compilation pass failed.\n");
		return -1;
	}
	SDL_GL_MakeCurrent(pWindow, glContext);

	int w = 1280;
	int h = 720;
	SDL_GetWindowSize(pWindow, &w, &h);

	MaterialVideoMode_t mode;
	mode.m_Width = w;
	mode.m_Height = h;
	mode.m_Format = IMAGE_FORMAT_RGBA8888;
	mode.m_RefreshRate = 60;

	MaterialSystem_Config_t config;
	config.m_VideoMode = mode;
	config.SetFlag(MATSYS_VIDCFG_FLAGS_WINDOWED, true);

	printf("[DEBUG] Invoking final Material System SetMode function on active SDL context handle...\n");
	if (!g_pMaterialSystem->SetMode((void *)pWindow, config))
	{
		printf("[DEBUG WARNING] Material System SetMode returned a warning flag mapping error.\n");
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.IniFilename = nullptr;
	io.DisplaySize = ImVec2((float)w, (float)h);

	ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
	ImGui_ImplOpenGL3_Init("#version 130");

	printf("[DEBUG] Beginning file asset traversal scan across VPK layers for model lists...\n");
	ScanModelsDirectoryRecursive("models");
	printf("[DEBUG] Traversal scan concluded. Total distinct model path assets tracked: %zu\n", g_ModelList.size());

	// FIXED FOR VIEWPORT CONTEXT VALUE STABILITY:
	// Start with a clean, un-rendered asset canvas placeholder on frame one.
	// This lets the graphics card bind its internal textures first before evaluating MDL logic!
	g_CurrentModelPath = "";
	MDLHandle_t hMdl = MDLHANDLE_INVALID;

	bool bRunning = true;
	SDL_Event event;

	float flCameraPitch = 0.0f;
	float flCameraYaw = 90.0f;
	float flZoomScale = 1.8f;
	float flPanX = 0.0f;
	float flPanY = 0.0f;
	float flPanZ = 35.0f;

	float flAnimCycle = 0.0f;
	uint32_t lastTicks = SDL_GetTicks();

	printf("[DEBUG] Booting primary system loop window frames now...\n");

	while (bRunning)
	{
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_MOUSEMOTION)
			{
				io.MousePos.x = (float)event.motion.x;
				io.MousePos.y = (float)event.motion.y;
			}
			if (event.type == SDL_MOUSEWHEEL)
			{
				io.MouseWheel += (float)event.wheel.y;
			}

			switch (event.type)
			{
			case SDL_QUIT:
				bRunning = false;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					w = event.window.data1;
					h = event.window.data2;
					io.DisplaySize = ImVec2((float)w, (float)h);
				}
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

		IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();
		if (pRenderContext)
		{
			pRenderContext->ClearColor3ub(45, 45, 48);
			pRenderContext->ClearBuffers(true, true, true);

			pRenderContext->Viewport(0, 0, w, h);
			pRenderContext->DepthRange(0.0f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LEQUAL);
			pRenderContext->Flush(false);

			Vector4D ambientCube[6];
			for (int side = 0; side < 6; side++)
			{
				ambientCube[side].Init(1.0f, 1.0f, 1.0f, 1.0f);
			}
			pRenderContext->SetAmbientLightCube(ambientCube);

			pRenderContext->MatrixMode(MATERIAL_PROJECTION);
			pRenderContext->LoadIdentity();
			double aspect = (h == 0) ? 1.0 : (double)w / (double)h;
			pRenderContext->PerspectiveX(45.0, aspect, 1.0, 2000.0);

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

			// Only render the 3D model geometry if a valid selection path is confirmed
			if (hMdl != MDLHANDLE_INVALID && !g_CurrentModelPath.empty())
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
				}
			}

			// --- IMGUI SELECTOR SIDEBAR PANEL ---
			ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
			ImGui::SetNextWindowSize(ImVec2(340, h - 20), ImGuiCond_Appearing);

			ImGui::Begin("HLMV Model Browser");
			ImGui::Text("Active Game Folder: %s", g_GameFolder.c_str());
			ImGui::Text("Total Assets Cached: %zu", g_ModelList.size());
			ImGui::Separator();

			ImGui::Text("Currently Rendering:");
			if (g_CurrentModelPath.empty())
			{
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[None Selected - Select Below]");
			}
			else
			{
				ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "%s", g_CurrentModelPath.c_str());
			}
			ImGui::Separator();

			static char szSearchFilter[256] = "";
			ImGui::InputText("Filter Search", szSearchFilter, IM_ARRAYSIZE(szSearchFilter));
			ImGui::Separator();

			if (ImGui::BeginChild("ScrollingModelList"))
			{
				for (size_t i = 0; i < g_ModelList.size(); i++)
				{
					if (strlen(szSearchFilter) > 0 && strstr(g_ModelList[i].c_str(), szSearchFilter) == nullptr)
						continue;
					bool bIsSelected = (g_ModelList[i] == g_CurrentModelPath);
					if (ImGui::Selectable(g_ModelList[i].c_str(), bIsSelected))
					{
						g_CurrentModelPath = g_ModelList[i];
						hMdl = g_pMDLCache->FindMDL(g_CurrentModelPath.c_str());
						Msg("[HLMV] Swapped active model target to: %s\n", g_CurrentModelPath.c_str());
					}
					if (bIsSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndChild();
			}
			ImGui::End();
			ImGui::Render();
			// --- COMPATIBILITY VERTEX FLIPPER LOOP ---
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
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
	_exit(0);
	return 0;
}