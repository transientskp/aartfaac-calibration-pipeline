#pragma once

#include <algorithm>
#include <utility>
#include <iostream>
#include <complex>
#include <Eigen/Dense>

#define RAD2DEG(x) ((x)*180/M_PI)
#define DEG2RAD(x) ((x)/180*M_PI)

namespace utils
{
float Subband2Frequency(const int subband, const float clock=2e8f);
float Channel2Frequency(const int subband, const int channel, const float clock=2e8f);
float Range2Frequency(const int subband, const int start, const int end, const float clock=2e8f);

/// Computes the sign of a value (branchless)
template <typename T>
T Sign(const T val)
{
  return (T(0) < val) - (val < T(0));
}

/// Clamp, a function such that: v in [min, max]
template<typename T>
T Clamp(const T val, const T min, const T max)
{
  return std::max<T>(min, std::min<T>(max, val));
}

/// Round x to nearest multiple of n
int RoundTo(const float x, const int n);

/**
 * @brief
 * Prints an std::vector to stderr in the octave readable format
 */
template<typename Derived>
void vector2stderr(const std::vector<Derived> &V, const char *name, const int precision=30)
{
  if (V.size() == 0)
    return;

  const char *type = (
                         typeid(V[0]) == typeid(std::complex<float>) ||
                         typeid(V[0]) == typeid(std::complex<double>)
                     ) ? "complex matrix" : "matrix";

  std::cerr << "# name: " << name << std::endl;
  std::cerr << "# type: " << type << std::endl;
  std::cerr << "# rows: " << V.size() << std::endl;
  std::cerr << "# columns: 1" << std::endl;

  std::cerr.unsetf(std::ios::floatfield);
  std::cerr.precision(precision);
  for (int i = 0; i < V.size(); i++)
    std::cerr << V[i] << "\t";

  std::cerr << std::endl << std::endl;
}

/**
 * @brief
 * Prints a matrix to stderr in the octave readable format
 */
template<typename Derived>
void matrix2stderr(const Eigen::DenseBase<Derived> &M, const char *name, const int precision=30)
{
  if (M.size() == 0)
    return;

  const char *type = (
                         typeid(M(0)) == typeid(std::complex<float>) ||
                         typeid(M(0)) == typeid(std::complex<double>)
                     ) ? "complex matrix" : "matrix";

  std::cerr << "# name: " << name << std::endl;
  std::cerr << "# type: " << type << std::endl;
  std::cerr << "# rows: " << M.rows() << std::endl;
  std::cerr << "# columns: " << M.cols() << std::endl;

  std::cerr.unsetf(std::ios::floatfield);
  std::cerr.precision(precision);
  for (int i = 0; i < M.rows(); i++)
  {
    for (int j = 0; j < M.cols(); j++)
      std::cerr << M(i,j) << "\t";
    std::cerr << std::endl;
  }

  std::cerr << std::endl << std::endl;
}

int PopCount(uint64_t x);

std::vector<std::pair<int,int>> ParseChannels(const std::string &value);
} // namespace utils
