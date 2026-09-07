#pragma once

// Core Source Engine Interfaces
#include "appframework/AppFramework.h"
#include "IHammer.h"
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
#include "vgui/IVGui.h"
#include "vgui/ISurface.h"
#include "inputsystem/iinputsystem.h"
#include "tier0/icommandline.h"
#include "p4lib/ip4.h"

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

private:
	int	MainLoop();
};

//-----------------------------------------------------------------------------
// Global systems
//-----------------------------------------------------------------------------
extern IHammer *g_pHammer;
extern IMaterialSystem *g_pMaterialSystem;
extern IFileSystem *g_pFileSystem;
extern IDataCache *g_pDataCache;
extern IInputSystem *g_pInputSystem;

//-----------------------------------------------------------------------------
// Define the application object
//-----------------------------------------------------------------------------
extern CHammerApp g_ApplicationObject;