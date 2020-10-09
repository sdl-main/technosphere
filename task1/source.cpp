#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <signal.h>
#include <sys/wait.h>
#include <iostream>

auto f_write = write;
auto f_read = read;
auto f_close = close;

class Process
{
private:
    pid_t pid;
    int pipefd[2];
public:
    explicit Process(const std::string& path) {
        pid = fork();
        if (pipe(pipefd) == -1) {
            exit(-1);
        }
        
        if (pid == 0) {
            // child
            dup2(pipefd[0], 0); // stdin  -> read side of pipe
            dup2(pipefd[1], 1); // stdout -> write side of pipe
            execl(path.c_str(), path.c_str(), nullptr);
            // unreachable
        }
        else {
            // parent
            
        }
    }
    
    ~Process() {
        f_close(pipefd[0]);
        f_close(pipefd[1]);
    }

    size_t write(const void* data, size_t len) {
        int sz = f_write(pipefd[1], data, len);
        if (sz == -1) {
            exit(-1);
        }
        return sz;
    }
    
    void writeExact(const void* data, size_t len) {
        size_t sz {0};
        const char *ptr = reinterpret_cast<const char*>(data);
        while (sz < len) {
            sz += write(ptr + sz, 1);
        }
    }
    
    size_t read(void* data, size_t len) {
        int sz = f_read(pipefd[0], data, len);
        if (sz == -1) {
            exit(-1);
        }
        return sz;
    }
    
    void readExact(void* data, size_t len) {
        size_t sz {0};
        char *ptr = reinterpret_cast<char*>(data);
        while (sz < len) {
            sz += read(ptr + sz, 1);
        }
    }

    void closeStdin() {
        f_close(pipefd[0]);
    }

    void close() {
        kill(pid, SIGINT);
        waitpid(pid, nullptr, 0);
    }
};

int main(void) {
    Process pr("/bin/echo");
    char c = 'a';
    pr.writeExact(&c, 1);
    pr.readExact(&c, 1);
    std::cout << c << std::endl;
    return 0;
}
