#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#define SIGSTP 19

#define CALL_SYS(syscall, syscall_name)                \
  do                                                   \
  {                                                    \
    if ((syscall) == -1)                               \
    {                                                  \
      string str_for_perror = string("smash error: "); \
      str_for_perror += syscall_name;                  \
      str_for_perror += " failed";                     \
      perror((char *)str_for_perror.c_str());          \
      return;                                          \
    }                                                  \
  } while (0)


using namespace std;



void ctrlZHandler(int sig_num) {
    SmallShell &smash = SmallShell::getInstance();
    pid_t procc_pid = smash.getCurrFgPid();
    std::cout << "smash: got ctrl-Z" << endl;
    if (procc_pid == -1)
    {
      return; // no process is runing in fg
    }
    else
    {
    CALL_SYS(kill(procc_pid,SIGSTOP),"kill");
    std::cout << "smash: process " << to_string(procc_pid) << " was stopped"<< endl; 
    JobsList::JobEntry* job = smash.getJobsList()->getJobByPid(procc_pid);
    if (job == nullptr)
    {
      smash.getJobsList()->addJob(smash.getCurrFgCommand(), procc_pid);
    }
    else
    {
      job->restartInsertionTime();
    }
    smash.getJobsList()->getJobByPid(procc_pid)->setStatus(SIGSTOP);
    smash.setCurrFgPid(-1);
    smash.setCurrFgCommand(nullptr);
  }
}

void ctrlCHandler(int sig_num) {
    SmallShell &smash = SmallShell::getInstance();
    pid_t procc_pid = smash.getCurrFgPid();
    std::cout << "smash: got ctrl-C" << endl;
    if (procc_pid == -1)
    {
      return ;//no process is running in fg
    }
    else
    {
      CALL_SYS(kill(procc_pid,SIGKILL),"kill" );
    }
    std::cout << "smash: process " << to_string(procc_pid) << " was killed"<< endl; 
}

void alarmHandler(int sig_num) {
  std::cout<<"im in "<<std::endl;
  // TODO: Add your implementation
 SmallShell &smash = SmallShell::getInstance();
  pid_t procc_pid = smash.getAlarmsList()->getLastAlarmPid();
  if (kill(procc_pid,0) == -1 ) // in case the process has already ended
  {
    int res =waitpid(procc_pid,nullptr,WNOHANG); // kill Zombie;
    std::cout << "smash: got an alarm"<<std::endl;
    return;
  }
  else
  {
    CALL_SYS(kill(procc_pid,SIGKILL),"kill");
    int res =waitpid(procc_pid,nullptr,WNOHANG); // kill Zombie;
    std::string cmd_line=smash.getCurrFgCommand()->getCmdLine();
    if ( smash.getCurrFgPid() == -1 || smash.getCurrFgPid() != procc_pid) // external command
    {
       cmd_line = smash.getJobsList()->getJobByPid(procc_pid)->getJobCommand();
    }
    std::cout << "smash: got an alarm\nsmash" + cmd_line+ " timed out!" << std::endl;
  }
}
