 
#include "pcr/proc/piped_child.h"
#include "test_helpers.h"

#include <cassert>
#include <iostream>
#include <string>

int main() 
{
    using namespace pcr::proc;

    ProcessSpec spec;
    spec.exe = "echo";
    spec.args = {"pcr_stdout_success"};


    auto child = PipedChild::spawn(spec);

    
    child.close_stdin_write();

    const std::string out = read_all_fd(child.stdout_read_fd());
    const std::string err = read_all_fd(child.stderr_read_fd());
    const WaitResult wr = child.wait();

    // handle carriage return for native win tool
    const std::string expected_out = "pcr_stdout_success\r\n";

    if (out != expected_out) {
        std::cout << "mismatch!\n";
        std::cout << "out size: " << out.size() << " expected: " << expected_out.size() << "\n";
        for (char c : out) std::printf("%02x ", (unsigned char)c);
        std::cout << "\n";
        return 1; 
    }
    assert(out == expected_out);
    assert(err.empty());
    assert(wr.exited);
    assert(wr.exit_code == 0);

    std::cout << "test_proc_piped_win: ok\n";
    return 0;
}
