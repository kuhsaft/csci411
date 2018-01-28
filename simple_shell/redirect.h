//
// Created by Peter on 1/28/2018.
//

#ifndef CSCI411_REDIRECT_H
#define CSCI411_REDIRECT_H

#include <string>

/// Redirects stdout to file
/// @param redirect_file the file to redirect to
/// @returns false if invalid file
bool redirect_stdout(const std::string &redirect_file);

/// Restores stdout
void restore_stdout();

#endif //CSCI411_REDIRECT_H
