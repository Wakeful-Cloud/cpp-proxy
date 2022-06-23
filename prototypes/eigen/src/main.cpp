//Imports
#include <Eigen/Dense>
#include <iostream>

//general_matrix_matrix_product::run isn't called if this is 6 or less
#define MATRIX_SIZE 7

int main()
{
  //Generate random matrices
  Eigen::MatrixXf matrixA, matrixB;
  matrixA = Eigen::MatrixXf::Random(MATRIX_SIZE, MATRIX_SIZE);
  matrixB = Eigen::MatrixXf::Random(MATRIX_SIZE, MATRIX_SIZE);

  //Multiply
  Eigen::MatrixXf matrixC = matrixA * matrixB;

  //Print output
  std::cout << "[Program] Matrix A:" << std::endl << matrixA << std::endl;
  std::cout << "[Program] Matrix B:" << std::endl << matrixB << std::endl;
  std::cout << "[Program] Matrix C:" << std::endl << matrixC << std::endl;

  return 0;
};