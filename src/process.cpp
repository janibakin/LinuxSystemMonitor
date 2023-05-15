#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return pid; }

// Return this process's CPU utilization
float Process::CpuUtilization() {
  
  string line, proc_stat;
  std::ifstream stream(LinuxParser::kProcDirectory + std::to_string(pid) + LinuxParser::kStatFilename);
  vector<string> pid_stats = {};
  if(stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    for(int i = 0; i < 22; ++i) {
      line_stream >> proc_stat;
      pid_stats.push_back(proc_stat);
    }
    float HZ = sysconf(_SC_CLK_TCK);
    float utime = std::stof(pid_stats[13]);
    float stime = std::stof(pid_stats[14]);
    float cutime = std::stof(pid_stats[15]);
    float cstime = std::stof(pid_stats[16]);
    float starttime = std::stof(pid_stats[21]);
    float total_time = static_cast<float>(utime + stime + cutime + cstime);
    float up_time = static_cast<float>(LinuxParser::UpTime());

    float start_proc_duration = up_time - (starttime / HZ);
    cpu_util = (total_time / HZ) / start_proc_duration;
  }
  return cpu_util;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return a.cpu_util < cpu_util;
}
