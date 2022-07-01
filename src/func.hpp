//Imports
#include "QBDI.h"

/**
 * @brief Get the `argumentIndex`th register-based argument of the current function
 *
 * **Note: this must be invoked immediately after entering a function, prior to register or stack
 * modifications!**
 *
 * @param gprState General Purpose Registers (GPR) state
 * @param fprState Floating Point Registers (FPR) state
 * @param argumentIndex Argument index (0-based)
 * @param isFloat Whether or not the argument is a floating-point data type
 * @return Argument
 */
void *getRegisterArgument(QBDI::GPRState *gprState, QBDI::FPRState *fprState, int argumentIndex, bool isFloat);