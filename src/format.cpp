#include <string>
#include <math.h>
#include <sstream>
#include "format.h"
#include <iomanip>

using std::string;
using std::to_string;
using std::stringstream;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(const long& seconds) { 
  int hh, mm, ss;
  hh = seconds / 3600;
  mm = seconds / 60 - hh * 60;
  ss = seconds - mm * 60 - hh * 3600;
  stringstream stream;
  stream << std::setw(2) << std::setfill('0') << hh << ':' <<
            std::setw(2) << std::setfill('0') << mm << ':' <<
            std::setw(2) << std::setfill('0') << ss;
  string clock = stream.str();
  return clock;
}
