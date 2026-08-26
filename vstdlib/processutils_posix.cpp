//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Linux implementation of process utilities
//
//===========================================================================//

#if defined(POSIX)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>

#include "vstdlib/iprocessutils.h"
#include "tier1/utllinkedlist.h"
#include "tier1/utlstring.h"
#include "tier1/utlbuffer.h"
#include "tier1/tier1.h"

//-----------------------------------------------------------------------------
// At the moment, we can only run one process at a time 
//-----------------------------------------------------------------------------
class CProcessUtils : public CTier1AppSystem< IProcessUtils >
{
    typedef CTier1AppSystem< IProcessUtils > BaseClass;

public:
    CProcessUtils() : BaseClass( false ) {}

    // Inherited from IAppSystem
    virtual InitReturnVal_t Init();
    virtual void Shutdown();

    // Inherited from IProcessUtils
    virtual ProcessHandle_t StartProcess( const char *pCommandLine, bool bConnectStdPipes );
    virtual ProcessHandle_t StartProcess( int argc, const char **argv, bool bConnectStdPipes );
    virtual void CloseProcess( ProcessHandle_t hProcess );
    virtual void AbortProcess( ProcessHandle_t hProcess );
    virtual bool IsProcessComplete( ProcessHandle_t hProcess );
    virtual void WaitUntilProcessCompletes( ProcessHandle_t hProcess );
    virtual int SendProcessInput( ProcessHandle_t hProcess, char *pBuf, int nBufLen );
    virtual int GetProcessOutputSize( ProcessHandle_t hProcess );
    virtual int GetProcessOutput( ProcessHandle_t hProcess, char *pBuf, int nBufLen );
    virtual int GetProcessExitCode( ProcessHandle_t hProcess );

private:
    struct ProcessInfo_t
    {
        // Linux: These are file descriptors (int), not HANDLEs
        int m_nChildStdinRd;    // Read end of stdin pipe (used by parent to write)
        int m_nChildStdinWr;    // Write end of stdin pipe (given to child)
        int m_nChildStdoutRd;   // Read end of stdout pipe (used by parent to read)
        int m_nChildStdoutWr;   // Write end of stdout pipe (given to child)
        int m_nChildStderrWr;   // Write end of stderr pipe (given to child)
        pid_t m_PID;            // Process ID, replaces HANDLE
        CUtlString m_CommandLine;
        CUtlBuffer m_ProcessOutput;
    };

    // Returns the last error that occurred
    char *GetErrorString( char *pBuf, int nBufLen );

    // creates the process, adds it to the list and writes the PID into info.m_PID
    ProcessHandle_t CreateProcess( ProcessInfo_t &info, bool bConnectStdPipes );

    // Shuts down the process handle (closes fds)
    void ShutdownProcess( ProcessHandle_t hProcess );

    // Methods used to read output back from a process
    int GetActualProcessOutputSize( ProcessHandle_t hProcess );
    int GetActualProcessOutput( ProcessHandle_t hProcess, char *pBuf, int nBufLen );

    CUtlFixedLinkedList< ProcessInfo_t >    m_Processes;
    ProcessHandle_t m_hCurrentProcess;
    bool m_bInitialized;
};

//-----------------------------------------------------------------------------
// Purpose: singleton accessor
//-----------------------------------------------------------------------------
static CProcessUtils s_ProcessUtils;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CProcessUtils, IProcessUtils, PROCESS_UTILS_INTERFACE_VERSION, s_ProcessUtils );

//-----------------------------------------------------------------------------
// Initialize, shutdown process system
//-----------------------------------------------------------------------------
InitReturnVal_t CProcessUtils::Init()
{
    InitReturnVal_t nRetVal = BaseClass::Init();
    if ( nRetVal != INIT_OK )
        return nRetVal;

    m_bInitialized = true;
    m_hCurrentProcess = PROCESS_HANDLE_INVALID;
    return INIT_OK;
}

void CProcessUtils::Shutdown()
{
    Assert( m_bInitialized );
    Assert( m_Processes.Count() == 0 );
    if ( m_Processes.Count() != 0 )
    {
        AbortProcess( m_hCurrentProcess );
    }
    m_bInitialized = false;
    return BaseClass::Shutdown();
}

