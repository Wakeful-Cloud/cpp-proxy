//Imports
#include <string>
#include <vector>

/**
 * @brief Get the executable path of the specified process
 * 
 * @param pid Process ID
 * @return Executable path
 */
std::string getPath(pid_t pid);

/**
 * @brief Process memory mapping information
 * @see https://man7.org/linux/man-pages/man5/proc.5.html
 */
struct MemoryMapEntry
{
  unsigned long addressStart;
  unsigned long addressEnd;
  unsigned long addressOffset;
  std::string permissions;
  bool is_read;
  bool is_write;
  bool is_execute;
  bool is_shared;
  bool is_private;
  dev_t device;
  ino_t inode;
  std::string path;
};

/**
 * @brief Get the memory mapping information of the specified process
 * 
 * @param pid Process ID
 * @return Memory maps
 */
std::vector<MemoryMapEntry> getMaps(pid_t pid);

/**
 * @brief Get a specific memory mapping entry of the specified process
 * 
 * @param pid Process ID
 * @param path Entry path
 * @return Memory map
 */
MemoryMapEntry getMap(pid_t pid, std::string path);