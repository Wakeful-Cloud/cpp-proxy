//Imports
#include "simple_math.hpp"
#include <iostream>

int main()
{
  //Get input
  int a, b;
  std::cout << "[Program] Enter a: ";
  std::cin >> a;
  std::cout << "[Program] Enter b: ";
  std::cin >> b;

  //Do math
  int c = add(a, b);

  //Print output
  std::cout << "[Program] " << a << " + " << b << " = " << c << std::endl;

  return 0;
};