#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "format.h"
#include "ncurses_display.h"
#include "system.h"

using std::string;
using std::to_string;

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent) {
  std::string result{"0%"};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100%";
}

void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  std::string os_info = "OS: " + system.OperatingSystem();
  mvwprintw(window, ++row, 2, "%s", os_info.c_str());

  std::string kernel_info = "Kernel: " + system.Kernel();
  mvwprintw(window, ++row, 2, "%s", kernel_info.c_str());

  mvwprintw(window, ++row, 2, "CPU: ");
  wattron(window, COLOR_PAIR(1));
  wmove(window, row, 10);
  wclrtoeol(window);
  std::string cpu_progress = ProgressBar(system.Cpu().Utilization());
  wprintw(window, "%s", cpu_progress.c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2, "Memory: ");
  wattron(window, COLOR_PAIR(1));
  wmove(window, row, 10);
  wclrtoeol(window);
  std::string memory_progress = ProgressBar(system.MemoryUtilization());
  wprintw(window, "%s", memory_progress.c_str());  wattroff(window, COLOR_PAIR(1));
  std::string total_procs = "Total Processes: " + std::to_string(system.TotalProcesses());
  mvwprintw(window, ++row, 2, "%s", total_procs.c_str());
  std::string running_procs = "Running Processes: " + std::to_string(system.RunningProcesses());
  mvwprintw(window, ++row, 2, "%s", running_procs.c_str());
  std::string uptime = "Up Time: " + Format::ElapsedTime(system.UpTime());
  mvwprintw(window, ++row, 2, "%s", uptime.c_str());
  wrefresh(window);
}

void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes,
                                      WINDOW* window, int n) {
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{16};
  int const ram_column{26};
  int const time_column{35};
  int const command_column{46};

  [[maybe_unused]] int rows, cols;
  getmaxyx(window, rows, cols);  // Get window dimensions

  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));
  for (int i = 0; i < n; ++i) {
    // Clear the line
    std::string empty_space(cols - 2, ' ');
    mvwprintw(window, ++row, pid_column, "%s", empty_space.c_str());

    mvwprintw(window, row, pid_column, "%d", processes[i].Pid());
    mvwprintw(window, row, user_column, "%s", processes[i].User().c_str());
    float cpu = processes[i].CpuUtilization() * 100;
    mvwprintw(window, row, cpu_column, "%s", to_string(cpu).substr(0, 4).c_str());
    mvwprintw(window, row, ram_column, "%s", processes[i].Ram().c_str());
    mvwprintw(window, row, time_column, "%s", Format::ElapsedTime(processes[i].UpTime()).c_str());
    mvwprintw(window, row, command_column, "%s", processes[i].Command().substr(0, cols - command_column).c_str());
  }
}

void NCursesDisplay::Display(System& system, int n) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color

  int x_max{getmaxx(stdscr)};
  WINDOW* system_window = newwin(9, x_max - 1, 0, 0);

  int system_rows, system_cols;
  getmaxyx(system_window, system_rows, system_cols);

  WINDOW* process_window = newwin(3 + n, x_max - 1, system_rows + 1, 0);

  while (1) {
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplaySystem(system, system_window);
    DisplayProcesses(system.Processes(), process_window, n);
    wrefresh(system_window);
    wrefresh(process_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  endwin();
}
