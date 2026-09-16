#include <QApplication>

#include <unistd.h> // Required for _exit() on Linux / POSIX systems

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

//-----------------------------------------------------------------------------
// CDummyLauncherMgr
//-----------------------------------------------------------------------------
#include "appframework/ilaunchermgr.h"

class GLMDisplayDB;
struct CShowPixelsParams;
struct CStackCrawlParams;
struct SDL_Cursor;

class CDummyLauncherMgr : public ILauncherMgr
{
private:
	void *m_pActiveWindowRef;
	void *m_pActiveGLContext;
	uint m_nRenderWidth;
	uint m_nRenderHeight;

public:
	CDummyLauncherMgr()
		: m_pActiveWindowRef(nullptr),
		  m_nRenderWidth(1024),
		  m_nRenderHeight(768)
	{
		static char s_MockGLContextData[512] = {0};
		m_pActiveGLContext = reinterpret_cast<void *>(&s_MockGLContextData[0]);
	}

	void SetActiveWindowRef(void *pWindowRef) { m_pActiveWindowRef = pWindowRef; }
	void UpdateRenderSize(uint w, uint h)
	{
		m_nRenderWidth = w;
		m_nRenderHeight = h;
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
	virtual void PumpWindowsMessageLoop() override {}
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
};

CDummyLauncherMgr s_DummyLauncherMgr;

extern "C" void Hammer_SetLauncherWindowContext(void *pWindowRef, int width, int height)
{
	s_DummyLauncherMgr.SetActiveWindowRef(pWindowRef);
	s_DummyLauncherMgr.UpdateRenderSize(static_cast<uint>(width), static_cast<uint>(height));
}

//-----------------------------------------------------------------------------
// Global systems
//-----------------------------------------------------------------------------
IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

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

	static CDummyLauncherMgr s_DummyLauncherMgr;
	g_pLauncherMgr = &s_DummyLauncherMgr;

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

	if (!g_pLauncherMgr)
	{
		static CDummyLauncherMgr s_DummyLauncherMgr;
		g_pLauncherMgr = &s_DummyLauncherMgr;
	}

	g_pMaterialSystem->SetShaderAPI("shaderapidx9.dll");

	return true;
}

void CHammerApp::Destroy()
{
	g_pFileSystem = NULL;
	g_pMaterialSystem = NULL;
	g_pDataCache = NULL;
	g_pInputSystem = NULL;
}

//-----------------------------------------------------------------------------
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
bool CHammerApp::PreInit( )
{
	SpewOutputFunc( HammerSpewFunc );
    //printf ("GetVProjectCmdLineValue() = %s\n", GetVProjectCmdLineValue());

	CFSSearchPathsInit initInfo;
	initInfo.m_pFileSystem = g_pFileSystem;
	initInfo.m_pDirectoryName = "hl2";

	if ( FileSystem_LoadSearchPaths( initInfo ) != FS_OK )
	{
		Error( "Unable to load search paths!\n" );
	}

    // ---- FIX: MOUNT VPK PACKAGES THROUGH VALIDATED FILESYSTEM INTERFACE ----
    // We add the archive search paths directly through g_pFileSystem.
    // This makes the engine map the inside of the VPK texture bundles right to the "GAME" path id pool.
    if (g_pFileSystem)
    {
        g_pFileSystem->AddSearchPath("hl2/hl2_textures.vpk", "GAME");
        g_pFileSystem->AddSearchPath("hl2/hl2_misc.vpk", "GAME");
        g_pFileSystem->AddSearchPath("hl2", "GAME");
    }

	g_pMaterialSystem->EnableEditorMaterials();
	g_pMaterialSystem->SetAdapter( 0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE );

	return true; 
}

void CHammerApp::PostShutdown() {}
//-----------------------------------------------------------------------------
// main application
//-----------------------------------------------------------------------------
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