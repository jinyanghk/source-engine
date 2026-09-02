
#include <QApplication>
#include <QString>
#include <QDebug>
#include <dlfcn.h>

#include "mainwindow.h"

// Core Source Engine Interfaces
#include "materialsystem/imaterialsystem.h"
#include "filesystem.h"
#include "interface.h"

// Define our global module handles and system pointers locally
IMaterialSystem* g_pMaterialSystem = nullptr;
IFileSystem*    g_pFileSystem = nullptr;

// Opaque stub factory function to pass into the Connect() method
static void* LauncherInterfaceFactory( const char *pName, int *pReturnCode )
{
    if ( pReturnCode ) 
        *pReturnCode = 0; // IFACE_OK

    if ( strcmp( pName, FILESYSTEM_INTERFACE_VERSION ) == 0 )
    {
        return static_cast<IFileSystem*>( g_pFileSystem );
    }

    return nullptr;
}

bool InitializeSourceEngineSubsystems()
{
    int status = 0;

    // 1. Resolve filesystem_stdio via native dlopen layout
    void* hFileSystemModule = dlopen( "bin/libfilesystem_stdio.so", RTLD_NOW | RTLD_GLOBAL );
    if ( !hFileSystemModule )
    {
        qCritical() << "Failed to dlopen libfilesystem_stdio.so:" << dlerror();
        return false;
    }
    
    CreateInterfaceFn fsFactory = (CreateInterfaceFn)dlsym( hFileSystemModule, "CreateInterface" );
    if ( !fsFactory ) return false;

    g_pFileSystem = (IFileSystem*)fsFactory( FILESYSTEM_INTERFACE_VERSION, &status );
    if ( !g_pFileSystem || status != 0 ) return false;

    if ( !g_pFileSystem->Connect( LauncherInterfaceFactory ) || g_pFileSystem->Init() != INIT_OK )
    {
        qCritical() << "Failed to initialize FileSystem system link!";
        return false;
    }

    // 2. Resolve materialsystem
    void* hMaterialSystemModule = dlopen( "bin/libmaterialsystem.so", RTLD_NOW | RTLD_GLOBAL );
    if ( !hMaterialSystemModule )
    {
        qCritical() << "Failed to dlopen libmaterialsystem.so:" << dlerror();
        return false;
    }

    CreateInterfaceFn matFactory = (CreateInterfaceFn)dlsym( hMaterialSystemModule, "CreateInterface" );
    if ( !matFactory ) return false;

    g_pMaterialSystem = (IMaterialSystem*)matFactory( MATERIAL_SYSTEM_INTERFACE_VERSION, &status );
    if ( !g_pMaterialSystem || status != 0 ) 
    {
        qCritical() << "Material System interface instantiation failed! Status:" << status;
        return false;
    }

    // 3. Connect to the Material System
    qInfo() << "Connecting to Material System instance...";
    
    // Fix: Corrected typo structure where 'if' statement was malformed
    if ( !g_pMaterialSystem->Connect( LauncherInterfaceFactory ) )
    {
        qCritical() << "Failed to Connect to Material System!";
        return false;
    }

    qInfo() << "Initializing Material System context...";
    if ( g_pMaterialSystem->Init() != INIT_OK )
    {
        qCritical() << "Failed to Init Material System!";
        return false;
    }

    g_pMaterialSystem->SetShaderAPI( "shaderapidx9" );
    return true;
}

int main(int argc, char** argv)  {
	QApplication app(argc, argv);

	auto pWin = new MainWindow(nullptr);
    pWin->setAttribute(Qt::WA_DeleteOnClose);
    pWin->show();
	
	return QApplication::exec();
}