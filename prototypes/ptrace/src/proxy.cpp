//Imports
#include "elf/elf.hpp"
#include "proc/proc.hpp"
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Get the address of the the symbol in the specified process
 *
 * @param pid Process ID
 * @param entries Process memory map entries
 * @param symbol Unmangled symbol name
 * @return Memory address (**Note: this is sensitive because it would allow an attacker to bypass
 * ASLR if leaked!**)
 */
unsigned long getAddress(pid_t pid, std::vector<MemoryMapEntry> entries, std::string symbol)
{
  //Get the executable path
  std::string path = getPath(pid);

  //Get the symbol address
  unsigned long symbolAddress = getSymbolAddress(path, symbol);

  //Get the base address
  unsigned long baseAddress = 0;
  for (MemoryMapEntry entry : entries)
  {
#ifdef DEBUG
    std::cout << "[Proxy] Memory mapping entry - address: " << std::hex << entry.addressStart << " - " << entry.addressEnd << " (Offset: " << entry.addressOffset << "), permissions: " << entry.permissions << ", device: " << major(entry.device) << ":" << minor(entry.device) << ", inode: " << std::dec << entry.inode << ", path: " << entry.path << std::endl;
#endif //DEBUG

    //Check if the paths match
    if (entry.path == path)
    {
      baseAddress = entry.addressStart;
      break;
    }
  }

  if (baseAddress == 0)
  {
    throw std::runtime_error("[Proxy] Failed to get base address!");
  }

  //Compute the full address
  unsigned long fullAddress = baseAddress + symbolAddress;

#ifdef DEBUG
    std::cout << "[Proxy] Base address: " << std::hex << baseAddress << ", symbol address: " << symbolAddress << ", full address: " << fullAddress << std::endl;
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

    //Switch to the target
    execl(target, target, NULL);
  }
  //Running in parent
  else
  {
    //Wait for the child to start
    int status;
    waitpid(child, &status, 0);

    //Get all memory map entries
    std::vector<MemoryMapEntry> entries = getMapEntries(child);

    //Get the address address
    unsigned long address = getAddress(child, entries, "add(int, int)");

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
    waitpid(child, &status, 0);

    if (WIFSTOPPED(status))
    {
#ifdef DEBUG
      std::cout << "[Proxy] Got signal: " << strsignal(WSTOPSIG(status)) << std::endl;
#endif //DEBUG
    }

    //Get the registers
    struct user_regs_struct registers;
    ptrace(PTRACE_GETREGS, child, 0, &registers);

    //Prepare to read the stack
    size_t bufferSize = 32;
    char buffer[bufferSize];
    struct iovec local[1];
    struct iovec remote[1];
    local[0].iov_base = buffer;
    local[0].iov_len = bufferSize;
    remote[0].iov_base = (void *)(registers.rsp);
    remote[0].iov_len = bufferSize;

    //Read the stack
    errno = 0;
    ssize_t read = process_vm_readv(child, local, 1, remote, 1, 0);

    if (read != bufferSize)
    {
      //Get the actual error
      int error = errno;

      throw std::runtime_error("[Proxy] Failed to read stack! Error: " + std::string(strerror(error)));
    }

    //Unpack the stack
    struct stack_s {
      unsigned long returnAddress;
      int a;
      int b;
    };
    stack_s* stack = reinterpret_cast<stack_s*>(buffer);

#ifdef DEBUG
    std::cout << "[Proxy] Return address: " << std::hex << stack->returnAddress << ", a: " << std::dec << stack->a << ", b: " << stack->b << std::endl;
#endif //DEBUG

    //Intercept
    if (true)
    {
      //Compute the sum
      int sum = stack->a + stack->b;

      //Alter the sum
      int altered = sum - 100;

      //Print
      std::cout << "[Proxy] " << std::dec << stack->a << " + " << stack->b << " = " << sum << std::endl;
      std::cout << "[Proxy] Returning " << std::dec << altered << " instead" << std::endl;

      //Store the sum to the accumulator
#ifdef __x86_64__
      registers.rax = altered;
#else
      registers.eax = altered;
#endif

      //Set the instruction pointer to the return address (To effectively skip the function execution)
#ifdef __x86_64__
      registers.rip = stack->returnAddress;
#else
      registers.eip = stack->returnAddress;
#endif

      //Increase the stack pointer (To effectively pop the return address which we already have)
#ifdef __x86_64__
      registers.rsp += 8;
#else
      registers.esp += 8;
#endif
    }
    //Pass-through
    else
    {
      //Back the instruction pointer up one (To before the breakpoint)
#ifdef __x86_64__
      registers.rip--;
#else
      registers.eip--;
#endif
    }

    //Restore the instruction
    ptrace(PTRACE_POKETEXT, child, address, original);

    //Update the registers
    ptrace(PTRACE_SETREGS, child, 0, &registers);

    //Continue child execution
    ptrace(PTRACE_CONT, child, 0, 0);

    //Wait for the child to finish
    waitpid(child, &status, 0);

    if (WIFEXITED(status))
    {
#ifdef DEBUG
      std::cout << "[Proxy] Child process exited" << std::endl;
#endif //DEBUG
    }
  }

  return 0;
};