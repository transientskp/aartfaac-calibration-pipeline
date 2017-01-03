#pragma once

#include <algorithm>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

namespace utils
{

template<typename I, typename P>
auto StablePartitionPosition(I f, I l, P p) -> I
{
  auto n = l - f;
  if (n == 0) return f;
  if (n == 1) return f + p(f);

  auto m = f + (n / 2);

  return rotate(StablePartitionPosition(f, m, p),
                m,
                StablePartitionPosition(m, l, p));
}


float Median(Ref<VectorXf> data)
{
  int n = data.rows();
  int h = n >> 1;

  nth_element(data.data(), data.data() + h, data.data() + n);
  float median = data(h);

  // if n is even the median is the mean of {v[h], v[h-1]}
  if (!(n & 1))
  {
    median += *max_element(data.data(), data.data() + h);
    median *= 0.5f;
  }

  return median;
}


float Sigma(Ref<VectorXf> data)
{
  return sqrt((data.array() - data.mean()).square().sum() * (1.0f / data.rows()));
}


template<int MaxIter = 100>
int SigmaClip(Ref<VectorXf> data, Ref<VectorXf> mask, float n = 2.0f)
{
  VectorXf copy(data);
  float median, sigma, sum;

  for (int i = 0; i < MaxIter; i++)
  {
    auto p = StablePartitionPosition(copy.data(), copy.data() + copy.rows(), [&](auto i)
    {
      return int(*(mask.data() + (i - copy.data())));
    });

    Map<VectorXf, Aligned> remains(copy.data(), int(p - copy.data()));

    median = Median(remains);
    sigma = Sigma(remains);
    sum = mask.sum();

    mask = (data.array() < median - n * sigma || data.array() > median + n * sigma).select(VectorXf::Zero(mask.rows()),
                                                                                           mask);

    if (sum == mask.sum())
      return i;

    copy = data;
  }

  return MaxIter;
}

} // namespace utils