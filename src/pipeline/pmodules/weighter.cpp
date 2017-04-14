#include <glog/logging.h>
#include <sstream>
#include "weighter.h"
#include "../../config.h"

int32_t index();

std::string Weighter::Name()
{
  std::stringstream ss;
  ss << "Weighter: ";
  if (mMaxNum == 0)
    ss << "unsupported header";
  else
  {
    ss.precision(2);
    constexpr int n = NUM_ANTENNAS == 288 ? (6*7/2) : (12*13/2);
    for (int i = 0; i < n; i++)
      ss << std::fixed << mBlob->mHdr->weights[i]/float(mMaxNum) << " ";
  }

  return ss.str();
}

void Weighter::Initialize(DataBlob &blob)
{
  (void) blob;
}

void Weighter::Run(DataBlob &b)
{
  constexpr int n = NUM_ANTENNAS == 288 ? (6*7/2) : (12*13/2);
  mMaxNum = 0;
  for (int i = 0; i < n; i++)
    mMaxNum = std::max(b.mHdr->weights[i], mMaxNum);

  mBlob = &b;

  // Header doesn't support weights yet
  if (mMaxNum == 0)
    return;

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
      w = b.mHdr->weights[Index(s0, s1)] / float(mMaxNum);
      raw.col(Index(a0, a1)) *= w;
    }
  }
}

int32_t Weighter::Index(int i, int j)
{
  return i*(i+1)/2 + j;
}