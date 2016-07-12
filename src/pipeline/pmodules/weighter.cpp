#include <glog/logging.h>
#include "weighter.h"
#include "../../config.h"

int32_t index();

std::string Weighter::Name()
{
  return "Weighter";
}

void Weighter::Initialize()
{
}

void Weighter::Run(DataBlob &b)
{
  constexpr int n = NUM_ANTENNAS == 288 ? (6*7/2) : (12*13/2);
  uint32_t max_num = 0;
  for (int i = 0; i < n; i++)
    max_num = std::max(b.mHdr->weights[i], max_num);

  // Header doesn't support weights yet
  if (max_num == 0)
  {
    VLOG(3) << "Incomming header does not support weights - skipping";
    return;
  }

  Eigen::Map<Eigen::MatrixXcf, Eigen::Aligned>
      raw(reinterpret_cast<std::complex<float>*>(b.mDatum->data()+sizeof(output_header_t)),
          NUM_CHANNELS,
          NUM_BASELINES);

  int s0, s1;
  float w;
  for (int a0 = 0; a0 < NUM_ANTENNAS; a0++)
  {
    s0 = a0/NUM_ANTENNAS_PER_STATION;
    for (int a1 = 0; a1 <= a0; a1++)
    {
      s1 = a1/NUM_ANTENNAS_PER_STATION;
      w = b.mHdr->weights[Index(s0, s1)] / float(max_num);
      raw.col(Index(a0, a1)) *= w;
    }
  }
}

int32_t Weighter::Index(int i, int j)
{
  return i*(i+1)/2 + j;
}