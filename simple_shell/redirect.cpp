//
// Created by Peter on 1/28/2018.
//

#include <cstdio>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "redirect.h"

// stdout file descriptor
const int stdoutfd(dup(fileno(stdout)));

bool redirect_stdout(const std::string &redirect_file) {
  // Open output file
  int new_stdout = open(redirect_file.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  // Check if file can be opened
  if (new_stdout == -1) {
    std::cerr << "ERROR: Failed redirect stdout to `" << redirect_file << "`\n";
    return false;
  }

  // Redirect stdout
  fflush(stdout);
  dup2(new_stdout, fileno(stdout));
  close(new_stdout);
}

void restore_stdout() {
  fflush(stdout);
  dup2(stdoutfd, fileno(stdout));
}
