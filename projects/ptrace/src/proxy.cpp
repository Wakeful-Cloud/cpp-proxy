//Imports
#include "elf/elf.hpp"
#include "proc/proc.hpp"
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/sysmacros.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Get the address of the the symbol in the specified process
 *
 * @param pid Process ID
 * @param symbol Unmangled symbol name
 * @return Memory address (**Note: this is sensitive because it would allow an attacker to bypass
 * ASLR if leaked!**)
 */
unsigned long getAddress(pid_t pid, std::string symbol)
{
  //Get the executable path
  std::string path = getPath(pid);

  //Get the memory map
  MemoryMapEntry map = getMap(pid, path);

  //Get the symbol address
  unsigned long symbolAddress = getSymbolAddress(path, symbol);

  //Compute the full address
  unsigned long fullAddress = map.addressStart + symbolAddress;

#ifdef DEBUG
    std::cout << "[Proxy] Base address: " << std::hex << map.addressStart << ", symbol address: " << std::hex << symbolAddress << ", full address: " << std::hex << fullAddress << std::endl;
#endif //DEBUG

  return fullAddress;
}

int main(int argc, char *argv[])
{
  //Validate arguments
  if (argc != 2)
  {
    throw std::runtime_error("[Proxy] Must pass target executable as an argument!");
  }

  //Get the arguments
  char* target = argv[1];

  //Fork this process
  errno = 0;
  pid_t child = fork();

  //Error
  if (child == -1)
  {
    //Get the actual error
    int error = errno;

    throw std::runtime_error("[Proxy] Failed to fork! Error: " + std::string(strerror(error)));
  }
  //Running in child
  else if (child == 0)
  {
    //Enable tracing
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    //Enable remote memory reading

    //Switch to the target
    execl(target, target, NULL);
  }
  //Running in parent
  else
  {
    //Wait for the child to start
    int status;
    wait(&status);

    //Get the address address
    unsigned long address = getAddress(child, "add(int, int)");

    //Get the original instruction
    errno = 0;
    long original = ptrace(PTRACE_PEEKTEXT, child, (void *)address, 0);

    if (original == -1)
    {
      //Get the actual error
      int error = errno;

      throw std::runtime_error("[Proxy] Failed to get the original target instruction data! Error: " + std::string(strerror(error)));
    }

    //Modify the instruction (Add breakpoint instruction)
    long modified = (original & 0xffffff00) | 0xcc;

    //Update the instruction
    ptrace(PTRACE_POKETEXT, child, address, modified);

    //Continue child execution
    ptrace(PTRACE_CONT, child, 0, 0);

    //Wait for the child to hit the breakpoint
    wait(&status);

    if (WIFSTOPPED(status))
    {
#ifdef DEBUG
      std::cout << "[Proxy] Got signal: " << strsignal(WSTOPSIG(status)) << std::endl;
#endif //DEBUG
    }

    //Get the registers
    struct user_regs_struct registers;
    ptrace(PTRACE_GETREGS, child, 0, &registers);

    //Restore the instruction
    ptrace(PTRACE_POKETEXT, child, address, original);

    //Back the IP up one (To before the breakpoint)
#ifdef __x86_64__
    registers.rip--;
#else
    registers.eip--;
#endif

    //Update the registers
    ptrace(PTRACE_SETREGS, child, 0, &registers);

    //Continue child execution
    ptrace(PTRACE_CONT, child, 0, 0);

    //Wait for the child to finish
    wait(&status);

    if (WIFEXITED(status))
    {
#ifdef DEBUG
      std::cout << "[Proxy] Child process exited!" << std::endl;
#endif //DEBUG
    }
  }

  return 0;
};