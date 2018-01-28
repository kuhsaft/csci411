//
// Created by Peter on 1/28/2018.
//

#ifndef CSCI411_COMMAND_H
#define CSCI411_COMMAND_H

#include <string>
#include <fstream>
#include <list>
#include <functional>
#include "string_util.h"

// Prints help information
void print_usage();

/// Tests fork exec
/// Creates a child process that send "Hi Mom!" to parent through pipe.
/// Parent process prints message to stdout
/// @retuns false if error
bool hi_mom();

/// Prompts user for a command
/// @return returns command as string
std::string getCommand();

/// Parses the command and appends to history
/// @param command the command to parse
/// @param exec the executable to run
/// @param args the arguments of the command
/// @param redirect_stdout if the output should be redirected
/// @param redirect_file the file to redirect to
/// @param history the fstream to append to
/// @returns true if the command is valid
bool parseCommand(
        const std::string &command,
        std::string &exec,
        std::list<std::string> &args,
        bool &redirect_stdout,
        std::string &redirect_file,
        std::fstream &history
);

/// Runs the command
/// @param cmd the command run
/// @param args the command arguments
/// @param quit the function to run on quit
/// @returns true if the command is successful
bool runCommand(
        const std::string &cmd,
        const std::list<std::string> &args,
        const std::function<void()> &quit
);


#endif //CSCI411_COMMAND_H