//-----------------------------------------------------------------------------
// Returns the last error that occurred (Linux version)
//-----------------------------------------------------------------------------
char *CProcessUtils::GetErrorString( char *pBuf, int nBufLen )
{
    // strerror_r is more portable, but may not be available everywhere.
    // Use the GNU version that returns a char* or the XSI-compliant one.
#ifdef _GNU_SOURCE
    return strerror_r( errno, pBuf, nBufLen );
#else
    strerror_r( errno, pBuf, nBufLen );
    return pBuf;
#endif
}

//-----------------------------------------------------------------------------
// Creates the process using fork + exec
//-----------------------------------------------------------------------------
ProcessHandle_t CProcessUtils::CreateProcess( ProcessInfo_t &info, bool bConnectStdPipes )
{
    pid_t pid = fork();

    if ( pid == -1 )
    {
        char buf[ 512 ];
        Warning( "Could not fork process for command:\n   %s\n"
                 "System gave the error message:\n   \"%s\"\n",
                 info.m_CommandLine.Get(), GetErrorString( buf, sizeof(buf) ) );
        return PROCESS_HANDLE_INVALID;
    }

    // Child process
    if ( pid == 0 )
    {
        // Set up stdin, stdout, stderr if pipes are used
        if ( bConnectStdPipes )
        {
            // Close the parent's ends of the pipes in the child
            if ( info.m_nChildStdinRd != -1 ) close( info.m_nChildStdinRd );
            if ( info.m_nChildStdoutRd != -1 ) close( info.m_nChildStdoutRd );
            if ( info.m_nChildStderrWr != -1 ) close( info.m_nChildStderrWr );

            // Duplicate pipe ends to standard file descriptors
            if ( info.m_nChildStdinWr != -1 )
            {
                dup2( info.m_nChildStdinWr, STDIN_FILENO );
                close( info.m_nChildStdinWr );
            }
            if ( info.m_nChildStdoutWr != -1 )
            {
                dup2( info.m_nChildStdoutWr, STDOUT_FILENO );
                close( info.m_nChildStdoutWr );
            }
            // stderr points to the same as stdout via a duplicate handle
            if ( info.m_nChildStderrWr != -1 )
            {
                dup2( info.m_nChildStderrWr, STDERR_FILENO );
                close( info.m_nChildStderrWr );
            }
        }

        // Parse the command line into arguments for execvp
        // This is a simplified parsing: it splits on spaces. For robust parsing,
        // consider using a proper shell or a more sophisticated parser.
        char *cmdLine = strdup( info.m_CommandLine.Get() );
        char *argv[128]; // Max 128 arguments
        int argc = 0;
        char *token = strtok( cmdLine, " " );
        while ( token && argc < 127 )
        {
            argv[argc++] = token;
            token = strtok( NULL, " " );
        }
        argv[argc] = NULL;

        // Execute the command
        execvp( argv[0], argv );
        
        // If execvp returns, an error occurred
        fprintf( stderr, "execvp failed for command %s: %s\n", argv[0], strerror( errno ) );
        free( cmdLine );
        _exit( 127 ); // Exit the child process with an error
    }

    // Parent process: record the PID
    info.m_PID = pid;
    m_hCurrentProcess = m_Processes.AddToTail( info );
    return m_hCurrentProcess;
}

