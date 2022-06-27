//Imports
#include <string>
#include <vector>

/**
 * @brief Symbol metadata
 */
struct Symbol
{
  unsigned long address;
  std::string mangledName;
  std::string unmangledName;
};

/**
 * @brief Get all symbols
 * 
 * @param path Binary path
 * @returns Symbols
 */
std::vector<Symbol> getSymbols(std::string path);