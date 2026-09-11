#include <QApplication>

#include <unistd.h> // Required for _exit() on Linux / POSIX systems

#include "mainwindow.h"

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
// Global systems
//-----------------------------------------------------------------------------
//IHammer *g_pHammer;
IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

#if defined(USE_SDL)
#include "appframework/ilaunchermgr.h"
//ILauncherMgr *g_pLauncherMgr = NULL;	// set in CMaterialSystem::Connect

// Forward declare the class used by the return type
class GLMDisplayDB;
struct CShowPixelsParams;
struct CStackCrawlParams;
struct SDL_Cursor;

class CDummyLauncherMgr : public ILauncherMgr
{
public:
    // Core crash fix: Return an empty data block instead of NULL
    virtual GLMDisplayDB* GetDisplayDB() override 
    { 
        static char dummyDisplayDB = {0}; 
        return reinterpret_cast<GLMDisplayDB*>(&dummyDisplayDB); 
    }

    // Fixed methods
    virtual bool CreateGameWindow( const char *pTitle, bool bWindowed, int nWidth, int nHeight ) override { return true; }

    // Pure virtual stubs required to make the class instantiable
    virtual bool Connect( CreateInterfaceFn factory ) override { return true; }
    virtual void Disconnect() override {}
    virtual void *QueryInterface( const char *pInterfaceName ) override { return nullptr; }
    virtual InitReturnVal_t Init() override { return INIT_OK; }
    virtual void Shutdown() override {}
    virtual void IncWindowRefCount() override {}
    virtual void DecWindowRefCount() override {}
    virtual int GetEvents( CCocoaEvent *pEvents, int nMaxEventsToReturn, bool debugEvents = false ) override { return 0; }
    virtual int PeekAndRemoveKeyboardEvents( bool *pbEsc, bool *pbReturn, bool *pbSpace, bool debugEvents = false ) override { return 0; }
    virtual void SetCursorPosition( int x, int y ) override {}
    virtual void SetWindowFullScreen( bool bFullScreen, int nWidth, int nHeight ) override {}
    virtual bool IsWindowFullScreen() override { return false; }
    virtual void MoveWindow( int x, int y ) override {}
    virtual void SizeWindow( int width, int tall ) override {}
    virtual void PumpWindowsMessageLoop() override {}
    virtual void DestroyGameWindow() override {}
    virtual void SetApplicationIcon( const char *pchAppIconFile ) override {}
    virtual void GetMouseDelta( int &x, int &y, bool bIgnoreNextMouseDelta = false ) override {}
    virtual void GetNativeDisplayInfo( int nDisplay, uint &nWidth, uint &nHeight, uint &nRefreshHz ) override { nWidth = 1920; nHeight = 1080; nRefreshHz = 60; }
    virtual void RenderedSize( uint &width, uint &height, bool set ) override {}
    virtual void DisplayedSize( uint &width, uint &height) override {}
    virtual PseudoGLContextPtr GetMainContext() override { return nullptr; }
    virtual PseudoGLContextPtr GetGLContextForWindow( void* windowref ) override { return nullptr; }
    virtual PseudoGLContextPtr CreateExtraContext() override { return nullptr; }
    virtual void DeleteContext( PseudoGLContextPtr hContext ) override {}
    virtual bool MakeContextCurrent( PseudoGLContextPtr hContext ) override { return true; }
    virtual void GetDesiredPixelFormatAttribsAndRendererInfo( uint **ptrOut, uint *countOut, GLMRendererInfoFields *rendInfoOut ) override {}

    // Newly added remaining stubs from the compiler log
    virtual void ShowPixels( CShowPixelsParams *params ) override {}
    virtual void GetStackCrawl( CStackCrawlParams *params ) override {}
    virtual void WaitUntilUserInput( int msSleepTime ) override {}
    virtual void *GetWindowRef() override { return nullptr; }
    virtual void SetMouseVisible( bool bState ) override {}
    virtual void SetMouseCursor( SDL_Cursor *hCursor ) override {}
    virtual void SetForbidMouseGrab( bool bForbidMouseGrab ) override {}
    virtual void OnFrameRendered() override {}
    virtual void SetGammaRamp( const uint16 *pRed, const uint16 *pGreen, const uint16 *pBlue ) override {}
    virtual double GetPrevGLSwapWindowTime() override { return 0.0; }
};

