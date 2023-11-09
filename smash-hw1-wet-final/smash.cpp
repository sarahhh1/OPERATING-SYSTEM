#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
const std::string WHITESPACE = " \n\r\t\f\v";

int main(int argc, char* argv[]) {

    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    if(signal(SIGXFSZ , fareHandler)==SIG_ERR) {
        perror("smash error: failed to set fare handler");
    }
    SmallShell& smash = SmallShell::getInstance();
    while(true) {
        std::cout << smash.smash_display_line;
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        if (std::string(cmd_line).find_first_not_of(WHITESPACE) == std::string::npos) {
            continue;
        }
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}
