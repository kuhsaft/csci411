/*
 * Peter Nguyen
 * CSCI 411 - Observing Linux Behavior
 *
 * Compile with `-std=c++11`
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <regex>
#include <cmath>

int main() {
  /************
   * Hostname *
   ************/
  std::cout << "A: Hostname: " << std::ifstream("/proc/sys/kernel/hostname").rdbuf() << std::endl;

  // Read /proc/cpuinfo
  std::ifstream cpuinfo_ifs("/proc/cpuinfo");
  std::string cpuinfo(
    (std::istreambuf_iterator<char>(cpuinfo_ifs)),
    (std::istreambuf_iterator<char>())
  );

  /******************************
   * Number of processing units *
   ******************************/
  size_t num_processors = 0;

  // Search for 'processor' in cpuinfo
  std::regex processors_regex("processor.*");
  std::smatch processors_match;
  std::string::const_iterator search_start(cpuinfo.cbegin());
  while (std::regex_search(search_start, cpuinfo.cend(), processors_match, processors_regex)) {
    ++num_processors; // Increment count each time found
    search_start += processors_match.position() + processors_match.length();
  };

  std::cout << "B: Number of processing units: " << num_processors << std::endl;
  std::cout << std::endl;

  /***************
   * CPU details *
   ***************/
  std::cout << "C: CPU(s) Type and Model:" << std::endl;

  // Search for 'processor', 'vendor_id', and 'model'
  std::regex processor_info_regex("(processor|vendor_id|model).*");
  std::smatch processors_info_match;
  search_start = cpuinfo.cbegin();
  while (std::regex_search(search_start, cpuinfo.cend(), processors_info_match, processor_info_regex)) {
    std::cout << "    " << processors_info_match[0] << std::endl; // Output each match
    search_start += processors_info_match.position() + processors_info_match.length();
  } 
  std::cout << std::endl;

  /******************
   * Kernel version *
   ******************/
  std::cout << "D: Linux Kernel Version: " << std::ifstream("/proc/version").rdbuf() << std::endl;

  /**********
   * Uptime *
   **********/
  // Read in /proc/uptime
  double uptime, idle_time;
  std::ifstream("/proc/uptime") >> uptime >> idle_time;

  // Output
  std::cout << "E: System time:" << std::endl;

  std::cout << std::setprecision(2) << std::fixed
            << "    Uptime: " << uptime << " seconds or "
            << static_cast<size_t>(uptime / 3600) << "h:"
            << static_cast<size_t>((static_cast<size_t>(uptime) % 3600) / 60) << "m:"
            << std::fmod(uptime, 60) << "s" << std::endl;

  std::cout << std::setprecision(2) << std::fixed
            << "    Idle Time: " << idle_time << " seconds or "
            << static_cast<size_t>(idle_time / 3600) << "h:"
            << static_cast<size_t>((static_cast<size_t>(idle_time) % 3600) / 60) << "m:"
            << std::fmod(idle_time, 60) << "s" << std::endl;

  std::cout << std::endl;

  /**********************
   * Memory information *
   **********************/
  std::cout << "F: Memory Information:" << std::endl;

  // Read /proc/meminfo
  std::ifstream meminfo_ifs("/proc/meminfo");
  std::string meminfo(
    (std::istreambuf_iterator<char>(meminfo_ifs)),
    (std::istreambuf_iterator<char>())
  );

  // Search for 'MemTotal' and 'MemFree'
  std::regex meminfo_regex("(MemTotal|MemFree).*");
  std::smatch meminfo_match;
  search_start = meminfo.cbegin();
  while (std::regex_search(search_start, meminfo.cend(), meminfo_match, meminfo_regex)) {
    std::cout << "    " << meminfo_match[0] << std::endl; // Output each match
    search_start += meminfo_match.position() + meminfo_match.length();
  } 

  return 0;
}

