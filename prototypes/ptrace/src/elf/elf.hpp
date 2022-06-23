//Imports
#include <string>

/**
 * @brief Get the address of the specified symbol
 * 
 * @param path Binary path
 * @param symbol Unmangled symbol name
 * @returns Symbol memory address
 */
unsigned long getSymbols(std::string path, std::string symbol);