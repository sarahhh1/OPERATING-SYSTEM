#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <time.h>
#include <utime.h>
#include "Commands.h"
#include <stdio.h>
#include <algorithm>
#include <linux/limits.h>
#include <sys/wait.h>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

////FIND FIRST CHARACTER
string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
////FIND LAST CHARACTER
string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
////GET CLEAN CMD
string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    //GET CLEAN CMD STRING IN ISS
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;
    FUNC_EXIT()
}

bool is_a_minus(std::string my){
    if(my[0]=='-')
    {
        int i = 1;
        while(my[i]){
            if(isdigit(my[i])==0){
                return false;
            }
            i++;
        }
      return true;
    }
    return false;
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

///helper func
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
//checked
Command * SmallShell::CreateCommand(const char* cmd_line) {
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if((cmd_s.find("|&")!=std::string::npos)||(cmd_s.find("|")!=std::string::npos)){
        return new PipeCommand(cmd_line);
    } else if (cmd_s.find(">>")!=std::string::npos||(cmd_s.find(">")!=std::string::npos)) {
		return new RedirectionCommand(cmd_line);
    }
    else if (firstWord.compare("pwd") == 0 ||firstWord.compare("pwd&") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0 ||firstWord.compare("showpid&") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("chprompt") == 0) {
        return new ChpromptCommand(cmd_line);
    }
    else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    }
    else if (firstWord.compare("jobs") == 0|| firstWord.compare("jobs&") == 0) {
        return new JobsCommand(cmd_line,&job_list);
    }
    else if (firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0) {
        return new ForegroundCommand(cmd_line,&job_list);
    }
    else if (firstWord.compare("bg") == 0||firstWord.compare("bg&") == 0) {
        return new BackgroundCommand(cmd_line,&job_list);
    }
    else if (firstWord.compare("quit") == 0|| firstWord.compare("quit&") == 0) {
        return new QuitCommand(cmd_line,&job_list);
    }
    else if (firstWord.compare("kill") == 0||firstWord.compare("kill&") == 0) {
        return new KillCommand(cmd_line,&job_list);

    }else if(firstWord.compare("setcore") == 0) {
        return new SetcoreCommand(cmd_line,&job_list);
    }
    else if(firstWord.compare("fare") == 0) {
        return new FareCommand(cmd_line);
	}else {
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}
/////////////////////small-shell function///////////////////////////////////////////////////////////
void SmallShell::executeCommand(const char *cmd_line_) {
    ///check finished jobs
    this->job_list.removeFinishedJobs();
    Command* cmd = CreateCommand(cmd_line_);
    if(cmd == nullptr){
        return;
    }
    string cmd_s = _trim(string(cmd_line_));
    this->cmd_line=cmd_s;
    this->curr_command=cmd;
    this->pid_running_ForGround=-1;
    cmd->execute();
    this->cmd_line="";
    this->curr_command=nullptr;
    this->pid_running_ForGround=-1;
    // Please note that you must fork smash process for some commands (e.g., external commands....)
    delete cmd;
}

SmallShell::~SmallShell() {

// TODO: add your implementation
}

////////general//////////////////////////////////////////////////////////////////////////////////
Command::Command(const char *_cmd_line) :cmd_line(_cmd_line){
    char * args_c[COMMAND_MAX_ARGS+3];
    char tmp_2[cmd_line.size()];
    strcpy(tmp_2,cmd_line.c_str());
    _removeBackgroundSign(tmp_2);
    int length =_parseCommandLine(tmp_2,args_c);
    string tmp[length];
    int i=0;
    for (i=0; i<length; i++)
    {
        tmp[i]= _trim(string(args_c[i]));
        free(args_c[i]);
        arguments.push_back(tmp[i]);
    }
    this->num_of_arguments=i-1;
    return;
}

//////// build-in/////////////////////////////////////////////////////////////////////////////////
BuiltInCommand::BuiltInCommand(const char *cmd_line): Command(cmd_line) {}
////////chprompt command////////
ChpromptCommand::ChpromptCommand(const char* cmd_line):BuiltInCommand( cmd_line ) {}

void ChpromptCommand::execute() {
    SmallShell &my_shell = SmallShell::getInstance();
    if (this->num_of_arguments==0){
        my_shell.smash_display_line="smash> ";
        return;
    }
    my_shell.smash_display_line= arguments[1] + "> ";;
    return;
}
////////pwd command/////////////
GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line): BuiltInCommand( cmd_line ) {}

