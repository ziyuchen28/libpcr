#pragma once

#include "prism/proc/child_proc.h"
#include "prism/proc/proc_spec.h"

namespace prism::proc {

// Convenience wrapper for the common parent<->child stdio-over-pipes case.
// Parent:
//   writes to stdin_fd()
//   reads from stdout_fd()
//   reads from stderr_fd()
class PipedChild 
{

public:
    PipedChild() = default;
    ~PipedChild();

    // PipedChild is move only
    PipedChild(const PipedChild&) = delete;
    PipedChild& operator=(const PipedChild&) = delete;


    PipedChild(PipedChild &&other) noexcept;
    PipedChild &operator=(PipedChild &&other) noexcept;

    static PipedChild spawn(const ProcessSpec &spec);

    ChildProcess &process() noexcept { 
        return process_; 
    }

    const ChildProcess &process() const noexcept { 
        return process_; 
    }

    int stdin_write_fd() const noexcept { 
        return parent_write_stdin_; 
    }

    int stdout_read_fd() const noexcept { 
        return parent_read_stdout_; 
    }

    int stderr_read_fd() const noexcept { 
        return parent_read_stderr_; 
    }

    void close_stdin_write();
    WaitResult wait();


private:
    static PipedChild from_raw(ChildProcess proc, 
                              int parent_write_stdin, 
                              int parent_read_stdout, 
                              int parent_read_stderr) noexcept;

    void close_fds() noexcept;

    ChildProcess process_;
    int parent_write_stdin_ = -1;
    int parent_read_stdout_ = -1;
    int parent_read_stderr_ = -1;
};

} // namespace prism::proc
