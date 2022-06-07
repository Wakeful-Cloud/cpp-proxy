//Imports
#include "simple_math.hpp"

template <typename T> T add(T a, T b)
{
  return a + b;
}

//Template instantiation
template int add(int, int);
template double add(double, double);