static CDummyLauncherMgr s_DummyLauncherMgr;

// This factory function intercepts requests for SDLMgrInterface001
void* HammerExtraFactory( const char *pInterfaceName, int *pReturnCode )
{
    if ( strcmp( pInterfaceName, "SDLMgrInterface001" ) == 0 )
    {
        if ( pReturnCode ) *pReturnCode = 0; // IFACE_OK
        return &s_DummyLauncherMgr;
    }
    if ( pReturnCode ) *pReturnCode = 1; // IFACE_FAILED
    return nullptr;
}

// This macro registers your dummy instance directly to the engine's local CreateInterface factory
//EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CDummyLauncherMgr, ILauncherMgr, "SDLMgrInterface001", s_DummyLauncherMgr );
#endif

//-----------------------------------------------------------------------------
// The application object
//-----------------------------------------------------------------------------
class CHammerApp : public CAppSystemGroup
{
public:
	// Methods of IApplication
	virtual bool Create( );
	virtual bool PreInit( );
	virtual int Main( );
	virtual void PostShutdown();
	virtual void Destroy();
};

//-----------------------------------------------------------------------------
// Define the application object
//-----------------------------------------------------------------------------
CHammerApp	g_ApplicationObject;

int main( int argc, char *argv[] )
{
    CommandLine()->CreateCmdLine( argc, argv );

    // FIX: Instead of calling Create/PreInit manually, pass control over to Run().
    // This activates ConnectSystems(), waking up the shader, inputsystem, and material contexts!
    return g_ApplicationObject.Run();
}

//-----------------------------------------------------------------------------
// Create all singleton systems
//-----------------------------------------------------------------------------
bool CHammerApp::Create( )
{
	// Save some memory so engine/hammer isn't so painful
	CommandLine()->AppendParm( "-disallowhwmorph", NULL );

    // 1. Manually instantiate your dummy launcher manager onto the global engine scope
    static CDummyLauncherMgr s_DummyLauncherMgr;
    g_pLauncherMgr = &s_DummyLauncherMgr;

	IAppSystem *pSystem;

	// Add in the cvar factory
	AppModule_t cvarModule = LoadModule( VStdLib_GetICVarFactory() );
	pSystem = AddSystem( cvarModule, CVAR_INTERFACE_VERSION );
	if ( !pSystem )
		return false;
	
	bool bSteam;
	char pFileSystemDLL[MAX_PATH];
	if ( FileSystem_GetFileSystemDLLName( pFileSystemDLL, MAX_PATH, bSteam ) != FS_OK )
		return false;

	AppModule_t fileSystemModule = LoadModule( pFileSystemDLL );
	g_pFileSystem = (IFileSystem*)AddSystem( fileSystemModule, FILESYSTEM_INTERFACE_VERSION );

	FileSystem_SetBasePaths( g_pFileSystem );

	AppSystemInfo_t appSystems[] = 
	{
		{ "materialsystem.dll",		MATERIAL_SYSTEM_INTERFACE_VERSION },
		{ "inputsystem.dll",		INPUTSYSTEM_INTERFACE_VERSION },
		{ "studiorender.dll",		STUDIO_RENDER_INTERFACE_VERSION },
		{ "vphysics.dll",			VPHYSICS_INTERFACE_VERSION },
		{ "datacache.dll",			DATACACHE_INTERFACE_VERSION },
		{ "datacache.dll",			MDLCACHE_INTERFACE_VERSION },
		{ "datacache.dll",			STUDIO_DATA_CACHE_INTERFACE_VERSION },
		//{ "vguimatsurface.dll",		VGUI_SURFACE_INTERFACE_VERSION },
		//{ "vgui2.dll",				VGUI_IVGUI_INTERFACE_VERSION },
		//{ "hammer_dll.dll",			INTERFACEVERSION_HAMMER },
		{ "", "" }	// Required to terminate the list
	};

	if ( !AddSystems( appSystems ) ) 
		return false;

	// Connect to interfaces loaded in AddSystems that we need locally
	g_pMaterialSystem = (IMaterialSystem*)FindSystem( MATERIAL_SYSTEM_INTERFACE_VERSION );
	//g_pHammer = (IHammer*)FindSystem( INTERFACEVERSION_HAMMER );
	g_pDataCache = (IDataCache*)FindSystem( DATACACHE_INTERFACE_VERSION );
	g_pInputSystem = (IInputSystem*)FindSystem( INPUTSYSTEM_INTERFACE_VERSION );
	g_pStudioRender = (IStudioRender*)FindSystem( STUDIO_RENDER_INTERFACE_VERSION );
	g_pMDLCache = (IMDLCache*)FindSystem( MDLCACHE_INTERFACE_VERSION );
	
	if ( !g_pLauncherMgr )
	{
		static CDummyLauncherMgr s_DummyLauncherMgr;
		g_pLauncherMgr = &s_DummyLauncherMgr;
	}

	// This has to be done before connection.
	//g_pMaterialSystem->SetShaderAPI( "shaderapiempty.dll" );
    g_pMaterialSystem->SetShaderAPI( "shaderapidx9.dll" );

	return true;
}

