#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#define SIGSTP 19

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

  std::cout << "smash: process " << to_string(procc_pid) << " was stopped"<< endl; 
  smash.getJobsList()->getJobByPid(procc_pid)->stopJob();
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
      if (kill(procc_pid,SIGKILL) == -1 )
      {
        perror("smash error: kill failed");
      }
    }
    std::cout << "smash: process " << to_string(procc_pid) << " was killed"<< endl; 
}

void alarmHandler(int sig_num) {
  
  // TODO: Add your implementation
 SmallShell &smash = SmallShell::getInstance();
  pid_t procc_pid = smash.getAlarmsList()->getLastAlarmPid();
  if (kill(procc_pid,SIGKILL) == -1 )
  {
    perror("smash error: kill failed");
  }
  int res =waitpid(procc_pid,nullptr,WNOHANG); // kill Zombie;
  std::string cmd_line=smash.getCurrFgLine();
  if ( smash.getCurrFgPid() == -1) // external command
  {
     cmd_line = smash.getJobsList()->getJobByPid(procc_pid)->getJobCommand();
  }
  std::cout << "smash: got an alarm\nsmash: " + cmd_line+ " timed out!" << std::endl;
}



