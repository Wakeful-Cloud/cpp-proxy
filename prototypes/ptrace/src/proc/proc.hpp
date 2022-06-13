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
 * @brief Get the memory mapping entries of the specified process
 * 
 * @param pid Process ID
 * @return Memory map entries (**Note: this is sensitive because it would allow an attacker to
 * bypass ASLR if leaked!**)
 */
std::vector<MemoryMapEntry> getMapEntries(pid_t pid);