void CHammerApp::Destroy()
{
	g_pFileSystem = NULL;
	g_pMaterialSystem = NULL;
	g_pDataCache = NULL;
	//g_pHammer = NULL;
	g_pInputSystem = NULL;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
SpewRetval_t HammerSpewFunc( SpewType_t type, tchar const *pMsg )
{
	if ( type == SPEW_ASSERT )
	{
		return SPEW_DEBUGGER;
	}
	else if( type == SPEW_ERROR )
	{
		//MessageBox( NULL, pMsg, "Hammer Error", MB_OK | MB_ICONSTOP );
		Msg ("Hammer Error %s\n", pMsg);
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
    printf ("GetVProjectCmdLineValue() = %s\n", GetVProjectCmdLineValue());

	//
	// Init the game and mod dirs in the file system.
	// This needs to happen before calling Init on the material system.
	//
	CFSSearchPathsInit initInfo;
	initInfo.m_pFileSystem = g_pFileSystem;
	initInfo.m_pDirectoryName = "hl2";

	if ( FileSystem_LoadSearchPaths( initInfo ) != FS_OK )
	{
		Error( "Unable to load search paths!\n" );
	}

	// Required to run through the editor
	g_pMaterialSystem->EnableEditorMaterials();

	// needed for VGUI model rendering
	g_pMaterialSystem->SetAdapter( 0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE );

	return true; 
}

void CHammerApp::PostShutdown()
{
}


//-----------------------------------------------------------------------------
// main application
//-----------------------------------------------------------------------------
int CHammerApp::Main( )
{
    // FIX: Boot up the Qt Framework workspace window context inside Main().
    // At this precise moment, every single engine module is 100% connected, alive,
    // and ready to draw graphics variables!
    int argc = 0;
    char *argv[] = { nullptr };
    QApplication app( argc, argv );

    auto pWin = new MainWindow( nullptr );
    pWin->setAttribute( Qt::WA_DeleteOnClose );
    pWin->show();
    
    int result = QApplication::exec();

#ifdef SW_HAMMER_TOOL
    // SW_HAMMER_TOOL SHUTDOWN SHIELD: Bypassing legacy global destructors 
    // inside tier0 / materialsystem libraries to prevent invalid memory 
    // free passes during shared library unloading cycles.
    _exit( result ); 
#else
    return result;
#endif
}
