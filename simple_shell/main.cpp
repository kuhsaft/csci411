/*
 * Peter Nguyen
 * CSCI 411 - Shell Program
 *
 * Compile with `-std=c++11`
 */

#include <iostream>
#include <list>
#include <functional>
#include <unistd.h>
#include <pwd.h>
#include <sys/signal.h>
#include "redirect.h"
#include "command.h"

/// Function to run on program exit
/// Defined in main to capture history
std::function<void()> on_quit;

/// Handles interrupt
/// @param signal the interrupt signal
void signal_handler(int signal) {
  std::cout << "\nInterrupted received (" << signal << "). Ctrl+C Pressed.\n";
  on_quit();
};

/// Opens the history file
/// @returns the path to the history file
std::string get_history_filepath() {
  const char *homedir;
  if ((homedir = getenv("HOME")) == nullptr) {
    homedir = getpwuid(getuid())->pw_dir;
  }
  std::string history_filepath(homedir);
  history_filepath += "/.simplesh_history";

  return history_filepath;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int main() {
  std::string history_filepath = get_history_filepath();
  std::fstream history_fs(history_filepath, std::ios::app);
  if (history_fs.fail()) {
    std::cerr << "ERROR: Failed to open history file `" << history_filepath << "`";
  }

  // Set on_exit function capturing history_fs
  on_quit = [&history_fs, &history_filepath]() {
    // Save history
    history_fs.close();
    history_fs.clear();

    // Print history
    std::cout << "\nHistory (" << history_filepath << "):\n";
    std::cout << "------------------------------------------------------------\n";
    history_fs.open(history_filepath, std::ios::in);
    std::cout << history_fs.rdbuf();
    history_fs.close();

    exit(0);
  };

  // Capture Ctrl+C
  signal(SIGINT, &signal_handler);

  std::cout << "Use the command `help` for a list of commands.\n\n";
  while (true) {
    std::string input = getCommand();

    std::string cmd, redirect_file;
    std::list<std::string> args;
    bool should_redirect_stdout;
    bool valid_command = parseCommand(
        input,
        cmd,
        args,
        should_redirect_stdout,
        redirect_file,
        history_fs
    );

    if (valid_command) {
      if (should_redirect_stdout)
        redirect_stdout(redirect_file);

      runCommand(cmd, args, on_quit);

      restore_stdout();
    }
  };
}

#pragma clang diagnostic pop