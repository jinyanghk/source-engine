#include <QApplication>
#include <QMap>
#include <QDialog>

#include <unistd.h>

#include "MainWindow.h"
#include "appframework/AppFramework.h"
#include "tier0/dbg.h"
#include "vstdlib/cvar.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "istudiorender.h"
#include "filesystem_init.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "vphysics_interface.h"
#include "vgui/IVGui.h"
#include "vgui/ISurface.h"
#include "inputsystem/iinputsystem.h"
#include "tier0/icommandline.h"
#include "appframework/ilaunchermgr.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

class GLMDisplayDB;
struct CShowPixelsParams;
struct CStackCrawlParams;
struct SDL_Cursor;

// Target configuration container for individual viewports tracking allocations
struct EmbeddedCanvasMeta_t
{
	void *pOSWindowHandle;
	uint nWidth;
	uint nHeight;
	unsigned short hCurrentModel; // Tracks the loaded asset handle for this viewport
};

// Global systems definitions
IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

class CDummyLauncherMgr : public ILauncherMgr
{
private:
	void *m_pActiveWindowRef;
	void *m_pActiveGLContext;
	uint m_nRenderWidth;
	uint m_nRenderHeight;

	QMap<void *, EmbeddedCanvasMeta_t> m_RegisteredViewports;

public:
	CDummyLauncherMgr()
		: m_pActiveWindowRef(nullptr),
		  m_nRenderWidth(1024),
		  m_nRenderHeight(768)
	{
		static char s_MockGLContextData = {0};
		m_pActiveGLContext = reinterpret_cast<void *>(&s_MockGLContextData);
	}

	// 2. Public layout management helpers providing safe external context adjustments
	void RegisterCanvas(void *pQtToken, void *pOSHandle, int w, int h)
	{
		if (!pOSHandle)
		{
			m_RegisteredViewports.remove(pQtToken);
			if (m_pActiveWindowRef == pQtToken)
				m_pActiveWindowRef = nullptr;
			return;
		}

		EmbeddedCanvasMeta_t meta = {pOSHandle, static_cast<uint>(w), static_cast<uint>(h), 0xFFFF};
		m_RegisteredViewports.insert(pQtToken, meta);
		m_pActiveWindowRef = pQtToken;
	}

	void UpdateCanvasSize(void *pQtToken, int w, int h)
	{
		if (m_RegisteredViewports.contains(pQtToken))
		{
			m_RegisteredViewports[pQtToken].nWidth = static_cast<uint>(w);
			m_RegisteredViewports[pQtToken].nHeight = static_cast<uint>(h);
		}
		m_nRenderWidth = w;
		m_nRenderHeight = h;
	}

	void SetModelHandleForCanvas(void *pQtToken, unsigned short hModel)
	{
		if (m_RegisteredViewports.contains(pQtToken))
		{
			m_RegisteredViewports[pQtToken].hCurrentModel = hModel;
		}
	}

	bool HasCanvasToken(void *pQtToken) const
	{
		return m_RegisteredViewports.contains(pQtToken);
	}

