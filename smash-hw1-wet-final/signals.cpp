#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/types.h>

using namespace std;
void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    SmallShell &A=SmallShell::getInstance();
    if (A.pid_running_ForGround > 0) {
        pid_t COMMANDTOKILL = (A.pid_running_ForGround);
      Command* mc=A.CreateCommand(A.cmd_line.c_str());
     A.job_list.addJob(mc, A.pid_running_ForGround, true);
        if (kill(COMMANDTOKILL, SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }
        A.pid_running_ForGround = -1;
        A.cmd_line="";
        A.curr_command= nullptr;
        cout<< "smash: process "<<(int)COMMANDTOKILL<<" was stopped"<<endl;
    }
}
void ctrlCHandler(int sig_num) {
    SmallShell &A=SmallShell::getInstance();
    cout<<"smash: got ctrl-C"<<endl;
    if(A.pid_running_ForGround >0){
        pid_t COMMANDTOKILL=(A.pid_running_ForGround);
        if(kill(COMMANDTOKILL,SIGKILL)==-1){
            perror("smash error: kill failed");
            return;
        }
        A.pid_running_ForGround = -1;
        A.cmd_line="";
        A.curr_command= nullptr;
        cout<<"smash: process "<<(int)COMMANDTOKILL<<" was killed"<< endl;
    }
}
void fareHandler(int sig_num)
{
}
