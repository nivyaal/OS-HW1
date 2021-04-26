#include "Commands.h"

#define WHITESPACE " "
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
using namespace std;
#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

//                                                      STRING PARSING FUNCTIONS
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}


int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(cmd_line).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

std::string _removeBackgroundSign(std::string str) {
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return "";
  }
  // if the command line does not end with & then return
  if (str[idx] != '&') {
    return str;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  str[idx] = ' ';
  // truncate the command line string up to the last non-space character
  str=_trim(str);
  return str;
}



//************************************************************************** Command Class ***************************************************************
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
  std::string cmd_s = _trim(string(cmd_line));
  cmd_s = _removeBackgroundSign(cmd_s);
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  firstWord = _trim(firstWord);
  if ( firstWord.compare("timeout") == 0)
  {
    return new TimeOutCommand(cmd_line);
  }
  else if (RedirectionCommand::isOneArrow(cmd_line) || RedirectionCommand::isTwoArrows(cmd_line))
  {
    return new RedirectionCommand(cmd_line);
  }
  else if ( PipeCommand::isPipe(cmd_line) || PipeCommand::isAndPipe(cmd_line))
  {
    return new PipeCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0) // builtin W arguments
  {
    return new ChangePromptCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0)
  {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("pwd") == 0)
  {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0) // builtin W arguments
  {
    return new ChangeDirCommand(cmd_line,this->getPrePath());
  }
  else if (firstWord.compare("jobs") == 0)
  {
    return new JobsCommand(cmd_line,this->getJobsList());
  }
  else if (firstWord.compare("kill") == 0) // builtin W arguments
  {
    return new KillCommand(cmd_line,this->getJobsList());
  }
  else if (firstWord.compare("fg") == 0)
  {
    return new ForegroundCommand(cmd_line,this->getJobsList());
  }
  else if (firstWord.compare("bg") == 0)
  {
    return new BackgroundCommand(cmd_line,this->getJobsList());
  }
  else if (firstWord.compare("quit") == 0)
  {
    return new QuitCommand(cmd_line,this->getJobsList());
  }
  else if ( firstWord.compare("cat") == 0) // builtin W arguments
  {
    return new CatCommand(cmd_line);
  }
  else
  {
    return new ExternalCommand(cmd_line);
  }  
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
  this->getJobsList()->removeFinishedJobs();
  Command* cmd = CreateCommand(cmd_line);
  std::string cmd_s = _trim(string(cmd_line));
  std::string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  cmd->execute();
  delete cmd;
}


//************************************************************************** SMASH ***************************************************************
// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() :prompt_name("smash> "),previous_path(""),curr_fg_pid(-1),max_job_id(1),smash_pid(getpid()){};

SmallShell::~SmallShell() {
// TODO: add your implementation
}

string getPwd()
{
  char buff[PATH_MAX];
  return getcwd(buff,sizeof(buff));
}



//************************************************************************** Jobs List Class ***************************************************************
void JobsList::addJob(Command* cmd,pid_t job_pid)
{
    time_t temp_time=time(nullptr);
    JobEntry curr_job(0,0,max_job_id+1,temp_time,job_pid,cmd->getCmdLine());
    max_job_id++;
    this->jobs_vector.push_back(curr_job);
    size++;
}

void JobsList::printJobsList()
{
    if (jobs_vector.empty())
    {
        return;
    }
   int cnt =0;
   for (auto &it : jobs_vector)
   {
        int diff = difftime(time(nullptr),it.getJobTime());
       std::cout<< "[" + std::to_string(it.getJobId()) + "] "+ it.getJobCommand() +  " : " +
        std::to_string(it.getJobPid()) +" "+ std::to_string(diff) + " secs" ;

        if (it.isJobStopped())
        {
            std::cout<<" (stopped)";
        }
        std::cout<<std::endl;
   }
    return;
}

void JobsList::JobEntry::stopJob()
{
  this->is_stopped=1;
  if (kill(job_pid,19) == -1)
  {
    perror("smash error: kill failed");
  }
}



void JobsList::JobEntry::continueJob()
{  
  this->is_stopped=0;
  if (kill(job_pid,18) == -1)
  {
    perror("smash error: kill failed");
  }
}

void JobsList::JobEntry::setStatus(int signal)
{
  if (signal == SIGSTOP)
  {
    this->is_stopped =1;
  }
  else if (signal == SIGCONT)
  {
    this -> is_stopped =0;
  }
}