void GetCurrDirCommand::execute() {
    char buffer[PATH_MAX] = {0};
    char *p = getcwd(buffer, size_t(PATH_MAX));
    if (p == NULL) {
        perror("smash error: getcwd failed\n");
        return;
    }
    cout << buffer;
    cout<<endl;
    return;
}
////////cd command/////////////
ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand( cmd_line ){}

void ChangeDirCommand::execute() {
    SmallShell &my_singelton=SmallShell::getInstance();
    bool first=false;
    if(my_singelton.last_Pwd==""){
        first= true;
    }
    if(num_of_arguments>2)
    {
        cerr<<"smash error: cd: too many arguments"<<endl;
        return;
    }
    ///no arguments
    if(num_of_arguments==0){
        return;
    }
    if(num_of_arguments==2 && !(arguments[2].compare("&") ==0))
    {
        cerr<<"smash error: cd: too many arguments"<<endl;
        return;
    }
    if( arguments[1].compare("-") == 0 ){
        std::string last_one=my_singelton.last_Pwd;
        ///check if last pwd exist
        if (my_singelton.last_Pwd.empty()){
            cerr<<"smash error: cd: OLDPWD not set"<<endl;
            return;
        }
        ///change to last_PWD and update lastpwd
        char buffer[PATH_MAX]={0};
        char * p=getcwd(buffer,size_t(PATH_MAX));
        if (p==NULL)
        {
            perror("smash error: getcwd failed");
            return;
        }
        const char *cstr = my_singelton.last_Pwd.c_str();
        int retVAL= chdir(cstr);
        if(retVAL==-1)
        {
            perror("smash error: chdir failed");
            my_singelton.last_Pwd=last_one;
            return;
        }
        my_singelton.last_Pwd= buffer;
        return;
    }
        char buff[PATH_MAX]={0};
        char * p1=getcwd(buff,size_t(PATH_MAX));
        if (p1==NULL)
        {
            perror("smash error: getcwd failed");
            return;
        }
    const char *new_path = arguments[1].c_str();
    int retVAL= chdir(new_path);
    if(retVAL==-1)
    {
        perror("smash error: chdir failed");
        return;
    }
    my_singelton.last_Pwd=buff;
    return;
}
///////showpid command/////////
ShowPidCommand::ShowPidCommand(const char *cmd_line) :BuiltInCommand(cmd_line){}

void ShowPidCommand::execute() {
    SmallShell &my_singelton=SmallShell::getInstance();
    cout << "smash pid is " << my_singelton.shell_pid<< endl;
    return;
}
/////////forground command////////////
ForegroundCommand::ForegroundCommand(const char *cmd_line,JobsList *jobs) :BuiltInCommand(cmd_line),jobs(jobs){}

