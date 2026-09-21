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
#include "appframework/ilaunchermgr.h"

//-----------------------------------------------------------------------------
// Global systems
//-----------------------------------------------------------------------------
IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

extern void* CreateSDLMgr();

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

	CreateSDLMgr();

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
			{ "vguimatsurface.dll",		VGUI_SURFACE_INTERFACE_VERSION },
			{ "vgui2.dll",				VGUI_IVGUI_INTERFACE_VERSION },			
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