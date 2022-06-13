//Imports
#include "proc.hpp"
#include <fstream>
#include <limits.h>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <vector>

std::string getPath(pid_t pid)
{
  //Format the process info path
  std::string proc = "/proc/" + std::to_string(pid) + "/exe";

  //Read the link
  char buffer[PATH_MAX];
  std::size_t length = readlink(proc.c_str(), buffer, PATH_MAX);

  //Error
  if (length == -1)
  {
    throw std::runtime_error("[Proc] Failed to get executable path!");
  }
  
  //Add null terminator
  buffer[length] = 0;

  //Convert to string
  std::string path(buffer);

  return path;
}

std::vector<MemoryMapEntry> getMapEntries(pid_t pid)
{
  //Format the process info path
  std::string proc = "/proc/" + std::to_string(pid) + "/maps";

  //Open the file
  std::ifstream file(proc);

  //Read line-by-line and parse
  std::string line;
  std::vector<MemoryMapEntry> entries;

  while (std::getline(file, line))
  {
    //Parse the line
    std::smatch matches;
    std::regex regex("^([0-9A-Fa-f]+)-([0-9A-Fa-f]+) ([rwxsp\\-]+) ([0-9A-Fa-f]+) ([0-9A-Fa-f]+):([0-9A-Fa-f]+) ([0-9]+) +(.+)?$");
    if (std::regex_search(line, matches, regex))
    {
      //Create the entry
      MemoryMapEntry entry;
      entry.addressStart = std::stoul(matches[1], nullptr, 16);
      entry.addressEnd = std::stoul(matches[2], nullptr, 16);

      std::string permisions = matches[3].str();
      entry.permissions = permisions;
      entry.is_read = permisions.find("r") != std::string::npos;
      entry.is_write = permisions.find("w") != std::string::npos;
      entry.is_execute = permisions.find("x") != std::string::npos;
      entry.is_shared = permisions.find("s") != std::string::npos;
      entry.is_private = permisions.find("p") != std::string::npos;

      entry.addressOffset = std::stoul(matches[4], nullptr, 16);

      unsigned int maj = std::stoi(matches[5].str(), nullptr, 16);
      unsigned int min = std::stoi(matches[6].str(), nullptr, 16);
      
      entry.device = makedev(maj, min);

      entry.inode = std::stoul(matches[7]);
      entry.path = matches[8].str();

      //Add
      entries.push_back(entry);
    }
  }

  //Close the file
  file.close();

  return entries;
}