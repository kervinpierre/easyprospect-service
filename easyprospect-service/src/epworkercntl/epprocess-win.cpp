#include <epworkercntl/epprocess-win.h>
#include <sstream>

bool process_win::is_running() const
{
    bool result = false;
    auto res    = WaitForSingleObject(proc_thread_handle, 0);

    switch (res)
    {
    case WAIT_OBJECT_0:
        break;

    case WAIT_FAILED:
        break;

    case WAIT_TIMEOUT:
        result = true;
        break;
    }

    return result;
}

void process_win::setup()
{
    ;
}

DWORD WINAPI process_win::run_process_thread(void* vptr)
{
    auto* wcntl = static_cast<process_win*>(vptr);

    wcntl->listen_loop();

    return 0;
}

void process_win::start()
{
    proc_thread_handle = CreateThread(
        NULL,               // default security attributes
        0,                  // use default stack size
        run_process_thread, // thread function name
        this,               // argument to thread function
        0,                  // use default creation flags
        &proc_thread_id);
}

void process_win::listen_loop()
{
    STARTUPINFOA        si;
    SECURITY_ATTRIBUTES sa;

    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output?redirectedfrom=MSDN

    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle       = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!MyCreatePipeEx(&output_handles_a[0], &output_handles_b[0], &sa, 0, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED))
    {
        spdlog::error("Error creating pipe for stdout");
        throw std::logic_error("Pipe error");
    }

    if (!MyCreatePipeEx(&output_handles_a[1], &output_handles_b[1], &sa, 0, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED))
    {
        spdlog::error("Error creating pipe for stderr");
        throw std::logic_error("Pipe error");
    }

    if (!MyCreatePipeEx(&input_handles[0], &input_handles[1], &sa, 0, FILE_FLAG_OVERLAPPED, FILE_FLAG_OVERLAPPED))
    {
        spdlog::error("Error creating pipe for stdin");
        throw std::logic_error("Pipe error");
    }
    // if (!CreatePipe(&output_handles_a[0], &output_handles_b[0], &sa, 0))
    //{
    //    spdlog::error("Error creating pipe for stdout");
    //    throw std::logic_error("Pipe error");
    //}

    // if (!CreatePipe(&output_handles_a[1], &output_handles_b[1], &sa, 0))
    //{
    //    spdlog::error("Error creating pipe for stderr");
    //    throw std::logic_error("Pipe error");
    //}

    // if (!CreatePipe(&input_handles[0], &input_handles[1], &sa, 0))
    //{
    //    spdlog::error("Error creating pipe for stdin");
    //    throw std::logic_error("Pipe error");
    //}

    if (!SetHandleInformation(output_handles_a[0], HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Error setting flag on stdout: {}", geterror_to_string());
        throw std::logic_error("Stdout error");
    }

    if (!SetHandleInformation(output_handles_a[1], HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Error setting flag on sterr");
        throw std::logic_error("Stderr error");
    }

    if (!SetHandleInformation(input_handles[1], HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Error setting flag on stdin");
        throw std::logic_error("Stdin error");
    }

    output_ovlp[0]        = {0};
    output_ovlp[1]        = {0};
    output_ovlp[2]        = {0};
    io_events[0]          = CreateEvent(NULL, FALSE, FALSE, NULL);
    io_events[1]          = CreateEvent(NULL, FALSE, FALSE, NULL);
    io_events[2]          = CreateEvent(NULL, FALSE, FALSE, NULL);
    output_ovlp[0].hEvent = io_events[0];
    output_ovlp[1].hEvent = io_events[1];
    output_ovlp[2].hEvent = io_events[2];

    ZeroMemory(&si, sizeof(si));
    si.cb         = sizeof(si);
    si.hStdOutput = output_handles_b[0];
    si.hStdError  = output_handles_b[1];
    si.hStdInput  = input_handles[1];
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    std::vector<char> cmd(command_line.begin(), command_line.end());
    cmd.push_back(0);

    spdlog::debug("Command line:\n'{}'", command_line);

    // Start the child process.
    if (!CreateProcessA(
            NULL,             // No module name (use command line)
            &cmd[0],          // Command line
            NULL,             // Process handle not inheritable
            NULL,             // Thread handle not inheritable
            TRUE,             // Set handle inheritance
            CREATE_NO_WINDOW, // No creation flags
            NULL,             // Use parent's environment block
            NULL,             // Use parent's starting directory
            &si,              // Pointer to STARTUPINFO structure
            &pi)              // Pointer to PROCESS_INFORMATION structure
    )
    {
        spdlog::debug("CreateProcess failed ({}).\n", GetLastError());
        return;
    }

    bool stop    = false;
    io_events[3] = pi.hProcess;
    std::vector<char> stdout_buff(buffsize);
    std::vector<char> stderr_buff(buffsize);
    DWORD             stdout_bytes;
    DWORD             stderr_bytes;

    bool stdout_pending = false;
    bool stderr_pending = false;

    bool stdout_process = true;
    bool stderr_process = true;

    int cycles = 0;

    do
    {
        // STDOUT processing
        if (stdout_process && !stdout_pending)
        {
            stdout_pending = true;
            stdout_process = false;

            bool so_read =
                ReadFile(output_handles_a[0], stdout_buff.data(), stdout_buff.size(), &stdout_bytes, &output_ovlp[0]);
            auto read_res = GetLastError();
            if (!so_read)
            {
                switch (read_res)
                {
                case ERROR_IO_PENDING:
                    break;

                default:
                    spdlog::error("STDOUT ReadFile() Error: {}", geterror_to_string(true, read_res));
                    throw std::logic_error("ReadFile() error");
                    break;
                }
            }
        }

        if (stdout_pending)
        {
            stdout_pending = false;

            auto gor_res = GetOverlappedResult(output_handles_a[0], &output_ovlp[0], &stdout_bytes, false);
            auto gor_err = GetLastError();
            if (!gor_res)
            {
                switch (gor_err)
                {
                case ERROR_HANDLE_EOF:
                    spdlog::debug("EOF when reading");
                    break;

                case ERROR_IO_INCOMPLETE:
                    stdout_pending = true;
                    break;

                default:
                {
                    spdlog::debug("GetOverlappedResult() error");
                }
                }
            }
            else
            {
                // Successful completion
                std::string o(stdout_buff.begin(), stdout_buff.end());
                if (stdout_bytes < buffsize)
                {
                    o.resize(stdout_bytes);
                }

                spdlog::debug("stdout read ended : '{}'", o);
            }
        }

        // STDERR Processing
        if (stderr_process && !stderr_pending)
        {
            stderr_pending = true;
            stderr_process = false;

            bool se_read =
                ReadFile(output_handles_a[1], stderr_buff.data(), stderr_buff.size(), &stderr_bytes, &output_ovlp[1]);
            auto read_res = GetLastError();
            if (!se_read)
            {
                switch (read_res)
                {
                case ERROR_IO_PENDING:
                    break;

                default:
                    spdlog::error("STDERR ReadFile() Error: {}", geterror_to_string(true, read_res));
                    throw std::logic_error("ReadFile() error");
                    break;
                }
            }
        }

        if (stderr_pending)
        {
            stderr_pending = false;

            auto gor_res = GetOverlappedResult(output_handles_a[1], &output_ovlp[1], &stderr_bytes, false);
            auto gor_err = GetLastError();
            if (!gor_res)
            {
                switch (gor_err)
                {
                case ERROR_HANDLE_EOF:
                    spdlog::debug("EOF when reading");
                    break;

                case ERROR_IO_INCOMPLETE:
                    stderr_pending = true;
                    break;

                default:
                {
                    spdlog::debug("GetOverlappedResult() error");
                }
                }
            }
            else
            {
                // Successful completion
                std::string o(stderr_buff.begin(), stderr_buff.end());
                if (stderr_bytes < buffsize)
                {
                    o.resize(stderr_bytes);
                }

                spdlog::debug("stderr read ended : '{}'", o);
            }
        }

        // TODO: KP. This should be manual reset I think.  Currently we have a 5s pause after each buffer fill.

        auto waitRes = WaitForMultipleObjects(4, io_events, FALSE, 5000);
        switch (waitRes)
        {
        case WAIT_OBJECT_0 + 0:
            // Stdout
            {
                stdout_process = true;
            }
            break;

        case WAIT_OBJECT_0 + 1:
            // Stderr
            {
                stderr_process = true;
            }
            break;

        case WAIT_OBJECT_0 + 2:
            // Stdin
            break;

        case WAIT_OBJECT_0 + 3:
            // Process ended
            stop = true;
            break;

        case WAIT_TIMEOUT:
            // Debugging, exit after waiting for a bit
            // if(++cycles>6)
            //   stop=true;
            break;

        case WAIT_FAILED:
        {
            std::stringstream err;
            err << "Wait for process failed:\n" << geterror_to_string();
            spdlog::error(err.str());
            stop = true;
        }
        break;

        default:
            break;
        }
    } while (!stop);
}

void process_win::stop() const
{
    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    CloseHandle(output_handles_b[0]);
    CloseHandle(output_handles_b[1]);
    CloseHandle(input_handles[1]);
}

process_win::~process_win()
{
    spdlog::debug("Destructor stopping process {}", pi.dwProcessId);

    TerminateProcess(pi.hProcess, 0);

    auto res = WaitForSingleObject(pi.hProcess, 5000);
    switch (res)
    {
    case WAIT_TIMEOUT:
        spdlog::warn("Process did not terminate in 5s");
        break;

    case WAIT_OBJECT_0:
        spdlog::warn("Process terminated");
        break;

    case WAIT_FAILED:
        spdlog::warn("Process terminate wait failed");
        break;

    default:
        spdlog::warn("Process terminate wait (unknown)");
        break;
    }

    stop();
}

using namespace easyprospect::service::control_worker;

extern volatile int ep_full_exit;

BOOL WINAPI easyprospect::service::control_worker::ep_win_console_handler(DWORD signal)
{

    if (signal == CTRL_C_EVENT)
    {
        ep_full_exit = 1;
        spdlog::debug("Ctrl-C handled\n"); // do cleanup
    }

    return TRUE;
}

auto geterror_to_string()
{
    char buf[256];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf,
        (sizeof(buf) / sizeof(char)),
        NULL);

    std::string res = buf;

    return res;
}

static volatile long pipe_serial_number;

BOOL APIENTRY easyprospect::service::control_worker::MyCreatePipeEx(
    OUT LPHANDLE             lpReadPipe,
    OUT LPHANDLE             lpWritePipe,
    IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN DWORD                 nSize,
    DWORD                    dwReadMode,
    DWORD                    dwWriteMode)

/*++
Routine Description:
    The CreatePipeEx API is used to create an anonymous pipe I/O device.
    Unlike CreatePipe FILE_FLAG_OVERLAPPED may be specified for one or
    both handles.
    Two handles to the device are created.  One handle is opened for
    reading and the other is opened for writing.  These handles may be
    used in subsequent calls to ReadFile and WriteFile to transmit data
    through the pipe.
Arguments:
    lpReadPipe - Returns a handle to the read side of the pipe.  Data
        may be read from the pipe by specifying this handle value in a
        subsequent call to ReadFile.
    lpWritePipe - Returns a handle to the write side of the pipe.  Data
        may be written to the pipe by specifying this handle value in a
        subsequent call to WriteFile.
    lpPipeAttributes - An optional parameter that may be used to specify
        the attributes of the new pipe.  If the parameter is not
        specified, then the pipe is created without a security
        descriptor, and the resulting handles are not inherited on
        process creation.  Otherwise, the optional security attributes
        are used on the pipe, and the inherit handles flag effects both
        pipe handles.
    nSize - Supplies the requested buffer size for the pipe.  This is
        only a suggestion and is used by the operating system to
        calculate an appropriate buffering mechanism.  A value of zero
        indicates that the system is to choose the default buffering
        scheme.
Return Value:
    TRUE - The operation was successful.
    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.
--*/

{
    HANDLE            ReadPipeHandle, WritePipeHandle;
    DWORD             dwError;
    std::stringstream PipeNameBuffer;

    //
    // Only one valid OpenMode flag - FILE_FLAG_OVERLAPPED
    //

    if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    //
    //  Set the default timeout to 120 seconds
    //

    if (nSize == 0)
    {
        nSize = 4096;
    }

    PipeNameBuffer << "\\\\.\\Pipe\\EpWorkerCntrl."
                   << fmt::format("{:#08x}.{:#08x}", GetCurrentProcessId(), InterlockedIncrement(&pipe_serial_number));

    ReadPipeHandle = CreateNamedPipeA(
        PipeNameBuffer.str().c_str(),
        PIPE_ACCESS_INBOUND | dwReadMode,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,          // Number of pipes
        nSize,      // Out buffer size
        nSize,      // In buffer size
        120 * 1000, // Timeout in ms
        lpPipeAttributes);

    if (!ReadPipeHandle)
    {
        return FALSE;
    }

    WritePipeHandle = CreateFileA(
        PipeNameBuffer.str().c_str(),
        GENERIC_WRITE,
        0, // No sharing
        lpPipeAttributes,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | dwWriteMode,
        NULL // Template file
    );

    if (INVALID_HANDLE_VALUE == WritePipeHandle)
    {
        dwError = GetLastError();
        CloseHandle(ReadPipeHandle);
        SetLastError(dwError);
        return FALSE;
    }

    *lpReadPipe  = ReadPipeHandle;
    *lpWritePipe = WritePipeHandle;
    return (TRUE);
}
