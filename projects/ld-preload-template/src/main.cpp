//Imports
#include "simple_math.hpp"
#include <iostream>

int main()
{
  //Print section
  std::cout << "[Program] Integer section..." << std::endl;

  //Get input
  int a, b;
  std::cout << "[Program] Enter a: ";
  std::cin >> a;
  std::cout << "[Program] Enter b: ";
  std::cin >> b;

  //Do math
  int c = add<int>(a, b);

  //Print output
  std::cout << "[Program] " << a << " + " << b << " = " << c << std::endl;

  //Print section
  std::cout << "[Program] Double section..." << std::endl;

  //Get input
  double x, y;
  std::cout << "[Program] Enter x: ";
  std::cin >> x;
  std::cout << "[Program] Enter y: ";
  std::cin >> y;

  //Do math
  double z = add<double>(x, y);

  //Print output
  std::cout << "[Program] " << x << " + " << y << " = " << z << std::endl;

  return 0;
};