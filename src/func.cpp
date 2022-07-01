//Imports
#include "QBDI.h"
#include "func.hpp"
#include <stdexcept>

void *getRegisterArgument(QBDI::GPRState *gprState, QBDI::FPRState *fprState, int argumentIndex, bool isFloat)
{
#ifdef __x86_64__
  //Register-based, integer arguments
  if (!isFloat)
  {
    //Get the register value
    QBDI::rword reg;
    switch (argumentIndex)
    {
    case 0:
      reg = gprState->rdi;
      break;

    case 1:
      reg = gprState->rsi;
      break;

    case 2:
      reg = gprState->rdx;
      break;

    case 3:
      reg = gprState->rcx;
      break;

    case 4:
      reg = gprState->r8;
      break;

    case 5:
      reg = gprState->r9;
      break;

    default:
      throw std::logic_error("Failed to get the argument! (Unsupported argument index)");
    }

    //Cast
    return reinterpret_cast<void *>(reg);
  }
  //Float-based, integer arguments
  else
  {
    //Get the register value
    switch (argumentIndex)
    {
    case 0:
      return fprState->xmm0;
      break;

    case 1:
      return fprState->xmm1;
      break;

    case 2:
      return fprState->xmm2;
      break;

    case 3:
      return fprState->xmm3;
      break;

    case 4:
      return fprState->xmm4;
      break;

    case 5:
      return fprState->xmm5;
      break;

    case 6:
      return fprState->xmm6;
      break;

    case 7:
      return fprState->xmm7;
      break;

    default:
      throw std::logic_error("Failed to get the argument! (Unsupported argument index)");
    }
  }
#endif //__x86_64__

  throw std::logic_error("Failed to get the argument! (Not implemented)");
}