void JobsList::killAllJobs(bool print_option)
{ 
  if (print_option == 1)
  {
    std::cout<< "smash: sending SIGKILL signal to "<<jobs_vector.size()<<" jobs:"<<std::endl;
  
  for ( auto& it: jobs_vector)
    {
      if (kill(it.getJobPid(),9) == -1)
      {
        perror("smash error: kill failed");
      }
      if (print_option == 1)
      {
        std::cout<<it.getJobPid()<<": "<<it.getJobCommand()<<std::endl;
      }  
    }
  }
  else
  {
    exit(0);
  }
}

void JobsList::removeFinishedJobs()
{
  pid_t pid;
  while ((pid = waitpid(-1, nullptr, WNOHANG)) > 0)
  {
    removeJobByPid(pid);
  }
}

void JobsList::removeJobByPid(int jobId)
{
  for( auto it = jobs_vector.begin();it!=jobs_vector.end(); it++)
  {
    if (jobId == it->getJobPid())
    {
      int jobPid = it->getJobId();
      jobs_vector.erase(it);
      size--;
      if (jobs_vector.size() == 0)
      {
        this->max_job_id = 0;
      }
      else
      {
        this->max_job_id = jobs_vector.back().getJobId();  
      }
      break;
    } 
  }
}

JobsList::JobEntry* JobsList::getJobById(int jobId)
{
  for (auto &it : jobs_vector)
  {
    if (it.getJobId() == jobId)
    {
      return &it;
    }
  }
  return nullptr;
}

JobsList::JobEntry* JobsList::getJobByPid(int jobPid)
{
  for (auto &it : jobs_vector)
  {
    if (it.getJobPid() == jobPid)
    {
      return &it;
    }
  }
  return nullptr;
}

  JobsList::JobEntry * JobsList::getLastJob(int* lastJobId)
  {
    if (jobs_vector.empty())
    {
      *lastJobId=-1;
      return nullptr;
    }
    if (lastJobId !=nullptr)
    {
      *lastJobId = jobs_vector.back().getJobId();
    }
    return &jobs_vector.back();
  }

  JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId)
    {
      *jobId=0;
      for ( auto it = jobs_vector.rbegin(); it!=jobs_vector.rend();it++ )
      {
        if (it->isJobStopped())
        {
          *jobId = it->getJobId();
           break;
        }
      }
      return getJobById(*jobId);
    }

//************************************************************************** ALARM LIST ***************************************************************

void AlarmList::setNewAlarm(const time_t future_time,const pid_t job_pid)
{
  alarm_list.insert({future_time,job_pid});
  if (alarm_list.begin()->second == job_pid)
  {
    int temp = future_time-time(nullptr);
     alarm(future_time-time(nullptr));
  }
  return;
}

  pid_t AlarmList::removeLastAlarm()
  {
    pid_t procc_to_remove = alarm_list.begin()->second;
    alarm_list.erase(alarm_list.begin());
    if (!alarm_list.empty())
    {
      alarm(alarm_list.begin()->first-time(nullptr));
    }  
    return procc_to_remove;
  }



//************************************************************************** Commands Virtual Execute ***************************************************************


void ChangePromptCommand::execute()
{
    SmallShell &smash = SmallShell::getInstance();
    char **args = new char *[COMMAND_MAX_ARGS];
  if (_parseCommandLine(cmd_line, args) == 1)
  {
    smash.SetShellPrompt("smash> ");
  } 
  else
  {
    smash.SetShellPrompt(string(args[1])+"> ");
  }
  delete[] args;
  return;
}

void JobsCommand::execute()
{
  jobs_list->printJobsList();
}

void KillCommand::execute()
{
  char **args = new char *[COMMAND_MAX_ARGS];
  if (_parseCommandLine(cmd_line, args) != 3) 
  {
    cerr << "smash error: kill: invalid arguments" << endl;
    delete[] args;
    return;
  }
  std::string signal_str = string(args[1]);
  std::string job_id_str = string(args[2]);
  delete[] args;
  int signal = stoi(signal_str);
  int job_id = stoi(job_id_str);
  signal=-signal;
  if (signal > 64 || signal <1)
  {
    cerr << "smash error: kill: invalid arguments" << endl;
    return;
  }
  JobsList::JobEntry* job = this->jobs_list->getJobById(job_id);
  if (job == nullptr)
  {
    cerr << "smash error: kill: job-id <job-id> does not exist" << endl;
    return;
  }
  if (kill(job->getJobPid(),signal) == -1)
  {
    perror("smash error: kill failed");
  }
  else
  {
    if (signal == SIGCONT || signal == SIGSTOP)
    {
      job->setStatus(signal);
    }
    std::cout<< "signal number " + std::to_string(signal) + " was sent to pid " + std::to_string(job->getJobPid())<<std::endl;
  }
  return;
}

