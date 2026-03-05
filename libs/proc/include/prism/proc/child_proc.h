#pragma once

#include "prism/proc/child_stdio.h"
#include "prism/proc/proc_spec.h"

#include <sys/types.h>

namespace prism::proc {

struct WaitResult 
{
    bool exited = false;
    int exit_code = -1;

    bool signaled = false;
    int term_signal = 0;
};

class ChildProcess 
{
public:
    ChildProcess() = default;
    ~ChildProcess();

    ChildProcess(const ChildProcess&) = delete;
    ChildProcess &operator=(const ChildProcess&) = delete;

    ChildProcess(ChildProcess &&other) noexcept;
    ChildProcess &operator=(ChildProcess &&other) noexcept;
    
    // to do: future support - spawn detached process 
    // hence prefer named constructor over constructor for overload extensibility 
    static ChildProcess spawn(const ProcessSpec &spec, const ChildStdioMap &stdio);

    pid_t pid() const noexcept { 
        return pid_; 
    }

    void terminate(int signal_number = 15); // SIGTERM
    // to do void term_graceful(std::chrono::milliseconds timeout); // SIGTERM -> wait -> SIGKILL
    WaitResult wait();

private:
    static ChildProcess from_pid(pid_t pid) noexcept;
    void reap_if_dead() noexcept;

    pid_t pid_ = -1;
};

} // namespace prism::proc