//-----------------------------------------------------------------------------
// Options for compilation
//-----------------------------------------------------------------------------
ProcessHandle_t CProcessUtils::StartProcess( const char *pCommandLine, bool bConnectStdPipes )
{
    Assert( m_bInitialized );

    // NOTE: For the moment, we can only run one process at a time
    if ( m_hCurrentProcess != PROCESS_HANDLE_INVALID )
    {
        WaitUntilProcessCompletes( m_hCurrentProcess ); 
    }

    ProcessInfo_t info;
    info.m_CommandLine = pCommandLine;
    info.m_PID = -1;

    // Initialize file descriptors to -1 (invalid)
    info.m_nChildStdinRd = info.m_nChildStdinWr = -1;
    info.m_nChildStdoutRd = info.m_nChildStdoutWr = -1;
    info.m_nChildStderrWr = -1;

    if ( !bConnectStdPipes )
    {
        // No pipe connection, just create the process
        return CreateProcess( info, false );
    }

    // Create pipes for stdin, stdout, stderr
    int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];

    if ( pipe( stdin_pipe ) == -1 )
    {
        Warning( "Failed to create stdin pipe for process: %s\n", strerror( errno ) );
        return PROCESS_HANDLE_INVALID;
    }
    if ( pipe( stdout_pipe ) == -1 )
    {
        Warning( "Failed to create stdout pipe for process: %s\n", strerror( errno ) );
        close( stdin_pipe[0] );
        close( stdin_pipe[1] );
        return PROCESS_HANDLE_INVALID;
    }
    // For stderr, we can just duplicate stdout's write end
    // Or create a separate pipe if we want to read stderr separately.
    // For simplicity, we'll make stderr point to the same pipe as stdout.
    // To simulate Windows' DuplicateHandle behavior, we'll duplicate the stdout write fd.
    if ( dup2( stdout_pipe[1], stderr_pipe[1] ) == -1 )
    {
        Warning( "Failed to duplicate stdout for stderr: %s\n", strerror( errno ) );
        close( stdin_pipe[0] ); close( stdin_pipe[1] );
        close( stdout_pipe[0] ); close( stdout_pipe[1] );
        return PROCESS_HANDLE_INVALID;
    }

    // Store pipe file descriptors in ProcessInfo
    info.m_nChildStdinRd = stdin_pipe[0];   // Parent reads from stdin? Actually parent writes to child's stdin.
    info.m_nChildStdinWr = stdin_pipe[1];   // Parent writes to this, child reads from its stdin.
    info.m_nChildStdoutRd = stdout_pipe[0]; // Parent reads child's stdout from here.
    info.m_nChildStdoutWr = stdout_pipe[1]; // Child writes to this.
    info.m_nChildStderrWr = stderr_pipe[1]; // Child writes stderr to this (same as stdout write end).

    // Set pipes to non-blocking? Might be needed for ReadFile/PeekNamedPipe equivalents.
    // For now, we'll use select/poll to check readability.

    ProcessHandle_t hProcess = CreateProcess( info, true );
    if ( hProcess != PROCESS_HANDLE_INVALID )
    {
        // Close the child's ends of the pipes in the parent
        if ( info.m_nChildStdinWr != -1 ) close( info.m_nChildStdinWr );
        if ( info.m_nChildStdoutWr != -1 ) close( info.m_nChildStdoutWr );
        if ( info.m_nChildStderrWr != -1 ) close( info.m_nChildStderrWr );
        // Keep the parent's ends open for reading/writing
        return hProcess;
    }

    // Clean up on failure
    if ( info.m_nChildStdinRd != -1 ) close( info.m_nChildStdinRd );
    if ( info.m_nChildStdoutRd != -1 ) close( info.m_nChildStdoutRd );
    if ( info.m_nChildStderrWr != -1 ) close( info.m_nChildStderrWr );
    return PROCESS_HANDLE_INVALID;
}

//-----------------------------------------------------------------------------
// Start up a process
//-----------------------------------------------------------------------------
ProcessHandle_t CProcessUtils::StartProcess( int argc, const char **argv, bool bConnectStdPipes )
{
    CUtlString commandLine;
    for ( int i = 0; i < argc; ++i )
    {
        commandLine += argv[i];
        if ( i != argc-1 )
        {
            commandLine += " ";
        }
    }
    return StartProcess( commandLine.Get(), bConnectStdPipes );
}

//-----------------------------------------------------------------------------
// Shuts down the process handle (closes all file descriptors)
//-----------------------------------------------------------------------------
void CProcessUtils::ShutdownProcess( ProcessHandle_t hProcess )
{
    ProcessInfo_t& info = m_Processes[hProcess];
    if ( info.m_nChildStderrWr != -1 ) close( info.m_nChildStderrWr );
    if ( info.m_nChildStdinRd != -1 ) close( info.m_nChildStdinRd );
    if ( info.m_nChildStdinWr != -1 ) close( info.m_nChildStdinWr );
    if ( info.m_nChildStdoutRd != -1 ) close( info.m_nChildStdoutRd );
    if ( info.m_nChildStdoutWr != -1 ) close( info.m_nChildStdoutWr );

    m_Processes.Remove( hProcess );
}