void ChangeDirCommand::execute()
{
  char **args = new char *[COMMAND_MAX_ARGS];
  if (_parseCommandLine(cmd_line, args) != 2)
  {
    std::cerr<<("smash error: cd: too many arguments")<<std::endl;
    delete[] args;
    return;
  } 
  std::string new_path;  
  if (strcmp(args[1],"-") == 0 )
  {
    if ((this->lastPwdptr).empty())
    {
      std::cerr << "smash error: cd: OLDPWD not set" << endl;
      return;
    }
    else
    {
      new_path=lastPwdptr;
    }
  }
  else
  {
    new_path=args[1];
  }
    char temp[PATH_MAX];
    char* curr_path=getcwd(temp, PATH_MAX);;
  if (chdir(new_path.c_str()) == -1)
  {
    perror("smash error: chdir failed");
    return;
  }
  else
  {
    lastPwdptr=curr_path;
  }
  return;
}

void CatCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  char **args = new char *[COMMAND_MAX_ARGS];
  int num_of_files =   _parseCommandLine(cmd_line, args);
  delete[] args;
  num_of_files--;
  if (num_of_files == 0)
  {
    cerr<<"smash error: cat: not enough arguments"<<std::endl;
  }
  int fd;
  char* buf[1024];
  for (int i=0;i<num_of_files;i++)
  {
    fd = open(args[i+1],O_RDWR);
    if (fd <0)
    {
      perror("smash: open failed");
      return;
    }
    int read_data;
    while(read_data = read(fd,buf,1) )
    {
      if (read_data == -1)
      {
        perror("smash: read failed");
        return;
      }
      int write_data = write(STDOUT_FILENO,buf,1);
      if ( write_data == -1) 
      {
        perror("smash: write failed");
        return;
      }
    }
    close(fd);
  } 
}

void ShowPidCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  std::cout<<"smash pid is "<<smash.getSmashPid()<<std::endl;
  return;  
}

void GetCurrDirCommand::execute()
{
  std::cout<<getPwd()<<std::endl;
  return;
}

void ForegroundCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  char **args = new char *[COMMAND_MAX_ARGS];
  int num_of_args =   _parseCommandLine(cmd_line, args);
  int status;
  JobsList::JobEntry * job;
  if (num_of_args > 2)
  {
    std::cerr<<("smash error: fg: invalid arguments")<<std::endl;
    delete[] args;
    return;
  }
  else if (num_of_args == 1)
  {
    if (smash.getJobsList()->isEmpty())
    {
        std::cerr<<("smash error: fg: jobs list is empty")<<std::endl;
        delete[] args;
        return;
    }
    job = smash.getJobsList()->getLastJob(nullptr);
  }
  else
  {
    int job_id = stoi(string(args[1]));
    job = smash.getJobsList()->getJobById(job_id);
    if (job == nullptr)
    {
      cerr << "smash error: fg: job-id "+string(args[1]) +"does not exist"<<std::endl;
      delete[] args;
      return;
    }
  }
  if (job->isJobStopped())
  {
    job->continueJob();
  }
  smash.setCurrFgPid(job->getJobPid());
  smash.setCurrFgCommand(smash.CreateCommand(job->getJobCommand().c_str()));
  smash.getJobsList()->removeJobByPid(job->getJobPid());
  cout <<job->getJobCommand() +" : " + to_string(job->getJobPid())<< std::endl; 
  waitpid(job->getJobPid(), &status, WUNTRACED);
  /*if (WIFEXITED(status) || WIFSIGNALED(status))
  {
    smash.getJobsList()->removeJobByPid(job->getJobPid());
  }*/
  delete[] args;
}

void BackgroundCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  char **args = new char *[COMMAND_MAX_ARGS];
  int num_of_args =   _parseCommandLine(cmd_line, args);
  int status;
  JobsList::JobEntry * job;
  if (num_of_args > 2)
  {
    std::cerr<<("smash error: fg: invalid arguments")<<std::endl;
    delete[] args;
    return;
  }
  else if (num_of_args == 1)
  {
    job = smash.getJobsList()->getLastStoppedJob(nullptr);
    if (job == nullptr)
    {
        std::cerr<<("smash error: bg: there is no stopped jobs to resume")<<std::endl;
        delete[] args;
        return;
    }
  }
  else
  {
    int job_id = stoi(string(args[1]));
    job = smash.getJobsList()->getJobById(job_id);
    if (job == nullptr)
    {
      cerr << "smash error: bg: job-id "+string(args[1]) +"does not exist"<<std::endl;
      delete[] args;
      return;
    }
    if (job->isJobStopped())
    {
      cerr << "smash error: bg: job-id "+string(args[1]) +"is already running in the background"<<std::endl;
      delete[] args;
      return;
    }
  }
  job->continueJob();
  return;
}
void ExternalCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  pid_t pid = fork();
  if (pid >0 ) //father
  {
    if (_isBackgroundComamnd(this->getCmdLine().c_str()))
    {
      smash.getJobsList()->addJob(this,pid);
    }
    else
    {
      smash.setCurrFgPid(pid);
      smash.setCurrFgCommand(this);
      int status;
      waitpid(pid, &status, WUNTRACED);
      if (WIFEXITED(status) || WIFSIGNALED(status))
      {
        smash.getJobsList()->removeJobByPid(pid);
      }
    }
  }
  else //son
  {
    std::string back_ground_command;
    back_ground_command = string(_trim(this->cmd_line));
    if (_isBackgroundComamnd(this->cmd_line))
    {
      int index =back_ground_command.find_last_of("&");
      back_ground_command = back_ground_command.erase(index,index+1);
    }
    setpgrp();
    execlp("/bin/bash", "bash", "-c", back_ground_command.c_str(), NULL);
    perror("smash error: execlp failed");
    exit(0);
  }
}

void QuitCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  char **args = new char *[COMMAND_MAX_ARGS];
  int num_of_args =   _parseCommandLine(cmd_line, args);
  bool print_option = 0;
  if (num_of_args == 2 && string(args[1]).compare("kill") == 0)
  {
    print_option = 1;
  }
  smash.getJobsList()->killAllJobs(print_option);
  exit(0);

}

void RedirectionCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  int saved_stdout = dup(STDOUT_FILENO);
 if (isOneArrow(cmd_line))
 { 
   redirectOneArrow(this->getCmdLine().c_str());
 }
 else if (isTwoArrows(cmd_line))
 {
   redirectTwoArrows((this->getCmdLine()).c_str());
 }  //parse command
  std::string cmd_str = string(cmd_line);
  cmd_str = _trim(cmd_str.erase(cmd_str.find_first_of(">"),cmd_str.size()));
  smash.executeCommand(cmd_str.c_str());
  dup2(saved_stdout,STDOUT_FILENO); 
}

void PipeCommand::execute()
{  
  SmallShell &smash = SmallShell::getInstance();
  int my_pipe[2];
  if (pipe(my_pipe) == -1)
  {
    perror("smash error: pipe failed");
  }
  int w_channel = 1;
  if (isAndPipe(this->getCmdLine().c_str()))
  {
    w_channel = 2;
  }
  std::string cmd_str_first = _trim(isolateFirstCommand(this->cmd_line));
  std::string cmd_str_second = _trim(_removeBackgroundSign(isolateSecondCommand(this->cmd_line)));
  int pid_1 = fork();
  if (pid_1<0)
  {
   perror("smash error: fork failed");
  }
  else if (pid_1==0) //son 1
  {
    dup2(my_pipe[1],w_channel);
    close(my_pipe[0]);
    close(my_pipe[1]);
    smash.executeCommand(cmd_str_first.c_str());
    exit(0);
  }
  int pid_2 = fork();
  if (pid_2<0)
  {
   perror("smash error: fork failed");
  }
  else if (pid_2 == 0) //son 2
  {
    dup2(my_pipe[0],0);
    close(my_pipe[0]);
    close(my_pipe[1]);
    smash.executeCommand(cmd_str_second.c_str());
    exit(0);
  }
  close(my_pipe[0]);
  close(my_pipe[1]);

   waitpid(pid_1,nullptr,0);
   waitpid(pid_2,nullptr,0);
   return;
}

void TimeOutCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  int alarm_time = getAlarmTime(cmd_line);
  pid_t pid = fork();
  if (pid >0)
  {
    smash.getJobsList()->addJob(this,pid);
    smash.getAlarmsList()->setNewAlarm(alarm_time, pid);
    if (!_isBackgroundComamnd(this->cmd_line))
    {
      smash.setCurrFgPid(pid);
      smash.setCurrFgCommand(this);
      int status;
      waitpid(pid, &status, WUNTRACED);
      if (WIFEXITED(status) || WIFSIGNALED(status))
      {
        smash.getJobsList()->removeJobByPid(pid);
      }
    }
  }
else //son
  {
    std::string back_ground_command;
    back_ground_command = string(_trim(this->cmd_line));
    if (_isBackgroundComamnd(this->cmd_line))
    {
      int index =back_ground_command.find_last_of("&");
      back_ground_command = back_ground_command.erase(index,index+1);
    }
    setpgrp();
    back_ground_command = (_trim(back_ground_command));
    back_ground_command = back_ground_command.erase(0,back_ground_command.find_first_of(" "));
    back_ground_command = (_trim(back_ground_command));
    back_ground_command = back_ground_command.erase(0,back_ground_command.find_first_of(" ")); 
    execlp("/bin/bash", "bash", "-c", back_ground_command.c_str(), NULL);
    perror("smash error: execlp failed");
    exit(0);
  }
}

time_t TimeOutCommand::getAlarmTime(const char* cmd_line)
{
  char **args = new char *[COMMAND_MAX_ARGS];
  int num_of_args =   _parseCommandLine(cmd_line, args);
  int alarm = stoi(std::string(args[1]));
  time_t curr_time= time(nullptr);
  return alarm + curr_time;
}


bool PipeCommand::isAndPipe(const char* cmd_line)
{
  std::string str = string(cmd_line);
  if (str.find("|&") != std::string::npos )
  {
    return true;
  }
  else
  {
    return false;
  }
  
}

bool PipeCommand::isPipe(const char* cmd_line)
{
  std::string str = string(cmd_line);
  if ((str.find("|") != std::string::npos) && (str.find("|&") == std::string::npos))
  {
    return true;
  }
  else
  {
    return false;
  }
}

std::string PipeCommand::isolateFirstCommand(const char* cmd_line)
{
  std::string str = string(cmd_line);
  if ((isAndPipe(cmd_line)) || (isPipe(cmd_line)))
  {
    str = str.erase(str.find_first_of("|"),str.size());
  }
  else
  {
    str = "";
  }
  
  return str;
}


std::string PipeCommand::isolateSecondCommand(const char* cmd_line)
{
    std::string str = string(cmd_line);
  if (isAndPipe(cmd_line))
  {
    str = str.erase(0,str.find_first_of("&")+1);

  }
  else if (isPipe(cmd_line))
  {

    str = str.erase(0,str.find_first_of("|")+1);
  }
  else
  {
    str = "";
  }
  
  return str;
}

bool isBuiltInCommand(std::string cmd_str_first_command)
{
  std::string str =cmd_str_first_command ;
      if (str == "chrpompt" ||
        str == "showpid" ||
        str == "pwd" ||
        str == "cd" ||
        str == "jobs" ||
        str == "kill" ||
        str == "fg" ||
        str == "bg" ||
        str == "quit") 
        {
          return true;
        }
        else
        {
          return false;
        }
}

bool RedirectionCommand::isOneArrow(const char* cmd_line)
{
  std::string str = string(cmd_line);
  if ((str.find(">") != std::string::npos) && (str.find(">>") == std::string::npos))
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool RedirectionCommand::isTwoArrows(const char* cmd_line)
{
  std::string str = string(cmd_line);
  if (str.find(">>") != std::string::npos)
  {
    return true;
  }
  else
  {
    return false;
  } 
}

void RedirectionCommand::redirectOneArrow(const char* cmd_line)
{
  std::string cmd_string = string(cmd_line);
  std::string file_name = _trim(cmd_string.erase(0,cmd_string.find_last_of(">")+1));
  int fd =open(file_name.c_str(),O_CREAT|O_RDWR, 0666);
  close(STDOUT_FILENO);
  dup2(fd,STDOUT_FILENO);
}

void RedirectionCommand::redirectTwoArrows(const char* cmd_line)
{
  std::string cmd_string = string(cmd_line);
  std::string file_name = _trim(cmd_string.erase(0,cmd_string.find_last_of(">")+1));
  int fd =open(file_name.c_str(),O_CREAT|O_APPEND|O_RDWR, 0666);
  close(STDOUT_FILENO);
  dup2(fd,STDOUT_FILENO);
}
