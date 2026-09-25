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

// Global systems pointers
IMaterialSystem *g_pMaterialSystem = nullptr;
IFileSystem *g_pFileSystem = nullptr;
IDataCache *g_pDataCache = nullptr;
IInputSystem *g_pInputSystem = nullptr;
IStudioRender *g_pStudioRender = nullptr;
IMDLCache *g_pMDLCache = nullptr;

extern void* CreateSDLMgr();

// Global handle to hold our active UI window context across steps
static MainWindow* g_pHammerMainWindow = nullptr;

class CHammerApp : public CAppSystemGroup
{
public:
	virtual bool Create() override;
	virtual bool PreInit() override;
	virtual int Main() override;
	virtual void PostShutdown() override;
	virtual void Destroy() override;
};

CHammerApp g_ApplicationObject;

int main(int argc, char *argv[])
{
	// 1. START THE QT FRAMEWORK ECOSYSTEM FIRST
	CommandLine()->CreateCmdLine(argc, argv);
	QApplication app(argc, argv);

	// 2. CONSTRUCT THE MAIN WINDOW AND DISPLAY SHELL
	g_pHammerMainWindow = new MainWindow(nullptr);
	g_pHammerMainWindow->setAttribute(Qt::WA_DeleteOnClose);
	g_pHammerMainWindow->show();

	// 3. EXTRACT THE WINDOW ID FROM THE CANVAS CONTAINER
	// We call a public getter on MainWindow to retrieve our placeholder widget's handle
	unsigned long long canvasWinId = g_pHammerMainWindow->GetViewportWindowID();
	
	if (canvasWinId != 0)
	{
		char paramBuffer[64];
		snprintf(paramBuffer, sizeof(paramBuffer), "%llu", canvasWinId);
		
		// Push the parent configuration token down into the core engine boot parameters pool
		CommandLine()->AppendParm("-parentWindow", paramBuffer);
		Msg("[Bootloader] Successfully routed parent window token parameter down: -parentWindow %s\n", paramBuffer);
	}

	// 4. HAND CONTROL OVER TO VALVE'S APPFRAMEWORK ENGINE BOOT SYSTEM
	return g_ApplicationObject.Run();
}

bool CHammerApp::Create()
{
	CommandLine()->AppendParm("-hammer", NULL);

	AppModule_t cvarModule = LoadModule(VStdLib_GetICVarFactory());
	IAppSystem* pSystem = AddSystem(cvarModule, CVAR_INTERFACE_VERSION);
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
			{"vguimatsurface.dll", VGUI_SURFACE_INTERFACE_VERSION},
			{"vgui2.dll", VGUI_IVGUI_INTERFACE_VERSION},			
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
	// 5. PUMP THE QT APPLICATION LOOP NATIVELY HERE
	int result = QApplication::exec();
	_exit(result);
	return result;
}