void ForegroundCommand::execute() {
    SmallShell &my_singelton = SmallShell::getInstance();
    if (num_of_arguments > 1) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    if (num_of_arguments == 1 && is_a_minus(arguments[1])) {
        cerr << "smash error: fg: job-id " << arguments[1] << " does not exist" << endl;
        return;
    }
    if (num_of_arguments == 1 && !is_number(arguments[1])) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    if (num_of_arguments == 1) {
        JobsList::JobEntry *my_job = jobs->getJobById(stoi(arguments[1]));
        int job_id = stoi(arguments[1]);
        if (!my_job) {
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            return;
        }
        pid_t my_pid = my_job->Job_PID;
        std::string job_name = my_job->cmd_line;
        cout << job_name << " : " << int(my_pid) << endl;
        if (my_job->is_stopped) {
            if (kill(my_pid, SIGCONT) == -1) {
                perror("smash error: kill failed");
                return;
            }
        }
        my_singelton.cmd_line = my_job->cmd_line;
        my_singelton.pid_running_ForGround = my_pid;
        int my_status;
        if (waitpid(my_pid, &my_status, WUNTRACED) == -1) {
            perror("smash error: waitpid failed");
            return;
        }
        if (!WIFSTOPPED(my_status)) {
            jobs->removeJobById(my_job->JobID);
        }
        return;
    }
    if (num_of_arguments == 0) {
        if (jobs->jobs_List.empty()) {
            cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        int THIS_JOB_ID = 0;
        JobsList::JobEntry *my_job = jobs->getLastJob(&THIS_JOB_ID);
        pid_t my_pid = my_job->Job_PID;
        cout << my_job->cmd_line << " : " << int(my_pid) << endl;
        if (my_job->is_stopped) {
            if (kill(my_pid, SIGCONT) == -1) {
                perror("smash error: kill failed");
                return;
            }
        }
        my_singelton.cmd_line = my_job->cmd_line;
        my_singelton.pid_running_ForGround = my_pid;
        int my_status;
        if (waitpid(my_pid, &my_status, WUNTRACED) == -1) {
            perror("smash error: waitpid failed");
            delete my_singelton.curr_command;
            my_singelton.curr_command = nullptr;
            return;
        }
        if (!WIFSTOPPED(my_status)) {
            jobs->removeJobById(my_job->JobID);
            return;
        }
        return;
    }
}
/////////background command///////////
BackgroundCommand::BackgroundCommand(const char *cmd_line,JobsList *jobs) :BuiltInCommand(cmd_line),jobs(jobs){}

void BackgroundCommand::execute() {
    if(num_of_arguments>1){
        cerr<<"smash error: bg: invalid arguments"<<endl;
        return;
    }
     if(num_of_arguments== 1 && is_a_minus(arguments[1])){
        cerr << "smash error: bg: job-id " << arguments[1] << " does not exist" << endl;
        return;
    }
    if(num_of_arguments==1&& !is_number(arguments[1])){
        cerr<<"smash error: bg: invalid arguments"<<endl;
        return;
    }
    if(num_of_arguments==1){
        JobsList::JobEntry* my_job= jobs->getJobById(stoi(arguments[1]));
        if(my_job==NULL)
        {
            cerr<<"smash error: bg: job-id "<<(stoi(arguments[1]))<<" does not exist"<<endl;
            return;
        }
        pid_t my_pid= my_job->Job_PID;
        if(!my_job->is_stopped)
            {
                cerr<<"smash error: bg: job-id "<<my_job->JobID<<" is already running in the background"<<endl;
                return;
            }
            cout<<my_job->cmd_line<< " : " <<int(my_pid)<< endl;
            if(kill(my_pid,SIGCONT)==-1) {
                perror("smash error: kill failed");
                return;
            }
             my_job->is_stopped=false;
			return;
        }
        
    if(num_of_arguments==0){
        if(jobs->jobs_List.empty())
        {
            cerr<<"smash error: bg: there is no stopped jobs to resume"<<endl;
            return;
        }
        int lastStopedID=0;
        JobsList::JobEntry* my_job= jobs->getLastStoppedJob(&lastStopedID);
        if(my_job==NULL){
            cerr<<"smash error: bg: there is no stopped jobs to resume"<<endl;
            return;
            }
        pid_t my_pid= my_job->Job_PID;
        cout<<my_job->cmd_line<<" : "<<int(my_pid)<< endl;
        if(kill(my_pid,SIGCONT)==-1) {
        perror("smash error: kill failed");
        return;
        }
        my_job->is_stopped=false;
        return;
    }
    return;
}
/////////quit command/////////////////
QuitCommand::QuitCommand(const char *cmd_line,JobsList* jobs): BuiltInCommand(cmd_line),jobs(jobs){}

void QuitCommand::execute() {
    SmallShell &my_singelton=SmallShell::getInstance();
    if(num_of_arguments==0) {
        if (kill(my_singelton.shell_pid, SIGKILL) == -1) {
            perror("smash error: kill failed");
            return;
        }
        return;
    }
    if(num_of_arguments==1&&arguments[1].compare("kill")==0)
    {    
		jobs->removeFinishedJobs();
        string dead_cmd;
        int num_of_jobs = jobs->jobs_List.size();
        cout <<"smash: sending SIGKILL signal to "<<num_of_jobs<<" jobs:"<<endl;
        for(int i=0;i<num_of_jobs;i++)
        {
            std::string dead_cmd= jobs->jobs_List[i].cmd_line;
            pid_t dead_PID=(jobs->jobs_List[i].Job_PID);
            if( kill(dead_PID,SIGKILL)==-1){
                perror("smash error: kill failed");
                return;
            }
            cout<<dead_PID<<": "<<dead_cmd;
            cout<<endl;
        }
    }
    my_singelton.pid_running_ForGround=-1;
    exit(0);
}

///////////////bonus/////////////////////////////////////////////////////////////////////////////////
KillCommand::KillCommand(const char *cmd_line, JobsList* jobs): BuiltInCommand(cmd_line),jobs(jobs){}

void KillCommand::execute()
{
    SmallShell& shell=SmallShell::getInstance();
    if (arguments.size()<3 || arguments.size()>3)
    {
        cerr<<"smash error: kill: invalid arguments"<<endl;
        return;
    }
    int job_ID;
    try
    {
        job_ID = stoi(arguments[2]);//convert string to int to get the job_id
    }
    catch(exception &)
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    JobsList::JobEntry * job_entry = shell.job_list.getJobById(job_ID);
    if ((arguments[1].c_str())[0] != '-')
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    int signum_id;
    try
    {
        signum_id= stoi(arguments[1].substr(1,arguments[1].length()-1));
    }
    catch(exception &)
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    if (signum_id<0 || signum_id>31)//range of signals
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    if (!job_entry)
    {
        cerr<<"smash error: kill: job-id "<<job_ID<<" does not exist"<<endl;
        return;
    }
    if (kill(job_entry->Job_PID,signum_id) == -1)
    {
        perror("smash error: kill failed");
        return;
    }
    else
    {
        cout << "signal number " << signum_id <<" was sent to pid "<<job_entry->Job_PID << endl;
        if(signum_id==SIGKILL)
        {
            if(waitpid(job_entry->Job_PID,NULL,0)==-1)
            {
                perror("smash error: waitpid failed");
                return;
            }
            shell.job_list.removeJobById(job_ID);
        }
    }
}

//////////////external command///////////////////////////////////////////////////////////////////////
ExternalCommand::ExternalCommand(const char *cmd_line_): Command(cmd_line_) {
    this->is_backGround=_isBackgroundComamnd(cmd_line_);
    is_it_complex();
}

void ExternalCommand::execute() {
    SmallShell &my_singelton = SmallShell::getInstance();
    pid_t my_son_pid = fork();
    if (my_son_pid == -1) {
        perror("smash error: fork failed");
        return;
    }
    if (my_son_pid > 0) {
        if (!is_backGround) {
            my_singelton.pid_running_ForGround = my_son_pid;
        }
    }
    ///son
    if (my_son_pid == 0) {
        if (setpgrp() == -1) {
            perror("smash error: setpgrp failed");
            return;
        }
        if (is_complex) {
            std::string cleanCommand;
            for (unsigned int i = 0; i < arguments.size() - 1; i++) {
                cleanCommand += (arguments[i] + " ");
            }
            std::string last;
            last = arguments[arguments.size() - 1];
            cleanCommand += last;
            char* my_dest;
            strcpy(my_dest,cleanCommand.c_str());
            char *argv[] = {"/bin/bash", "-c", my_dest, NULL};

            if( execv("/bin/bash",argv)==-1)
            {
                perror("smash error: execv failed");
                return;
            }
        }
        else{
            char* argv[this->num_of_arguments+2] ;

            int i;
            for(i=0;i<this->num_of_arguments+1;i+=1)
            {
                argv[i]=const_cast<char*>((arguments[i]).c_str());
            }
            argv[num_of_arguments+1]= nullptr;
            ///copy arguments to argv
            if(execvp(argv[0], argv) == -1){
                perror("smash error: execvp failed");
                return;
            }
        }
        
    }
    else{
        if(is_backGround){
            my_singelton.job_list.addJob(this,my_son_pid,false);
        }else{
            int status;
            int  retValWaitPid=waitpid(my_son_pid,&status,WUNTRACED);
            if(retValWaitPid==-1){
                perror("smash error: waitpid failed");
                 return;
        }
		}
        return;
    }
}
/////////jobs function//////////////////////////////////////////////////////////////////////////
JobsCommand::JobsCommand(const char *cmd_line, JobsList* jobs) :BuiltInCommand(cmd_line),jobs(jobs){}

void JobsCommand::execute() {
    JobsList * job_s =&(SmallShell::getInstance().job_list);
    job_s->removeFinishedJobs();
    job_s->printJobsList();
}

JobsList::JobEntry::JobEntry( std::string cmd_line,int JobID, pid_t Job_PID,time_t insertation_Time, bool is_stopped)
        : cmd_line(cmd_line),
          JobID(JobID),
          Job_PID(Job_PID),
          insertation_Time(insertation_Time),
          is_stopped(is_stopped) {}


JobsList::JobsList() : Max_jobID(0), jobs_List(){}

void JobsList::addJob(Command *cmd,pid_t pid,bool isStopped)
{
  removeFinishedJobs();
  time_t insert_time= time(NULL);
    if (insert_time== -1)
    {
        perror("smash error: time failed");
        return;
    }
    ///because comment in jobs about adding again
    if(getJobByPid(pid)!=nullptr){
        JobsList::JobEntry *my_job= getJobByPid((int)pid);
        my_job->insertation_Time=insert_time;
        if(isStopped){
			my_job->is_stopped=true;
		}else{
			my_job->is_stopped=false;
			}
     return;
    }
    JobEntry new_job(cmd->cmd_line,Max_jobID+1,pid,insert_time,isStopped);
    Max_jobID++;
    jobs_List.push_back(new_job);
}
///job list sorted by job id
void JobsList::printJobsList()
{
    int size_of_jobs=jobs_List.size();
    time_t job_time = time(NULL);//default value
    for(int i=0; i<size_of_jobs;i++)
    {
        string job_commend = jobs_List[i].cmd_line;
        int job_pid = jobs_List[i].Job_PID;
        if (jobs_List[i].is_stopped == true)
        {
            cout << "["<< jobs_List[i].JobID <<"] " << job_commend<<" : "<<(int)job_pid<<" "<<(int)(job_time-this->jobs_List[i].insertation_Time)<<" secs "<<"(stopped)"<<endl;
        } else
        {
            cout << "["<< jobs_List[i].JobID <<"] " << job_commend<<" : "<<(int)job_pid<<" "<<(int)(job_time-this->jobs_List[i].insertation_Time)<<" secs"<< endl;
        }
    }
}

void JobsList::killAllJobs()
{   this->removeFinishedJobs();
    int size=jobs_List.size();
    for(int i=0;i<size;i++)
    {
        if (kill(jobs_List[i].Job_PID,SIGKILL) == -1)
        {
            perror("smash error: kill failed");
            return;
        }
    }
}

void JobsList::removeFinishedJobs() {
    SmallShell &my_smash = SmallShell::getInstance();
    if (this->jobs_List.empty()) {
        return;
    }
    pid_t res = waitpid(-1, nullptr, WNOHANG);
    while (res > 0) {
        for (unsigned int i = 0; i < my_smash.job_list.jobs_List.size(); ++i) {
            if (this->jobs_List[i].Job_PID == res) {
                my_smash.job_list.jobs_List.erase((my_smash.job_list.jobs_List.begin() + i));
            }
        }
        res = waitpid(-1, nullptr, WNOHANG);
    }
    getLastJob(&Max_jobID);
    return;
}

JobsList::JobEntry * JobsList::getJobById(int jobId)
{
    for(auto& cur_job : jobs_List){
        if (cur_job.JobID == jobId) {
            return &cur_job;
        }
    }
    return nullptr;
}

JobsList::JobEntry *JobsList::getJobByPid(int jobPid)
{
    for(auto& cur_job : jobs_List) {
        if (cur_job.Job_PID == jobPid) {
            return &cur_job;
        }
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
    SmallShell &my_shell = SmallShell::getInstance();
    for (auto it = jobs_List.begin(); it != jobs_List.end(); ++it) {
        auto del_job = *it;
        if (del_job.JobID == jobId) {
            jobs_List.erase(it);
            if(jobId==Max_jobID)
            {
                getLastJob(&Max_jobID);
            }
            break;
        }
    }
}

JobsList::JobEntry *JobsList:: getLastJob(int* lastJobId)
    {
   int max_id=0;
    for(unsigned int i=0;i<jobs_List.size();i++)
    {
        if(jobs_List[i].JobID>max_id)
        {
            max_id=jobs_List[i].JobID;
        }
    }
    (* lastJobId)=max_id;
    return getJobById(max_id);
    }

    JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
    {   int last_stopped=-1;
        for(unsigned int i=0;i<jobs_List.size();i++)
        {
            if(jobs_List[i].is_stopped)
            {
                last_stopped=jobs_List[i].JobID;
             }
        }
        (*jobId)=last_stopped;
        return getJobById(last_stopped);
    }

void RedirectionCommand::prepare(){
	is_append=false;
	is_it_append();
        size_t break_here;
        break_here = (string(cmd_line)).find(">");
   if(is_append){
        break_here = (string(cmd_line)).find(">>");
    }
	string to_split = cmd_line;
    command = to_split.substr(0, break_here);
    break_here++;
    if(is_append) {
        break_here += 1;
    }
    string to_split2 = cmd_line;
    string tmp= to_split2.substr(break_here,to_split.length()-break_here);
    tmp= _trim(tmp);
    tmp+= " ";
    file_name = tmp.substr(0, tmp.find_first_of(WHITESPACE));
}
//////////////////////////// io reedirection&pipe////////////////////////////////////////////
RedirectionCommand::RedirectionCommand(const char* cmd_line) : Command((cmd_line))
{
	this->prepare();
}

void cleanupRedi(int my_file,int new_fd){
   if(close(my_file)==-1) {
        perror("smash error: close failed");
    }
    if(dup2(new_fd,1)==-1) {
        perror("smash error: dup2 failed");
    }
    return;
}

void RedirectionCommand::execute()
{   SmallShell &my_shell = SmallShell::getInstance();
    int new_fd= dup(1);
    if(new_fd==-1) {
        perror("smash error: dup failed");
        return;
    }
    ///CHECK KIND OF COMMAND
    if(is_append) {
		int my_file=open(file_name.c_str(), O_CREAT|O_APPEND|O_RDWR,0655);
        if(my_file ==-1) {
            perror("smash error: open failed");
            return;
        }
        if(dup2(my_file,1)==-1) {
            perror("smash error: dup2 failed");
        }

        Command *cmd = my_shell.CreateCommand(command.c_str());
        cmd->execute();
		cleanupRedi(my_file, new_fd);
		}else{         
			int my_file=open(file_name.c_str(), O_CREAT|O_RDWR|O_TRUNC,0655);
			if(my_file ==-1){
            perror("smash error: open failed");
            return;
			}
			if(dup2(my_file,1)==-1) {
            perror("smash error: dup2 failed");
			}
			Command *cmd = my_shell.CreateCommand(command.c_str());
			cmd->execute();
			cleanupRedi(my_file, new_fd);
			}
		return;
}

//////////////////////pipe///////////////////////////
PipeCommand::PipeCommand( const char *cmdline): Command(cmdline) {}

void PipeCommand::prepare(){
	is_stderr_to_write=false;
	is_it_stderr_to_write();
	size_t break_here;
	break_here = (string(cmd_line)).find("|");
   if(is_stderr_to_write){
        break_here = (string(cmd_line)).find("|&");
    }
	string to_split = cmd_line;
    command1 = to_split.substr(0, break_here);
    break_here++;
    if(is_stderr_to_write){
	break_here += 1;
    }
    string to_split2 = cmd_line;
    string tmp= to_split2.substr(break_here,to_split.length()-break_here);
    tmp= _trim(tmp);
    tmp+= " ";
    command2 = tmp.substr(0, tmp.find_last_of(WHITESPACE));
}

void PipeCommand::execute() {
	this->prepare();
    SmallShell &my_shell = SmallShell::getInstance();
    int fd[2];
    if(pipe(fd)==-1) {
        perror("smash error: pipe failed");
        return;
    }
    pid_t son1_pid= fork();
    if(son1_pid==-1){
        perror("smash error: fork failed");
        return;
    }
    if (son1_pid == 0) {
        if( setpgrp()==-1){
            perror("smash error: setpgrp failed");
            return;
        }
        if(is_stderr_to_write){
            if( dup2(fd[1],2)==-1)
            {
                perror("smash error: dup2 failed");
                return;
            }
            if(close(fd[0])==-1){
                perror("smash error: close failed");
                return;
            }
            if(close(fd[1])==-1){
                perror("smash error: close failed");
                return;
            }
        }
        else{
            if( dup2(fd[1],1)==-1)
            {
                perror("smash error: dup2 failed");
                return;
            }
            if(close(fd[0])==-1){
                perror("smash error: close failed");
                return;
            }
            if(close(fd[1])==-1){
                perror("smash error: close failed");
                return;
            }
        }
     Command* my_command=my_shell.CreateCommand(command1.c_str());
       my_command->execute();
        exit(0);
    }
    pid_t son2_pid= fork();
    if(son2_pid==-1){
        perror("smash error: fork failed");
        return;
    }
    
    if (son2_pid == 0) {
        if( setpgrp()==-1){
            perror("smash error: setpgrp failed");
            return;
        }
        // second child
        if(dup2(fd[0],0)==-1){
            {
                perror("smash error: dup2 failed");
                return;
            }
        }
        if( close(fd[0])==-1)
        {
            perror("smash error: close failed");
            return;
        }
        if(close(fd[1])==-1)
        {
            perror("smash error: close failed");
            return;
        }
        Command* my_command=my_shell.CreateCommand(command2.c_str());
        my_command->execute();
        exit(0);
    }
   
    if(close(fd[0])==-1){
        perror("smash error: close failed");
        return;
    }
    if(close(fd[1])){
        perror("smash error: close failed");
        return;
    }


    if(waitpid(son1_pid, NULL,WUNTRACED)==-1){
        perror("smash error: waitpid failed");
        return;
    }
    if(waitpid(son2_pid, NULL,WUNTRACED)==-1){
        perror("smash error: waitpid failed");
        return;
    }
    return;
}

 //////////////////fare Command////////////////
FareCommand::FareCommand(const char* cmd_line): BuiltInCommand(cmd_line){}

void FareCommand::execute()
{
    int counter_for_source=0;
    ///////error handling///////////
    if (num_of_arguments<3 || num_of_arguments>3)
    {
        cerr<<"smash error: fare: invalid arguments"<<endl;
        return;
    }
    string line_to_read;
    string my_file(arguments[1]);
    if(FareRead(my_file,line_to_read)==-1)
    {
        return;
    }
    std::string source=arguments[2];
    std::string destination=arguments[3];
    size_t src_len=source.length();
    size_t des_len=destination.length();
    int size_cpy=line_to_read.size();
    char buff_copy[size_cpy];
    strcpy(buff_copy,line_to_read.c_str());
    auto position=line_to_read.find(source);
    while(position!= string::npos)
    {
        line_to_read.replace(position,src_len,destination);
        counter_for_source++;
        position=line_to_read.find(source,position+des_len);
    }
    FareWrite(my_file,line_to_read,size_cpy,buff_copy,counter_for_source,source);
}
void FareCommand::FareWrite(std::string &my_file, std::string &line_to_write,int size_cpy,char *buff_line_copy,int counter,string &source)
{
    int fd_wr=open(my_file.c_str(),O_WRONLY | O_TRUNC);
    if(fd_wr <0)
    {
        perror("smash error: open failed");
        return ;
    }
    size_t size=line_to_write.size();
    char line_of_buff[size];
    strcpy(line_of_buff,line_to_write.c_str());
    if(write(fd_wr,line_of_buff,size)!=size)
    {
        int ret_fd=open(my_file.c_str(),O_WRONLY | O_TRUNC);
        if(ret_fd < 0)
            {
              perror("smash error: open failed");
              return;
            }
        if(write(ret_fd,buff_line_copy,size_cpy)!=size_cpy)
        {
            perror("smash error: write failed");
        }
        if(close(ret_fd)<0)
        {
            perror("smash error: close failed");
            return;
        }
        if(close(fd_wr) < 0)
        {
            perror("smash error: close failed");
            return ;
        }
        return ;
    }
    if(close(fd_wr)<0)
    {
        perror("smash error: close failed");
        return;
    }
    cout<<"replaced "<<counter<<" instances of the string "<<'\"'<<source<< '\"'<<endl;
     return ;
}
int FareCommand::FareRead(std::string &my_file, std::string &line_to_read)
{
    ifstream file_read(my_file);
    if(!file_read)
    {
        perror("smash error: open failed");
        return -1;
    }
    char ch;
    while(file_read.get(ch))
    {
        line_to_read.push_back(ch);
    }
    file_read.close();
    return 0;
}
/////////////////set core///////////////////
SetcoreCommand::SetcoreCommand(const char* cmd_line,JobsList* jobs):BuiltInCommand(cmd_line),jobs(jobs){}

void SetcoreCommand::execute()
{
    SmallShell& shell=SmallShell::getInstance();
    ///////ERROR HANDLING///////////
    if(num_of_arguments>2||num_of_arguments<2)
    {
        cerr<<"smash error: setcore: invalid arguments"<<endl;
        return;
    }
    JobsList::JobEntry* my_job=jobs->getJobById(stoi(arguments[1]));
    if(my_job== nullptr)
    {
        cerr << "smash error: bg: job-id " << arguments[1] << " does not exist" << endl;
        return;
    }
    size_t core=stoi(arguments[2]);
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core,&set);
    if(sched_setaffinity(my_job->Job_PID, sizeof(cpu_set_t),&set)==-1)
    {
        perror("smash error: setcore: invalid core number");
        return;
    }
    return;
}