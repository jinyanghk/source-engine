#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#if defined(SDL_VIDEO_DRIVER_X11)
#include <X11/Xlib.h>
#endif

#include "appframework/ilaunchermgr.h"

extern ILauncherMgr *g_pLauncherMgr;

extern "C" Window ExtractX11WindowFromLauncher()
{
    if (!g_pLauncherMgr) 
		return 0;
    else
    {
        // On nillerusr's engine, GetWindowRef() returns the underlying SDL_Window*
        SDL_Window* pSDLWindow = (SDL_Window*)g_pLauncherMgr->GetWindowRef();
        if (pSDLWindow)
        {
            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version);
            if (SDL_GetWindowWMInfo(pSDLWindow, &wmInfo))
            {
                #if defined(SDL_VIDEO_DRIVER_X11)
                if (wmInfo.subsystem == SDL_SYSWM_X11)
                {
                    return wmInfo.info.x11.window;
                }
                #endif
            }
        }
    }
    return 0;
}
