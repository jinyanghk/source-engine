//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#define	USED

#ifdef _WIN32
#include <windows.h>
#endif

#if defined ( POSIX )
#include <thread>

#include <pthread.h>
#include <sched.h>

#define IDLE_PRIORITY_CLASS SCHED_IDLE
#define THREAD_PRIORITY_LOWEST 0
#define THREAD_PRIORITY_IDLE 0

typedef pthread_mutex_t CRITICAL_SECTION;

inline void InitializeCriticalSection(CRITICAL_SECTION* ps)
{
   pthread_mutex_init(ps, NULL);
}

inline void DeleteCriticalSection(CRITICAL_SECTION* ps)
{
   pthread_mutex_destroy(ps);
}

inline void EnterCriticalSection(CRITICAL_SECTION* ps)
{
   pthread_mutex_lock(ps);
}

inline void LeaveCriticalSection(CRITICAL_SECTION* ps)
{
   pthread_mutex_unlock(ps);
}

inline int GetCurrentProcess()
{
	return pthread_self();
}

inline void SetPriorityClass(int pid, int policy)
{
	//int policy = SCHED_IDLE;
    struct sched_param param;
	param.sched_priority = 0; // Priority must be 0 for SCHED_IDLE

	pthread_setschedparam(pid, policy, &param);
}

inline void SetThreadPriority(int pid, int priority)
{
	pthread_setschedprio(pid, priority);
}

#endif

#include "cmdlib.h"
#define NO_THREAD_NAMES
#include "threads.h"
#include "pacifier.h"

#define	MAX_THREADS	16


class CRunThreadsData
{
public:
	int m_iThread;
	void *m_pUserData;
	RunThreadsFn m_Fn;
};

CRunThreadsData g_RunThreadsData[MAX_THREADS];


int		dispatch;
int		workcount;
qboolean		pacifier;

qboolean	threaded;
bool g_bLowPriorityThreads = false;

#ifdef _WIN32
HANDLE g_ThreadHandles[MAX_THREADS];
#endif

#if defined ( POSIX )
pthread_t g_ThreadHandles[MAX_THREADS];
#endif


/*
=============
GetThreadWork

=============
*/
int	GetThreadWork (void)
{
	int	r;

	ThreadLock ();

	if (dispatch == workcount)
	{
		ThreadUnlock ();
		return -1;
	}

	UpdatePacifier( (float)dispatch / workcount );

	r = dispatch;
	dispatch++;
	ThreadUnlock ();

	return r;
}


ThreadWorkerFn workfunction;

void ThreadWorkerFunction( int iThread, void *pUserData )
{
	int		work;

	while (1)
	{
		work = GetThreadWork ();
		if (work == -1)
			break;
		 
		workfunction( iThread, work );
	}
}

void RunThreadsOnIndividual (int workcnt, qboolean showpacifier, ThreadWorkerFn func)
{
	if (numthreads == -1)
		ThreadSetDefault ();
	
	workfunction = func;
	RunThreadsOn (workcnt, showpacifier, ThreadWorkerFunction);
}


/*
===================================================================

WIN32

===================================================================
*/

int		numthreads = -1;
CRITICAL_SECTION		crit;
static int enter;


class CCritInit
{
public:
	CCritInit()
	{
		InitializeCriticalSection (&crit);
	}
} g_CritInit;



void SetLowPriority()
{
	SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
}


void ThreadSetDefault (void)
{
#ifdef _WIN32
	SYSTEM_INFO info;

	if (numthreads == -1)	// not set manually
	{
		GetSystemInfo (&info);
		numthreads = info.dwNumberOfProcessors;
		if (numthreads < 1 || numthreads > 32)
			numthreads = 1;
	}
#endif

#if defined ( POSIX )
	// numthreads = 16;
	numthreads = std::thread::hardware_concurrency();
#endif
	Msg ("%i threads\n", numthreads);
}


void ThreadLock (void)
{
	if (!threaded)
		return;
	EnterCriticalSection (&crit);
	if (enter)
		Error ("Recursive ThreadLock\n");
	enter = 1;
}

void ThreadUnlock (void)
{
	if (!threaded)
		return;
	if (!enter)
		Error ("ThreadUnlock without lock\n");
	enter = 0;
	LeaveCriticalSection (&crit);
}

#ifdef _WIN32
// This runs in the thread and dispatches a RunThreadsFn call.
DWORD WINAPI InternalRunThreadsFn( LPVOID pParameter )
#endif
#if defined ( POSIX )
void* InternalRunThreadsFn( void* pParameter )
#endif
{
	CRunThreadsData *pData = (CRunThreadsData*)pParameter;
	pData->m_Fn( pData->m_iThread, pData->m_pUserData );
	return 0;
}


void RunThreads_Start( RunThreadsFn fn, void *pUserData, ERunThreadsPriority ePriority )
{
	Assert( numthreads > 0 );
	threaded = true;

	if ( numthreads > MAX_TOOL_THREADS )
		numthreads = MAX_TOOL_THREADS;

	for ( int i=0; i < numthreads ;i++ )
	{
		g_RunThreadsData[i].m_iThread = i;
		g_RunThreadsData[i].m_pUserData = pUserData;
		g_RunThreadsData[i].m_Fn = fn;

#ifdef _WIN32
		DWORD dwDummy;
		g_ThreadHandles[i] = CreateThread(
		   NULL,	// LPSECURITY_ATTRIBUTES lpsa,
		   0,		// DWORD cbStack,
		   InternalRunThreadsFn,	// LPTHREAD_START_ROUTINE lpStartAddr,
		   &g_RunThreadsData[i],	// LPVOID lpvThreadParm,
		   0,			// DWORD fdwCreate,
		   &dwDummy );
#endif

#if defined ( POSIX )
		pthread_create( &g_ThreadHandles[i], NULL, InternalRunThreadsFn, &g_RunThreadsData[i]);
#endif

		if ( ePriority == k_eRunThreadsPriority_UseGlobalState )
		{
			if( g_bLowPriorityThreads )
				SetThreadPriority( g_ThreadHandles[i], THREAD_PRIORITY_LOWEST );
		}
		else if ( ePriority == k_eRunThreadsPriority_Idle )
		{
			SetThreadPriority( g_ThreadHandles[i], THREAD_PRIORITY_IDLE );
		}
	}
}


void RunThreads_End()
{
#ifdef _WIN32
	WaitForMultipleObjects( numthreads, g_ThreadHandles, TRUE, INFINITE );
	for ( int i=0; i < numthreads; i++ )
		CloseHandle( g_ThreadHandles[i] );
#endif
	threaded = false;
}
	

/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn( int workcnt, qboolean showpacifier, RunThreadsFn fn, void *pUserData )
{
	int		start, end;

	start = Plat_FloatTime();
	dispatch = 0;
	workcount = workcnt;
	StartPacifier("");
	pacifier = showpacifier;

#ifdef _PROFILE
	threaded = false;
	(*func)( 0 );
	return;
#endif

	
	RunThreads_Start( fn, pUserData );
	RunThreads_End();


	end = Plat_FloatTime();
	if (pacifier)
	{
		EndPacifier(false);
		printf (" (%i)\n", end-start);
	}
}


