//Imports
#include "simple_math.hpp"
#include <execinfo.h>
#include <iostream>
#include <string>

//Original signature
template <typename T>
using original_add_t = T (T a, T b);

//Proxy function
template <typename T> T add(T a, T b)
{
  //Get the backtrace
  void *backtraceEntries[1];
  std::size_t backtraceSize = backtrace(backtraceEntries, 1024);
  char **rawBacktrace = backtrace_symbols(backtraceEntries, backtraceSize);

  //Ensure the backtrace isn't null
  if (rawBacktrace == NULL)
  {
    throw std::runtime_error("[Proxy] Backtrace is null!");
  }

  //Get the last backtrace entry
  std::string last = std::string(rawBacktrace[0]);

  //Free the raw backtrace
  free(rawBacktrace);

  //Parse the last backtrace entry
  std::size_t openingParanthesis = last.find_last_of('(');
  std::size_t offsetPlus = last.find_last_of('+');

  //Get the mangled function name
  std::string mangledName = last.substr(openingParanthesis + 1, offsetPlus - openingParanthesis - 1);

  //Clear existing errors
  dlerror();

  //Get the original function
  original_add_t<T>* originalAdd = *(original_add_t<T>*)dlsym(RTLD_NEXT, mangledName.c_str());

  //Handle error
  char* error = dlerror();
  if (error != NULL)
  {
    throw std::runtime_error("[Proxy] " + std::string(error));
  }

  //Call original function
  T sum = originalAdd(a, b);

  //Alter sum
  T altered = sum - 100;

  //Print state
  std::cout << "[Proxy] " << a << " + " << b << " = " << sum << std::endl;
  std::cout << "[Proxy] Returning " << altered << " instead" << std::endl;

  return altered;
}

//Proxy template instantiation
template int add(int, int);
template double add(double, double);