#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// TODO: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  long total, free, buff, cache;
  string key, line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key;
      if(key == "MemTotal:") {
        string total_s;
        line_stream >> total_s;
        total = std::stol(total_s);
      } else if(key == "MemFree:") {
        string free_s;
        line_stream >> free_s;
        free = std::stol(free_s);
      } else if(key == "Buffers:") {
        string buff_s;
        line_stream >> buff_s;
        buff = std::stol(buff_s);
      } else if(key == "Cached:") {
        string cache_s;
        line_stream >> cache_s;
        cache = std::stol(cache_s);
      }
    }
  }
  return static_cast<float>(total - free - cache - buff) / total;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> uptime;
    }
  }
  return std::stol(uptime);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}

// Read and return the number of active jiffies for a PID
// long LinuxParser::ActiveJiffies(int pid) { return 0; }

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long a_jiffies = 0;
  vector<CPUStates> active_states = {
    kUser_,
    kNice_,
    kSystem_,
    kIRQ_,
    kSoftIRQ_,
    kSteal_
  };
  vector<string> utilisation = CpuUtilization();
  for(int state : active_states) {
    a_jiffies += std::stol(utilisation[state]);
  }
  return a_jiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long idle_j = 0;
  vector<CPUStates> idle_states = {
    kIdle_,
    kIOwait_
  };
  vector<string> utilisation = CpuUtilization();
  for(int state : idle_states) {
    idle_j += std::stol(utilisation[state]);
  }
  return idle_j;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> utilisation;
  string line, key, value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key;
      if(key == "cpu") {
        while(line_stream >> value) {
          utilisation.emplace_back(value);
        }
        break;
      }
    }
  }
  return utilisation;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string np, line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key;
      if(key == "processes") {
        line_stream >> np;
        break;
      }
    }
  }
  return std::stoi(np);
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string pr, line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key;
      if(key == "procs_running") {
        line_stream >> pr;
        return std::stoi(pr);
      }
    }
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if(stream.is_open()) {
    if(std::getline(stream, line)) {
      return line;
    }
  }
  return string();
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key, mem;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key;
      if(key == "VmSize:") {
        line_stream >> mem;
        int mem_mb = std::stof(mem) / 1000; // convert kb to Mb
        mem = std::to_string(mem_mb);
        break;
      }
    }
  }
  return mem;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, uid;
  std::ifstream stream_uid(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if(stream_uid.is_open()) {
    while(std::getline(stream_uid, line)) {
      std::istringstream line_stream(line);
      line_stream >> key;
      if(key == "Uid:") {
        line_stream >> uid;
        break;
      }
    }
  }
  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line, key, user;
  string uid = LinuxParser::Uid(pid);
  string string_to_find = "x:" + uid;
  std::ifstream stream_usr(kPasswordPath);
  if(stream_usr.is_open()) {
    while(std::getline(stream_usr, line)) {
      // if string is found
      if(line.find(string_to_find) != string::npos) {
        std::istringstream line_stream(line);
        std::getline(line_stream, user, ':');
        break;
      }
    }
  }
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, key, value;
  long uptime;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    if(std::getline(stream, line)) {
      std::istringstream line_stream(line);
      for(int i = 0; i < 22; i++) {
        line_stream >> value;
      }
      long starttime = std::stol(value) / sysconf(_SC_CLK_TCK);
      uptime = UpTime() - starttime;
    }
  }
  return uptime;
}