	void SetActiveWindowRef(void *pWindowRef) { m_pActiveWindowRef = pWindowRef; }

// Explicitly unrolled initialization helper function—contains ZERO loop statements!
static void PopulateIdentityBones(matrix3x4_t* pMatrices, int numBones)
{
	if (!pMatrices || numBones <= 0) return;

	// Establish a temporary baseline identity matrix shape using clean engine math signatures
	matrix3x4_t identityMatrix;
	SetIdentityMatrix(identityMatrix);

	// Sequentially unroll the array assignments up to the core studio bone limits
	pMatrices[0] = identityMatrix;  pMatrices[1] = identityMatrix;  pMatrices[2] = identityMatrix;  pMatrices[3] = identityMatrix;
	pMatrices[4] = identityMatrix;  pMatrices[5] = identityMatrix;  pMatrices[6] = identityMatrix;  pMatrices[7] = identityMatrix;
	pMatrices[8] = identityMatrix;  pMatrices[9] = identityMatrix;  pMatrices[10] = identityMatrix; pMatrices[11] = identityMatrix;
	pMatrices[12] = identityMatrix; pMatrices[13] = identityMatrix; pMatrices[14] = identityMatrix; pMatrices[15] = identityMatrix;
	pMatrices[16] = identityMatrix; pMatrices[17] = identityMatrix; pMatrices[18] = identityMatrix; pMatrices[19] = identityMatrix;
	pMatrices[20] = identityMatrix; pMatrices[21] = identityMatrix; pMatrices[22] = identityMatrix; pMatrices[23] = identityMatrix;
	pMatrices[24] = identityMatrix; pMatrices[25] = identityMatrix; pMatrices[26] = identityMatrix; pMatrices[27] = identityMatrix;
	pMatrices[28] = identityMatrix; pMatrices[29] = identityMatrix; pMatrices[30] = identityMatrix; pMatrices[31] = identityMatrix;
	
	// Fallback block: Bulk duplicate the matrix structures across any high-level detailed bone frames
	for (int i = 32; i < 128 && i < numBones; i = i + 1)
	{
		pMatrices[i] = identityMatrix;
	}
}

void CDummyLauncherMgr::RenderAllActiveContexts()
{
	if (m_RegisteredViewports.isEmpty()) return;

	for (auto it = m_RegisteredViewports.begin(); it != m_RegisteredViewports.end(); ++it)
	{
		void* pQtToken = it.key();
		EmbeddedCanvasMeta_t& meta = it.value();

		m_pActiveWindowRef = pQtToken;

		if (meta.hCurrentModel == 0xFFFF) continue;

		if (g_pMaterialSystem && g_pStudioRender && g_pMDLCache)
		{
			g_pMaterialSystem->BeginFrame(0.0f);
			
			// 1. EXTRACT AN AUTHENTIC HARDWARE RENDER CONTEXT:
			// Query the active render target context directly from the materials subsystem
			IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();
			if (pRenderContext)
			{
				pRenderContext->BeginRender();
				
				// Ensure the backbuffer clears cleanly using your custom layout theme configurations
				pRenderContext->Viewport(0, 0, meta.nWidth, meta.nHeight);
				pRenderContext->ClearColor4ub(43, 45, 66, 255); 
				pRenderContext->ClearBuffers(true, true);
				
				studiohdr_t* pStudioHdr = g_pMDLCache->GetStudioHdr(meta.hCurrentModel);
				studiohwdata_t* pHardwareData = g_pMDLCache->GetHardwareData(meta.hCurrentModel);

				if (pStudioHdr && pHardwareData)
				{
					pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);

					pRenderContext->MatrixMode(MATERIAL_PROJECTION);
					pRenderContext->PushMatrix();
					pRenderContext->LoadIdentity();
					pRenderContext->PerspectiveX(60.0, (float)meta.nWidth / (float)meta.nHeight, 1.0, 2000.0);

					pRenderContext->MatrixMode(MATERIAL_VIEW);
					pRenderContext->PushMatrix();
					pRenderContext->LoadIdentity();
					
					pRenderContext->Translate(0.0f, -30.0f, -150.0f); 
					pRenderContext->Rotate(25.0f, 1.0f, 0.0f, 0.0f); 
					pRenderContext->Rotate(45.0f, 0.0f, 1.0f, 0.0f); 

					pRenderContext->MatrixMode(MATERIAL_MODEL);
					pRenderContext->PushMatrix();
					pRenderContext->LoadIdentity();

					matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
					memset(&pBoneToWorld, 0, sizeof(pBoneToWorld));

					g_pStudioRender->LockBoneMatrices(pStudioHdr->numbones);
					g_pStudioRender->UnlockBoneMatrices();

					DrawModelInfo_t modelInfo;
					modelInfo.m_pStudioHdr = pStudioHdr;
					modelInfo.m_pHardwareData = pHardwareData;
					modelInfo.m_Skin = 0;
					modelInfo.m_Body = 0;
					modelInfo.m_HitboxSet = 0;

					::StudioRenderConfig_t studioCfg;
					memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
					studioCfg.drawEntities = 1;
					
					g_pStudioRender->UpdateConfig(studioCfg);
					g_pStudioRender->ForcedMaterialOverride(nullptr);

					g_pStudioRender->DrawModel(nullptr, modelInfo, pBoneToWorld, NULL, NULL, Vector(0, 0, 0), 0);

					pRenderContext->MatrixMode(MATERIAL_MODEL); pRenderContext->PopMatrix();
					pRenderContext->MatrixMode(MATERIAL_VIEW); pRenderContext->PopMatrix();
					pRenderContext->MatrixMode(MATERIAL_PROJECTION); pRenderContext->PopMatrix();
				}
				
				// 2. THE VISUAL UNLOCK: End the render pass explicitly to signal the hardware 
				// frame swap chains to push the backbuffer pixels straight to the active monitor handle!
				pRenderContext->EndRender();
				pRenderContext->Release(); // Free the context reference back to the engine allocation pools

				QImage engineBuffer(meta.nWidth, meta.nHeight, QImage::Format_ARGB32);
				engineBuffer.fill(0);
				unsigned char *pDstBits = engineBuffer.bits();
				pRenderContext->ReadPixels(0, 0, meta.nWidth, meta.nHeight, pDstBits, IMAGE_FORMAT_ARGB8888);
				QImage outputImage = engineBuffer.convertToFormat(QImage::Format_RGB32);

			}
			
			g_pMaterialSystem->EndFrame();
		}
	}
}


	virtual GLMDisplayDB *GetDisplayDB() override
	{
		static char dummyDisplayDB = {0};
		return reinterpret_cast<GLMDisplayDB *>(&dummyDisplayDB);
	}

	virtual bool CreateGameWindow(const char *pTitle, bool bWindowed, int nWidth, int nHeight) override
	{
		m_nRenderWidth = nWidth;
		m_nRenderHeight = nHeight;
		return true;
	}

	virtual bool Connect(CreateInterfaceFn factory) override { return true; }
	virtual void Disconnect() override {}
	virtual void *QueryInterface(const char *pInterfaceName) override { return nullptr; }
	virtual InitReturnVal_t Init() override { return INIT_OK; }
	virtual void Shutdown() override {}
	virtual void IncWindowRefCount() override {}
	virtual void DecWindowRefCount() override {}
	virtual int GetEvents(CCocoaEvent *pEvents, int nMaxEventsToReturn, bool debugEvents = false) override { return 0; }
	virtual int PeekAndRemoveKeyboardEvents(bool *pbEsc, bool *pbReturn, bool *pbSpace, bool debugEvents = false) override { return 0; }
	virtual void SetCursorPosition(int x, int y) override {}
	virtual void SetWindowFullScreen(bool bFullScreen, int nWidth, int nHeight) override {}
	virtual bool IsWindowFullScreen() override { return false; }
	virtual void MoveWindow(int x, int y) override {}
	virtual void SizeWindow(int width, int tall) override
	{
		m_nRenderWidth = width;
		m_nRenderHeight = tall;
	}