//-----------------------------------------------------------------------------
// Closes the process
//-----------------------------------------------------------------------------
void CProcessUtils::CloseProcess( ProcessHandle_t hProcess )
{
    Assert( m_bInitialized );
    if ( hProcess != PROCESS_HANDLE_INVALID )
    {
        WaitUntilProcessCompletes( hProcess );
        ShutdownProcess( hProcess );
    }
}

//-----------------------------------------------------------------------------
// Aborts the process
//-----------------------------------------------------------------------------
void CProcessUtils::AbortProcess( ProcessHandle_t hProcess )
{
    Assert( m_bInitialized );
    if ( hProcess != PROCESS_HANDLE_INVALID )
    {
        if ( !IsProcessComplete( hProcess ) )
        {
            ProcessInfo_t& info = m_Processes[hProcess];
            kill( info.m_PID, SIGTERM ); // Send SIGTERM to terminate
            // Optionally wait for it to die
            waitpid( info.m_PID, NULL, WNOHANG );
        }
        ShutdownProcess( hProcess );
    }
}

//-----------------------------------------------------------------------------
// Returns true if the process is complete
//-----------------------------------------------------------------------------
bool CProcessUtils::IsProcessComplete( ProcessHandle_t hProcess )
{
    Assert( m_bInitialized );
    Assert( hProcess != PROCESS_HANDLE_INVALID );
    if ( m_hCurrentProcess != hProcess )
        return true;

    pid_t pid = m_Processes[hProcess].m_PID;
    int status;
    pid_t result = waitpid( pid, &status, WNOHANG );
    if ( result == -1 )
    {
        // Error, assume process is gone
        return true;
    }
    return ( result == pid );
}

//-----------------------------------------------------------------------------
// Methods used to write input into a process (unimplemented)
//-----------------------------------------------------------------------------
int CProcessUtils::SendProcessInput( ProcessHandle_t hProcess, char *pBuf, int nBufLen )
{
    // Unimplemented yet
    Assert( 0 );
    return 0;
}

//-----------------------------------------------------------------------------
// Methods used to read output back from a process
//-----------------------------------------------------------------------------
int CProcessUtils::GetActualProcessOutputSize( ProcessHandle_t hProcess )
{
    Assert( hProcess != PROCESS_HANDLE_INVALID );

    ProcessInfo_t& info = m_Processes[ hProcess ];
    if ( info.m_nChildStdoutRd == -1 )
        return 0;

    // Use select to check if data is available without blocking
    fd_set readfds;
    FD_ZERO( &readfds );
    FD_SET( info.m_nChildStdoutRd, &readfds );
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int ret = select( info.m_nChildStdoutRd + 1, &readfds, NULL, NULL, &tv );
    if ( ret <= 0 )
        return 0; // No data or error

    // Check how many bytes are available using FIONREAD (ioctl)
    int bytes_available = 0;
    if ( ioctl( info.m_nChildStdoutRd, FIONREAD, &bytes_available ) == -1 )
    {
        char buf[ 512 ];
        Warning( "Could not check size of pipe associated with command %s\n"
                 "System gave the error message:\n   \"%s\"\n",
                 info.m_CommandLine.Get(), GetErrorString( buf, sizeof(buf) ) );
        return 0;
    }

    // Add 1 for auto-NULL termination
    return ( bytes_available > 0 ) ? bytes_available + 1 : 0;
}

int CProcessUtils::GetActualProcessOutput( ProcessHandle_t hProcess, char *pBuf, int nBufLen )
{
    ProcessInfo_t& info = m_Processes[ hProcess ];
    if ( info.m_nChildStdoutRd == -1 )
        return 0;

    // First, get the number of bytes available
    int bytes_available = 0;
    if ( ioctl( info.m_nChildStdoutRd, FIONREAD, &bytes_available ) == -1 )
    {
        char buf[ 512 ];
        Warning( "Could not read from pipe associated with command %s\n"
                 "System gave the error message:\n   \"%s\"\n",
                 info.m_CommandLine.Get(), GetErrorString( buf, sizeof(buf) ) );
        return 0;
    }

    int bytes_to_read = min( bytes_available, nBufLen - 1 );
    if ( bytes_to_read <= 0 )
        return 0;

    // Read the data
    ssize_t nRead = read( info.m_nChildStdoutRd, pBuf, bytes_to_read );
    if ( nRead <= 0 )
        return 0;

    // No need to convert \r\n to \n on Linux, but we can keep the same behavior
    // For consistency, we'll do a simple conversion if needed
    // Actually, Linux programs usually output \n only, so we can leave as is.

    return (int)nRead;
}

