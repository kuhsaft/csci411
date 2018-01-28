//
// Created by Peter on 1/28/2018.
//

#ifndef CSCI411_STRING_UTIL_H
#define CSCI411_STRING_UTIL_H

#include <string>
#include <list>

/// Splits input string into components via whitespace
/// @param input the string to split
/// @returns a list of the elements
std::list<std::string> splitString(const std::string &input);

/// Joins the list of strings with the delimeter
/// @param list the list of strings to join
/// @param delim the delimiter used to join the strings
/// @returns the joined list
std::string joinString(const std::list<std::string> &list, const std::string &delim);

/// Trims whitespace from start and end of string
/// @param str the string to trim
void trim(std::string &str);

#endif //CSCI411_STRING_UTIL_H