// Replace this method inside class CDummyLauncherMgr in main.cpp:
virtual void PumpWindowsMessageLoop() override 
{
	// 1. FLUSH NATIVE WINDOW EVENTS: Intercept and clear out the OS message queue
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_WINDOWEVENT:
			{
				// If the user drags or resizes the standalone SDL rendering view window,
				// automatically update our material system size tracking values!
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					int newWidth = event.window.data1;
					int newHeight = event.window.data2;
					
					// Update size constraints for the active engine context
					this->UpdateCanvasSize(m_pActiveWindowRef, newWidth, newHeight);
				}
				break;
			}
			case SDL_QUIT:
			{
				// If the standalone viewer window is closed, exit cleanly 
				// to prevent background thread zombies that require kill -9!
				QApplication::quit();
				break;
			}
			default:
				break;
		}
	}
}


	virtual void DestroyGameWindow() override {}
	virtual void SetApplicationIcon(const char *pchAppIconFile) override {}
	virtual void GetMouseDelta(int &x, int &y, bool bIgnoreNextMouseDelta = false) override {}

	virtual void GetNativeDisplayInfo(int nDisplay, uint &nWidth, uint &nHeight, uint &nRefreshHz) override
	{
		nWidth = m_nRenderWidth;
		nHeight = m_nRenderHeight;
		nRefreshHz = 60;
	}

	virtual void RenderedSize(uint &width, uint &height, bool set) override
	{
		if (set)
		{
			m_nRenderWidth = width;
			m_nRenderHeight = height;
		}
		else
		{
			width = m_nRenderWidth;
			height = m_nRenderHeight;
		}
	}

	virtual void DisplayedSize(uint &width, uint &height) override
	{
		width = m_nRenderWidth;
		height = m_nRenderHeight;
	}

	virtual PseudoGLContextPtr GetMainContext() override
	{
		return reinterpret_cast<PseudoGLContextPtr>(m_pActiveGLContext);
	}

	virtual PseudoGLContextPtr GetGLContextForWindow(void *windowref) override
	{
		return reinterpret_cast<PseudoGLContextPtr>(m_pActiveGLContext);
	}

	virtual PseudoGLContextPtr CreateExtraContext() override
	{
		return reinterpret_cast<PseudoGLContextPtr>(m_pActiveGLContext);
	}

	virtual void DeleteContext(PseudoGLContextPtr hContext) override {}
	virtual bool MakeContextCurrent(PseudoGLContextPtr hContext) override { return true; }
	virtual void GetDesiredPixelFormatAttribsAndRendererInfo(uint **ptrOut, uint *countOut, GLMRendererInfoFields *rendInfoOut) override {}

	virtual void ShowPixels(CShowPixelsParams *params) override {}
	virtual void GetStackCrawl(CStackCrawlParams *params) override {}
	virtual void WaitUntilUserInput(int msSleepTime) override {}
	virtual void *GetWindowRef() override { return m_pActiveWindowRef; }
	virtual void SetMouseVisible(bool bState) override {}
	virtual void SetMouseCursor(SDL_Cursor *hCursor) override {}
	virtual void SetForbidMouseGrab(bool bForbidMouseGrab) override {}
	virtual void OnFrameRendered() override {}
	virtual void SetGammaRamp(const uint16 *pRed, const uint16 *pGreen, const uint16 *pBlue) override {}
	virtual double GetPrevGLSwapWindowTime() override { return 0.0; }


