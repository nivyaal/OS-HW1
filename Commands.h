#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <climits>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <utility>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <vector>



//******************************************************************************** Command  *********************************************************

class JobsList;
class AlarmList;

class Command {
// TODO: Add your data members
protected:
const char* cmd_line;
 public:
  Command(const char* line):cmd_line(line){};
  virtual ~Command(){};
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
  std::string getCmdLine(){return cmd_line;};

};

//UTILS
std::string _ltrim(const std::string& s);
std::string _rtrim(const std::string& s);
std::string _trim(const std::string& s);
int _parseCommandLine(const char* cmd_line, char** args);
bool _isBackgroundComamnd(const char* cmd_line);
std::string _removeBackgroundSign(std::string str);
bool redirectOneArrow(const char* cmd_line);
bool redirectTwoArrows(const char* cmd_line);
class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~ExternalCommand() {}
  void execute() override;
};



//**************************************************************************** Command types Classes  *********************************************************

//                                                                                SPECIAL COMMANDS
class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~PipeCommand() {}
  void execute() override;
  static std::string isolateFirstCommand(const char* cmd_line);
  static std::string isolateSecondCommand(const char* cmd_line);
  static bool isAndPipe(const char* cmd_line);
  static bool isPipe(const char* cmd_line);
  static bool isBuiltInCommand(std::string cmd_str_first_command);
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~RedirectionCommand(){}
  void execute() override;
  static bool isOneArrow(const char* cmd_line);
  static bool isTwoArrows(const char* cmd_line);
  bool redirectOneArrow(const char* cmd_line);
  bool redirectTwoArrows (const char* cmd_line);
  //void prepare() override;
  //void cleanup() override;
};

class TimeOutCommand : public BuiltInCommand{
  public:
  TimeOutCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~TimeOutCommand() {}
  void execute() override;
  time_t getAlarmTime(const char* cmd_line);
};

//                                                                                BUILT IN COMMANDS


class ChangePromptCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  ChangePromptCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~ChangePromptCommand(){}
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
std::string& lastPwdptr;
public:
  ChangeDirCommand(const char* cmd_line,std::string& plastPwd):BuiltInCommand(cmd_line),lastPwdptr(plastPwd)
  {};
  virtual ~ChangeDirCommand(){}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
   JobsList* jobs_list;
   public:
  QuitCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs_list(jobs){};
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs_list;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs_list(jobs){};
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs_list;
 public:
  KillCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs_list(jobs){};
  virtual ~KillCommand() {}
  void execute() override;
  int signalStringToInt(std::string signal);
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs_list;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line), jobs_list(jobs){};
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
  JobsList* jobs_list;
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),jobs_list(jobs){};
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class CatCommand : public BuiltInCommand {
 public:
  CatCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~CatCommand() {}
  void execute() override;
};



//******************************************************************************** ALARM LIST  *********************************************************

class AlarmList {

  std::map<time_t,pid_t> alarm_list;
  public:
  AlarmList():alarm_list(std::map<time_t, pid_t>()){};
  ~AlarmList(){};
  void setNewAlarm(const time_t time,const pid_t job_pid);
  pid_t removeLastAlarm();
  pid_t getLastAlarmPid()
  {return alarm_list.begin()->second;
  };


};


//******************************************************************************** JOBS LIST  *********************************************************

class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
   private:
   bool is_done;
   bool is_stopped;
   int job_id;
   time_t insertion_time;
   pid_t job_pid;
   std::string job_command;
   public:
   JobEntry(bool done_status,bool stopped_status,int id,time_t time,pid_t pid,std::string command):
   is_done(done_status),is_stopped(stopped_status),job_id(id),insertion_time(time),job_pid(pid),job_command(command){};
   bool isJobDone(){  return this->is_done;};
   bool isJobStopped(){ return this->is_stopped;};
   int getJobId(){  return this->job_id;};
   pid_t getJobPid(){ return this->job_pid;};
   int getJobTime(){  return this->insertion_time;}
   void restartInsertionTime(){ this-> insertion_time = time(nullptr);}
   void stopJob();
   void continueJob();
   void setStatus(int signal);
   std::string getJobCommand(){ return this->job_command;}
  };
 // TODO: Add your data members
  std::vector<JobEntry> jobs_vector;
  int max_job_id;
  int size;
 public:
  JobsList():max_job_id(0),jobs_vector( std::vector<JobEntry>()),size(0){};
  ~JobsList(){};
  void addJob(Command* cmd,pid_t job_pid);
  void printJobsList();
  void killAllJobs(bool print_option);
  JobEntry * getJobById(int jobId);
  JobEntry * getJobByPid(int jobPid);
  void removeFinishedJobs();
  void removeJobByPid(int jobPid);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry* getLastStoppedJob();
  bool isEmpty(){return size == 0;};
  // TODO: Add extra methods or modify exisitng ones as needed
};

//******************************************************************************** SMASH  *********************************************************


class SmallShell {
 private:
  // TODO: Add your data members
  std::string prompt_name;
  std::string previous_path;
  Command* curr_fg_command;
  pid_t smash_pid;
  pid_t curr_fg_pid;
  int max_job_id;
  JobsList job_list;
  AlarmList alarm_list;


  SmallShell();
 public:
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
  void SetShellPrompt(std::string new_header)
  {this->prompt_name=new_header;};
  std::string getShellPrompt(){ return this->prompt_name;}
  std::string& getPrePath(){  return this->previous_path;}
  void setCurrFgPid(pid_t fg_num){  this->curr_fg_pid=fg_num;}
  pid_t getCurrFgPid(){ return this->curr_fg_pid;}JobsList* getJobsList(){  return &this->job_list;}
  AlarmList* getAlarmsList(){  return &this->alarm_list;}
  pid_t getSmashPid(){  return smash_pid;}
  void setCurrFgCommand(Command* curr_fg) {curr_fg_command = curr_fg;}
  Command* getCurrFgCommand() { return this-> curr_fg_command;}
};

#endif //SMASH_COMMAND_H_
