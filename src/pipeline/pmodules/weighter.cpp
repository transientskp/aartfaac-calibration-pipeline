#include <glog/logging.h>
#include "weighter.h"
#include "../../config.h"

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

  for (int i = 0; i < n; i++)
  {
    float w = b.mHdr->weights[i] / float(max_num);
    raw.block(i*48, 0, i*48+47, NUM_BASELINES) *= w;
  }
}