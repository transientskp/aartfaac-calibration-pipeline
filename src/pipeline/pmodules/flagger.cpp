#include "flagger.h"
#include "../../config.h"
#include "../datablob.h"
#include "../../utils/sigmaclip.h"

#include <glog/logging.h>
#include <iomanip>
#include <sstream>

DECLARE_double(antsigma);
DECLARE_double(vissigma);

std::string Flagger::Name()
{
  std::stringstream ss;
  ss << "Flagger: ";
  ss << mBlob->mHdr->flagged_dipoles.count();
  ss << " " << mBlob->mHdr->flagged_channels.count();
  return ss.str();
}

void Flagger::Initialize()
{
  mAntennas.resize(NUM_ANTENNAS);
  mAntSigma = FLAGS_antsigma;
  mVisSigma = FLAGS_vissigma;
}

void Flagger::Run(DataBlob &b)
{
  mBlob = &b;
  using namespace std;

  const int N = NUM_BASELINES;
  const int M = NUM_CHANNELS;

  mVisMask = Eigen::MatrixXf::Zero(M, N);

  // map raw data to complex eigen matrix
  Eigen::Map<Eigen::MatrixXcf, Eigen::Aligned>
    raw(reinterpret_cast<std::complex<float>*>(b.mDatum->data()+sizeof(output_header_t)),
         NUM_CHANNELS,
         NUM_BASELINES);

  // collapse to channel vector
  mChannels = raw.rowwise().mean().array().abs();
  mChannelMask = Eigen::VectorXf::Ones(M);
  utils::SigmaClip(mChannels, mChannelMask, mVisSigma);

  // mask rows that contribute
  for (int i = 0; i < M; i++)
  {
    if (mChannelMask(i))
      mVisMask.row(i).setOnes();
    else
      b.mHdr->flagged_channels[i+1] = true;
  }

  // apply clip mask to data
  raw.array() *= mVisMask.array();

  // compute mean such that we ignore clipped data, this is our final result
  mResult = raw.colwise().sum().array() / mVisMask.colwise().sum().array();

  // construct acm from result
  for (int i = 0, s = 0; i < NUM_ANTENNAS; i++)
  {
    b.mACM.col(i).head(i + 1) = mResult.segment(s, i + 1).conjugate();
    b.mACM.row(i).head(i + 1) = mResult.segment(s, i + 1);
    b.mACM(i,i) = std::complex<float>(b.mACM(i,i).real(), 0.0f);
    s += i + 1;
  }

  // clear NaN values
  b.mACM = (b.mACM.array() != b.mACM.array()).select(complex<float>(0.0f, 0.0f), b.mACM);

  // Compute antenna power
  mAntennas = b.mACM.colwise().mean().array().abs();
  mAntMask = (mAntennas.array() < 1e-5f).select(Eigen::VectorXf::Zero(NUM_ANTENNAS), Eigen::VectorXf::Ones(NUM_ANTENNAS));
  utils::SigmaClip(mAntennas, mAntMask, mAntSigma);

  // Now we can determine bad antennas
  for (int i = 0; i < NUM_ANTENNAS; i++)
  {
    if (mAntMask(i))
      continue;

    b.mHdr->flagged_dipoles[i] = true;
    b.mMask.col(i).setOnes();
    b.mMask.row(i).setOnes();
    b.mACM.col(i).setZero();
    b.mACM.row(i).setZero();
  }
}
