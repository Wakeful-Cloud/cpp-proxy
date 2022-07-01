//Imports
#include "Eigen/Core"
#include "QBDI.h"
#include "QBDIPreload.h"
#include "elf.hpp"
#include "func.hpp"
#include "proc.hpp"
#include <iostream>
#include <regex>
#include <unistd.h>

/**
 * @brief Matrix item type
 */
#define MATRIX_TYPE float

/**
 * @brief Eigen::internal::general_matrix_matrix_product::run partial frame at call-time
 */
struct stack_s {
  QBDI::rword returnAddress;
  long rhsStride;
  MATRIX_TYPE* _res;
  long resIncr;
  long resStride;
  float alpha;
  Eigen::internal::level3_blocking<float,float>& blocking;
  Eigen::internal::GemmParallelInfo<long>* info = 0;
};

/**
 * @brief Get the address of the the symbol in the specified process
 *
 * @param pid Process ID
 * @param filter Unmangled symbol filter
 * @return Memory address (**Note: this is sensitive because it would allow an attacker to bypass
 * ASLR if leaked!**)
 */
static QBDI::rword getAddress(pid_t pid, std::regex filter)
{
  //Get the executable path
  std::string path = getPath(pid);

  //Get all symbols
  std::vector symbols = getSymbols(path);

  //Get the symbol address
  QBDI::rword symbolAddress = 0;
  for (Symbol symbol : symbols)
  {
    if (std::regex_match(symbol.unmangledName, filter))
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
  //Read register-based arguments
  long rows = reinterpret_cast<long>(getRegisterArgument(gprState, fprState, 0, false));
  long cols = reinterpret_cast<long>(getRegisterArgument(gprState, fprState, 1, false));
  long depth = reinterpret_cast<long>(getRegisterArgument(gprState, fprState, 2, false));
  const MATRIX_TYPE *_lhs = reinterpret_cast<const MATRIX_TYPE *>(getRegisterArgument(gprState, fprState, 3, false));
  long lhsStride = reinterpret_cast<long>(getRegisterArgument(gprState, fprState, 4, false));
  const MATRIX_TYPE *_rhs = reinterpret_cast<const MATRIX_TYPE *>(getRegisterArgument(gprState, fprState, 5, false));

  //Read stack-based arguments
  stack_s* stack = reinterpret_cast<stack_s*>(gprState->rsp);

#ifdef VERBOSE
  std::cout << "[Proxy] Return address: " << std::hex << stack->returnAddress << ", rows: " << std::dec << rows << ", columns: " << cols << ", depth: " << depth << ", left-hand matrix address: " << std::hex << _lhs << ", left-hand matrix stride: " << std::dec << lhsStride << ", right-hand matrix address: " << std::hex << _rhs << ", right-hand matrix stride: " << std::dec << stack->rhsStride << ", result matrix address: " << std::hex << stack->_res << ", result matrix incr: " << std::dec << stack->resIncr << ", result matrix stride: " << stack->resStride << ", alpha: " << stack->alpha << std::endl;
#endif //VERBOSE

  //Intercept
  if (true)
  {
    //Print
    std::cout << "[Proxy] Size " << std::dec << rows << " rows, " << cols << " columns" << std::endl;

    //Store a different matrix
    for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
      {
        //Compute the item address
        QBDI::rword itemAddress = reinterpret_cast<QBDI::rword>(stack->_res) + (i * stack->resStride * sizeof(MATRIX_TYPE)) + (j * sizeof(MATRIX_TYPE));

        //Cast
        MATRIX_TYPE *item = reinterpret_cast<MATRIX_TYPE *>(itemAddress);

        //Update the item
        *item = 1.5 * i * j;
      }
    }

    //Store the sum to the accumulator
// #ifdef __x86_64__
//     gprState->rax = altered;
// #else
//     gprState->eax = altered;
// #endif

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
   * @param fpuCtx Original Floating Point Registers (FPR) context
   * @return QBDIpreload state
   */
  int qbdipreload_on_premain(void *gprCtx, void *fprCtx)
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

    //Get the target address
    QBDI::rword address = getAddress(pid, std::regex("^Eigen::internal::general_matrix_matrix_product<.*>::run\\(.*\\)$"));

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