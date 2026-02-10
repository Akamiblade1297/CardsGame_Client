#ifndef STRINGFUNC_H
#define STRINGFUNC_H
#include <vector>
#include <string>

/**
 * @brief Concatenate all strings inside vector with a delimiter
 * @param vec A vector of strings
 * @param del A delimiter to insert between strings
 * @return Concatenated string
 */
std::string join ( const std::vector<std::string>& vec, const std::string& del );

/**
 * @brief Break string into vector by delimiter
 * @param str A C-string to split
 * @param del A delimiter
 * @return Vector of strings
 */
std::vector<std::string> split( char* str, char del );

#endif
