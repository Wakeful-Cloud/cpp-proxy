//Imports
#include "QBDI.h"
#include "QBDIPreload.h"
#include "elf.hpp"
#include "proc.hpp"
#include <iostream>
#include <unistd.h>

#ifdef VERBOSE
#include <sys/sysmacros.h>
#endif //VERBOSE

/**
 * @brief Get the address of the the symbol in the specified process
 *
 * @param pid Process ID
 * @param name Unmangled symbol name
 * @return Memory address (**Note: this is sensitive because it would allow an attacker to bypass
 * ASLR if leaked!**)
 */
static QBDI::rword getAddress(pid_t pid, std::string name)
{
  //Get the executable path
  std::string path = getPath(pid);

  //Get all symbols
  std::vector symbols = getSymbols(path);

  //Get the symbol address
  QBDI::rword symbolAddress = 0;
  for (Symbol symbol : symbols)
  {
    if (symbol.unmangledName == name)
    {
      symbolAddress = symbol.address;
      break;
    }
  }

  if (symbolAddress == 0)
  {
    throw std::runtime_error("[Proxy] Failed to get symbol address!");
  }

  //Get all memory map entries
  std::vector<QBDI::MemoryMap> entries = QBDI::getCurrentProcessMaps(true);

  //Get the base address
  QBDI::rword baseAddress = 0;
  for (QBDI::MemoryMap entry : entries)
  {
#ifdef VERBOSE
    std::cout << "[Proxy] Memory mapping entry - address: " << std::hex << entry.range.start() << " - " << entry.range.end() << ", permissions: " << entry.permission << ", path: " << entry.name << std::endl;
#endif //VERBOSE

    //Check if the paths match
    if (entry.name == path && (entry.permission & QBDI::PF_EXEC) == 0)
    {
      baseAddress = entry.range.start();
      break;
    }
  }

  if (baseAddress == 0)
  {
    throw std::runtime_error("[Proxy] Failed to get base address!");
  }

  //Compute the full address
  QBDI::rword fullAddress = baseAddress + symbolAddress;

#ifdef VERBOSE
  std::cout << "[Proxy] Base address: " << std::hex << baseAddress << ", symbol address: " << symbolAddress << ", full address: " << fullAddress << std::endl;
#endif //VERBOSE

  return fullAddress;
}

static QBDI::VMAction onTarget(QBDI::VMInstanceRef vm, QBDI::GPRState *gprState, QBDI::FPRState *fprState, void *data)
{
  //Unpack the stack
  struct stack_s {
    unsigned long returnAddress;
    int a;
    int b;
  };
  stack_s* stack = reinterpret_cast<stack_s*>(gprState->rsp);

  #ifdef VERBOSE
  std::cout << "[Proxy] Return address: " << std::hex << stack->returnAddress << ", a: " << std::dec << stack->a << ", b: " << stack->b << std::endl;
  #endif //VERBOSE

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
    gprState->rax = altered;
#else
    gprState->eax = altered;
#endif

    //Set the instruction pointer to the return address (To effectively skip the function execution)
#ifdef __x86_64__
    gprState->rip = stack->returnAddress;
#else
    gprState->eip = stack->returnAddress;
#endif

    //Increase the stack pointer (To effectively pop the return address which we already have)
#ifdef __x86_64__
    gprState->rsp += 8;
#else
    gprState->esp += 8;
#endif

    //Update the registers
    vm->setGPRState(gprState);

    return QBDI::BREAK_TO_VM;
  }
  //Pass-through
  else
  {
    //Print
    std::cout << "[Proxy] Passing-through." << std::endl;

    return QBDI::CONTINUE;
  }
}

extern "C"
{
  /**
   * @brief QBDIPreload constructor
   *
   */
  QBDIPRELOAD_INIT;

  /**
   * @brief QBDIPreload program entrypoint handler
   *
   * @param main Main function address
   * @return QBDIpreload state
   */
  int qbdipreload_on_start(void *main)
  {
    return QBDIPRELOAD_NOT_HANDLED;
  }

  /**
   * @brief QBDIPreload main function preload handler
   *
   * @param gprCtx Original General Purpose Registers (GPR) context
   * @param fpuCtx Original Floating Point Unit (FPU) context
   * @return QBDIpreload state
   */
  int qbdipreload_on_premain(void *gprCtx, void *fpuCtx)
  {
    return QBDIPRELOAD_NOT_HANDLED;
  }

  /**
   * @brief QBDIPreload main function hijacked handler
   *
   * @param argc Argument count
   * @param argv Argument values
   * @return QBDIpreload state
   */
  int qbdipreload_on_main(int argc, char *argv[])
  {
    return QBDIPRELOAD_NOT_HANDLED;
  }

  /**
   * @brief QBDIPreload main function ready-to-run handler
   *
   * @param vm QBDIPreload virtual machine
   * @param start Start address
   * @param stop Stop address
   * @return QBDIpreload state
   */
  int qbdipreload_on_run(QBDI::VMInstanceRef vm, QBDI::rword start, QBDI::rword stop)
  {
    //Get own pid
    pid_t pid = getpid();

    //Get the address
    QBDI::rword address = getAddress(pid, "add(int, int)");

    //Register callbacks
    vm->addCodeAddrCB((QBDI::rword)address, QBDI::PREINST, onTarget, NULL);

    //Run
    vm->run(start, stop);

    return QBDIPRELOAD_NO_ERROR;
  }

  /**
   * @brief QBDIPreload program exit handler handler
   *
   * @param status Exit status
   * @return QBDIpreload state
   */
  int qbdipreload_on_exit(int status)
  {
    return QBDIPRELOAD_NO_ERROR;
  }
}