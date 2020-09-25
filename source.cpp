#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>


void print_usage() {
    // print usage
}


enum CMD_TYPE {
     TO_LOWER,
     TO_UPPER,
     SUBSTITUTE,
     UNDEFINED
};

// Additional info (e.g. for SUBSTITUTE command)
class CommandInfo {
public:
    virtual void print() {}
};

class SubCommandInfo : public CommandInfo {
public:
    char a, b;
    SubCommandInfo() {}
    SubCommandInfo(char a, char b) {
        this->a = a;
        this->b = b;
    }
    SubCommandInfo(std::string cmd_str) {
        char aa, bb;
        int field_num;
        std::sscanf(cmd_str.c_str(), "%d:R%c%c", &field_num, &aa, &bb);
        a = aa;
        b = bb;
    }
    virtual void print() {
        std::cout << a << " " << b << std::endl;
    }
};

struct Command {
    CMD_TYPE type;
    int field_num;
    CommandInfo *info;
    Command() {
        type = UNDEFINED;
        field_num = 0;
        info = new CommandInfo();
    }
    Command(std::string cmd_str) {
        type = determine_cmd_type(cmd_str);
        std::sscanf(cmd_str.c_str(), "%d:", &field_num);
        if (type == SUBSTITUTE) {
            info = new SubCommandInfo(cmd_str);
        }
        else {
            info = new CommandInfo();
        }
    }
    
    CMD_TYPE determine_cmd_type(std::string cmd) {
        int n = cmd.size();
        for (int i = 0; i < n; ++i) {
            if (cmd[i] == ':' && i != n - 1) {
                switch (cmd[i + 1]) {
                case 'u':
                    return TO_LOWER;
                case 'U':
                    return TO_UPPER;
                case 'R':
                    return SUBSTITUTE;
                default:
                    return UNDEFINED;
                }
            }
        }
        return UNDEFINED;
    }

    void print_num() {
        std::cout << field_num << std::endl;
    }

    void print_type() {
        std::string type_str = "";
        switch (type) {
        case TO_LOWER:
            type_str = "TO_LOWER";
            break;
        case TO_UPPER:
            type_str = "TO_UPPER";
            break;
        case SUBSTITUTE:
            type_str = "SUBSTITUTE";
            break;
        case UNDEFINED:
            type_str = "UNDEFINED";
            break;
        default:
            break;
        }
        std::cout << type_str << std::endl;
    }
    
    void print_info() {
        print_num();
        print_type();
        info->print();
    }
    ~Command() {
        //delete info;
    }
};

// processes one line with a command
class CommandProcessor {
private:
    Command *cmd;
    std::string line;
public:
    CommandProcessor() {
        line = "";
        cmd = new Command();
    }

    void set_cmd(Command *cmd) {
        this->cmd = cmd;
    }

    CMD_TYPE get_type() {
        return cmd->type;
    }

    void set_line(std::string &line) {
        this->line = line;
    }
    
    std::string get_line() {
        return this->line;
    }

    void process() {
        std::istringstream istr;
        istr.str(line);
        int cnt = 0;
        int i;
        for (i = 0; i < line.size() && cnt < cmd->field_num; ++i) {
            if (line[i] == '\t') {
                ++cnt;
            }
        }

        for (; i < line.size() && line[i] != '\t'; ++i) {
            if (cmd->type == TO_UPPER) {
                line[i] = toupper(line[i]);
            }
            else if (cmd->type == TO_LOWER) {
                line[i] = tolower(line[i]);
            }
            else if (cmd->type == SUBSTITUTE && line[i] == ((SubCommandInfo*)(cmd->info))->a) {
                line[i] = ((SubCommandInfo*)(cmd->info))->b;
            }
            else {
                // unreachable
            }
        }
    }

    
    ~CommandProcessor() {
        //delete cmd;
    }
};

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_usage();
    }
    else {
        std::string filename = argv[1];
        std::ifstream file(filename);
        
        if (file.is_open()) {
            
            std::vector<Command> queue;
            for (int i = 2; i < argc; ++i) {
                queue.push_back(Command(argv[i]));
                if (queue.back().type == UNDEFINED) {
                    std::cerr << "Undefined command [" << i - 1 << "]\n";
                    file.close();
                    return -1;
                }
            }

            CommandProcessor *proc = new CommandProcessor();
            std::string line = "";
            while (std::getline(file, line)) {
                proc->set_line(line);
                for (auto& p : queue) {
                    proc->set_cmd(&p);
                    proc->process();
                }
                std::cout << proc->get_line() << std::endl;
            }
            delete proc;
        }
        else {
            std::cerr << "Could not open file.\n";
            return -1;
        }

        file.close();
    }
    return 0;
}