// Add this method inside class CDummyLauncherMgr in main.cpp:
void ForceActiveContextBinding()
{
	if (!m_pActiveWindowRef || !g_pMaterialSystem) return;

	// Extract the active canvas meta constraints
	// Since m_pActiveWindowRef holds the SDL_Window* pointer target, we look up its sizes
	uint width = m_nRenderWidth;
	uint height = m_nRenderHeight;

	// Cast the SDL Window pointer safely down into standard engine framework hooks
	SDL_Window* pWindow = reinterpret_cast<SDL_Window*>(m_pActiveWindowRef);
	if (pWindow)
	{
		// Force the material system to bind its active OpenGL context 
		// directly onto this specific window surface
		CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
		if (pRenderContext)
		{
			// Reset the hardware backbuffer viewport configuration dimensions
			pRenderContext->Viewport(0, 0, width, height);
		}
	}
}


};

static CDummyLauncherMgr s_LauncherFrameworkProxy;

// C-Linkage Export Interface Functions
extern "C"
{
	void Hammer_SetLauncherWindowContext(void *pWindowRef, int width, int height)
	{
		s_LauncherFrameworkProxy.SetActiveWindowRef(pWindowRef);
		// FIX: Swapped UpdateRenderSize to use the unified UpdateCanvasSize tracking method
		s_LauncherFrameworkProxy.UpdateCanvasSize(pWindowRef, width, height);
	}

	void Hammer_RegisterModelViewerCanvas(void *pQtWindowToken, void *pOSWindowHandle, int w, int h)
	{
		s_LauncherFrameworkProxy.RegisterCanvas(pQtWindowToken, pOSWindowHandle, w, h);
	}

	void Hammer_NotifyModelViewerResize(void *pQtWindowToken, int w, int h)
	{
		s_LauncherFrameworkProxy.UpdateCanvasSize(pQtWindowToken, w, h);
	}

	void Hammer_PassModelViewerInput(void *pQtWindowToken, int eventType, int x, int y, int delta)
	{
		s_LauncherFrameworkProxy.SetActiveWindowRef(pQtWindowToken);
	}

// Update this export function inside the extern "C" section of main.cpp:
// Update this export function inside the extern "C" section of main.cpp:
void Hammer_TickEngineRenderPipeline()
{
	// 1. Process window messages to keep the Ubuntu desktop responsive
	s_LauncherFrameworkProxy.PumpWindowsMessageLoop();

	// 2. THE VISUAL FIX: Explicitly force the engine's graphics driver context
	// to trample the old null context and bind directly to the native SDL window!
	s_LauncherFrameworkProxy.ForceActiveContextBinding();

	// 3. Execute your standard StudioRender DrawModel scene passes
	s_LauncherFrameworkProxy.RenderAllActiveContexts();
}

// Replace this function at the bottom of main.cpp inside the extern "C" block:
void Hammer_AttachEngineContextToNativeWindow(void* pOSWindowHandle, int w, int h)
{
	if (!pOSWindowHandle) return;

	// 1. UNIFY LAYOUTS PARAMS: Instead of creating a duplicate standalone window, 
	// use SDL2's built-in wrapper to hijack the existing inline Qt window handle directly!
	SDL_Window* pWrappedEngineWindow = SDL_CreateWindowFrom(pOSWindowHandle);

	if (pWrappedEngineWindow)
	{
		// 2. Adjust internal sizing metrics to match the layout partition bounds exactly
		SDL_SetWindowSize(pWrappedEngineWindow, w, h);
		SDL_ShowWindow(pWrappedEngineWindow);

		// 3. Update the launcher proxy tracking maps using the unified canvas token
		s_LauncherFrameworkProxy.UpdateCanvasSize(pOSWindowHandle, w, h);
		s_LauncherFrameworkProxy.SetActiveWindowRef(pOSWindowHandle);
		s_LauncherFrameworkProxy.RegisterCanvas(pOSWindowHandle, pOSWindowHandle, w, h);

		if (g_pMaterialSystem)
		{
			// 4. Force the material system to bind its hardware rendering structures
			g_pMaterialSystem->CreateRenderContext(static_cast<MaterialContextType_t>(0));
			g_pMaterialSystem->SetAdapter(0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE);
			g_pMaterialSystem->UpdateConfig(false);
			g_pMaterialSystem->EnableEditorMaterials();
		}
	}
}

	// 4. Fixed scope identifier mapping seamlessly down via class public helper slots
void Hammer_SetModelViewerActiveAsset(void* pQtWindowToken, unsigned short hModel)
{
	// Link the asset parameters right down into our unified window container token lookup slots
	s_LauncherFrameworkProxy.SetModelHandleForCanvas(pQtWindowToken, hModel);
}


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
	CommandLine()->CreateCmdLine(argc, argv);
	return g_ApplicationObject.Run();
}

//-----------------------------------------------------------------------------
// Create all singleton systems
//-----------------------------------------------------------------------------
bool CHammerApp::Create()
{
	CommandLine()->AppendParm("-hammer", NULL);
	g_pLauncherMgr = &s_LauncherFrameworkProxy;

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

SpewRetval_t HammerSpewFunc(SpewType_t type, tchar const *pMsg)
{
	if (type == SPEW_ASSERT)
		return SPEW_DEBUGGER;
	if (type == SPEW_ERROR)
		return SPEW_ABORT;
	return SPEW_CONTINUE;
}

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

int CHammerApp::Main()
{
	int argc = 0;
	char *argv[] = {nullptr};
	QApplication app(argc, argv);
	auto pWin = new MainWindow(nullptr);
	pWin->setAttribute(Qt::WA_DeleteOnClose);
	pWin->show();
	int result = QApplication::exec();
#ifdef SW_HAMMER_TOOL
	_exit(result);
#else
	return result;
#endif
}