int CProcessUtils::GetProcessOutputSize( ProcessHandle_t hProcess )
{
    Assert( m_bInitialized );
    if ( hProcess == PROCESS_HANDLE_INVALID )
        return 0;

    return GetActualProcessOutputSize( hProcess ) + m_Processes[hProcess].m_ProcessOutput.TellPut();
}

int CProcessUtils::GetProcessOutput( ProcessHandle_t hProcess, char *pBuf, int nBufLen )
{
    Assert( m_bInitialized );

    if ( hProcess == PROCESS_HANDLE_INVALID )
        return 0;

    ProcessInfo_t &info = m_Processes[hProcess];
    int nCachedBytes = info.m_ProcessOutput.TellPut();
    int nBytesRead = 0;
    if ( nCachedBytes )
    {
        nBytesRead = min( nBufLen-1, nCachedBytes );
        info.m_ProcessOutput.Get( pBuf, nBytesRead );
        pBuf[ nBytesRead ] = 0;
        nBufLen -= nBytesRead;
        pBuf += nBytesRead;
        if ( info.m_ProcessOutput.GetBytesRemaining() == 0 )
        {
            info.m_ProcessOutput.Purge();
        }

        if ( nBufLen <= 1 )
            return nBytesRead;
    }

    // Auto-NULL terminate
    int nActualCountRead = GetActualProcessOutput( hProcess, pBuf, nBufLen );
    pBuf[nActualCountRead] = 0;
    return nActualCountRead + nBytesRead + 1;
}

//-----------------------------------------------------------------------------
// Returns the exit code for the process
//-----------------------------------------------------------------------------
int CProcessUtils::GetProcessExitCode( ProcessHandle_t hProcess )
{
    Assert( m_bInitialized );
    ProcessInfo_t &info = m_Processes[hProcess];
    int status;
    pid_t result = waitpid( info.m_PID, &status, WNOHANG );
    if ( result == -1 || result == 0 )
        return -1; // Process still active or error

    if ( WIFEXITED( status ) )
        return WEXITSTATUS( status );
    else if ( WIFSIGNALED( status ) )
        return -1; // Terminated by signal

    return -1;
}

//-----------------------------------------------------------------------------
// Waits until a process is complete
//-----------------------------------------------------------------------------
void CProcessUtils::WaitUntilProcessCompletes( ProcessHandle_t hProcess )
{
    Assert( m_bInitialized );

    if ( ( hProcess == PROCESS_HANDLE_INVALID ) || ( m_hCurrentProcess != hProcess ) )
        return;

    ProcessInfo_t &info = m_Processes[ hProcess ];

    if ( info.m_nChildStdoutRd == -1 )
    {
        // No pipes: just wait for the process
        waitpid( info.m_PID, NULL, 0 );
    }
    else
    {
        // With pipes: we need to read output while waiting to avoid blocking the child
        fd_set readfds;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms timeout

        while ( waitpid( info.m_PID, NULL, WNOHANG ) == 0 )
        {
            // Check if there's data to read from stdout
            FD_ZERO( &readfds );
            FD_SET( info.m_nChildStdoutRd, &readfds );
            int ret = select( info.m_nChildStdoutRd + 1, &readfds, NULL, NULL, &tv );
            if ( ret > 0 )
            {
                int nLen = GetActualProcessOutputSize( hProcess );
                if ( nLen > 0 )
                {
                    int nPut = info.m_ProcessOutput.TellPut();
                    info.m_ProcessOutput.EnsureCapacity( nPut + nLen );
                    int nBytesRead = GetActualProcessOutput( hProcess, (char*)info.m_ProcessOutput.PeekPut(), nLen );
                    info.m_ProcessOutput.SeekPut( CUtlBuffer::SEEK_HEAD, nPut + nBytesRead );
                }
            }
        }
    }

    m_hCurrentProcess = PROCESS_HANDLE_INVALID;
}

#endif // POSIX