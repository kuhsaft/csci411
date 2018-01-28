//
// Created by Peter on 1/28/2018.
//

#include <iostream>
#include <unistd.h>
#include "string_util.h"
#include "command.h"

void print_usage() {
    std::cout << "\nCommands:\n"
              << "  myprocess (returns the current process ID)\n"
              << "  allprocesses (returns all current processes)\n"
              << "  chgd [directory] (changes the current working directory)\n"
              << "  clr (clears the screen)\n"
              << "  dir [directory] (lists the contents of directory)\n"
              << "  environ (lists all environment variables)\n"
              << "  repeat <string> (prints the string to stdout)\n"
              << "  hiMom (forks a process and waits for response)\n"
              << "  quit (quits the shell)\n"
              << "  help (displays this message)\n"
              << std::endl;

    std::cout << "Note:\n"
              << "  stdout can be redirected via `< <file>`\n"
              << std::endl;
}

bool hi_mom() {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        std::cerr << "ERROR: Could not create pipe\n";
        return false;
    }

    pid_t cpid = fork();
    if (cpid == -1) {
        std::cerr << "ERROR: Could not create child process\n";
        return false;
    }

    if (cpid == 0) { // Child process
        write(pipefd[1], "Hi Mom!", 8);
        close(pipefd[1]);
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);
    } else { //Parent process
        char buf[120];
        read(pipefd[0], &buf, 8);
        std::cout << "Message from child: " << buf << "\n";
        close(pipefd[0]);
        close(pipefd[1]);
        return true;
    }
}

std::string getCommand() {
    std::string input;

    std::cout << "simplesh $ ";
    std::getline(std::cin, input);
    trim(input);
    return input;
}

bool parseCommand(const std::string &command,
                  std::string &exec,
                  std::list<std::string> &args,
                  bool &redirect_stdout,
                  std::string &redirect_file,
                  std::fstream &history
) {
    if (command.empty()) {
        exec = std::string();
        args = std::list<std::string>();
        redirect_stdout = false;
        redirect_file = std::string();
        return true;
    }

    // Save to history
    history << command << std::endl;

    bool is_valid = true;

    std::list<std::string> cmd = splitString(command);
    exec = cmd.front();
    cmd.pop_front();

    redirect_stdout = false;
    auto cmd_iter = cmd.begin();
    while (cmd_iter != cmd.end() && *cmd_iter != ">") ++cmd_iter;
    if (cmd_iter != cmd.end()) {
        // Remove ">" and increment iterator
        cmd_iter = cmd.erase(cmd_iter);
        if (cmd_iter == cmd.end()) {
            std::cerr << "ERROR: Missing file location for redirect";
            redirect_file = std::string();
            redirect_stdout = false;
            is_valid = false;
        } else {
            redirect_file = *cmd_iter;
            cmd.erase(cmd_iter);
            redirect_stdout = true;
            is_valid = true;
        }
    }

    args = cmd;
    return is_valid;
}

bool runCommand(const std::string &cmd,
                const std::list<std::string> &args,
                const std::function<void()> &quit
) {
    // Do nothing if empty input
    if (cmd.empty()) {
        std::cout << std::endl;
        return true;
    }

    if (cmd == "myprocess") {
        std::cout << getpid() << std::endl;
    } else if (cmd == "allprocesses") {
        system("ps");
    } else if (cmd == "chgd") {
        if (!args.empty()) {
            if (args.size() > 1) {
                std::cerr << "ERROR: Too many arguments\n";
                return false;
            } else if (chdir(args.front().c_str()) == -1) {
                std::cerr << "ERROR: Cannot find the path `" << cmd.front() << "`\n";
                return false;
            }
        }
    } else if (cmd == "clr") {
        system("clear");
    } else if (cmd == "dir") {
        std::string arguments = joinString(args, " ");
        std::string ls_cmd = "ls -al " + arguments;
        system(ls_cmd.c_str());
    } else if (cmd == "environ") {
        system("env");
    } else if (cmd == "repeat") {
        std::string arguments = joinString(args, " ");
        std::string echo_cmd = "echo " + arguments;
        system(echo_cmd.c_str());
    } else if (cmd == "hiMom") {
        hi_mom();
    } else if (cmd == "quit") {
        quit();
    } else if (cmd == "help") {
        print_usage();
    } else {
        std::string arguments = joinString(args, " ");
        std::string sys_cmd = cmd + " " + arguments;
        system(sys_cmd.c_str());
    }

    return true;
}
