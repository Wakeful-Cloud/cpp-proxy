//Imports
#include "simple_math.hpp"
#include <iostream>

int main()
{
  //Get input
  int a, b;
  std::cout << "Enter a: ";
  std::cin >> a;
  std::cout << "Enter b: ";
  std::cin >> b;

  //Do math
  int sum = add(a, b);

  //Print output
  std::cout << a << " + " << b << " = " << sum << std::endl;

  return 0;
};