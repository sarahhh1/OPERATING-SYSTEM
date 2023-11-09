#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_


#include <time.h>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <memory>
#include "signal.h"
#include <unistd.h>
#include <linux/limits.h>
#include <fstream>
#include <iostream>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
class Command {
public:
    std::string cmd_line;
    std::vector<std::string> arguments;
    int num_of_arguments;
    Command(const char* _cmd_line);
    virtual ~Command()=default;
    virtual void execute() = 0;
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
    bool is_complex;
    bool is_backGround;
public:
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
    void is_it_complex(){
        is_complex= (cmd_line.find("*")!=std::string::npos||cmd_line.find("?")!=std::string::npos);
    }
    void is_it_backGround(){
        char * last_character;
        (* last_character)= (this->arguments[arguments.size() - 1].back());
        is_backGround= ((last_character)=="&")?true:false;

    }
};

class PipeCommand : public Command {
    std::string  command1;
    std::string  command2;
    bool is_stderr_to_write;
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {}
    void prepare() ;
    void execute() override;
    void is_it_stderr_to_write(){
        is_stderr_to_write= (cmd_line.find("|&")!=std::string::npos);
    }
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
    bool is_append;
    std::string command;
    std::string file_name;
public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    void prepare();
     void is_it_append(){
       is_append= (cmd_line.find(">>")!=std::string::npos);
    }
};

class ChpromptCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ChpromptCommand(const char* cmd_line);
    virtual ~ChpromptCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
public:
// TODO: Add your data members public:
    ChangeDirCommand(const char* cmd_line);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
    JobsList* jobs;
// TODO: Add your data members
public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() {}
    void execute() override;
};

class JobsList {
public:
    class JobEntry {
    public:
        // TODO: Add your data members
        std::string cmd_line;
        int JobID;
        pid_t Job_PID;
        time_t insertation_Time;//start time
        bool is_stopped;

        JobEntry( std::string cmd_line,int JobID, pid_t Job_PID,time_t insertation_Time, bool is_stopped);//constructor

        ~JobEntry()=default;
    };
public:
    int Max_jobID;
    std::vector<JobEntry> jobs_List;
    JobsList();
    ~JobsList()=default;

    void addJob(Command *cmd,pid_t pid,bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry * getJobById(int jobId);
    JobEntry * getJobByPid(int jobPid);
    void removeJobById(int jobId);

    JobEntry * getLastJob(int *lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    JobsCommand(const char* cmd_line, JobsList* jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members

public:
    JobsList* jobs;
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {

    // TODO: Add your data members
public:
    JobsList* jobs;
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    /* Bonus */
    // TODO: Add your data members
    JobsList* jobs;
public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() {}
    void execute() override;
};

class FareCommand : public BuiltInCommand {
public:
    FareCommand(const char* cmd_line);
    virtual ~FareCommand() {}
    void execute() override;
    void FareWrite(std::string &my_file, std::string &line_to_write,int size_cpy,char *buff_line_copy,int counter,std::string &source);
    int FareRead(std::string &my_file, std::string &line_to_read);
};

class SetcoreCommand : public BuiltInCommand {
    JobsList* jobs;
public:
    SetcoreCommand(const char* cmd_line,JobsList* jobs);
    virtual ~SetcoreCommand() {}
    void execute() override;
};

class SmallShell {
    SmallShell() : cmd_line(""), shell_pid(getpid()), job_list(), smash_display_line("smash> "), pid_running_ForGround(-1){

        last_Pwd="";
    }
public:
    std::string cmd_line;
    pid_t shell_pid;
    JobsList job_list;
    std::string smash_display_line;
    pid_t pid_running_ForGround;
    Command*curr_command;
    std::string last_Pwd;

    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
