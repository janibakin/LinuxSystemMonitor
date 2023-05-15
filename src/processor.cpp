#include "processor.h"
#include "linux_parser.h"
#include <iostream>

// Return the aggregate CPU utilization
float Processor::Utilization() {
  return static_cast<float>(LinuxParser::ActiveJiffies()) / LinuxParser::Jiffies();
}
