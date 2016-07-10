#include "flagger.h"
#include "../../config.h"
#include "../datablob.h"

#include <glog/logging.h>

DECLARE_double(antsigma);
DECLARE_double(vissigma);

std::string Flagger::Name()
{
  return "Flagger";
}

void Flagger::Initialize()
{
  mAntennas.resize(NUM_ANTENNAS);
  mAntTmp.resize(NUM_ANTENNAS);
  mAntSigma = FLAGS_antsigma;
  mVisSigma = FLAGS_vissigma;
}

void Flagger::Run(DataBlob &b)
{
  using namespace std;

  const int N = NUM_BASELINES;
  const int M = NUM_CHANNELS;
  const int HALF_M = M / 2;

  mAbs.resize(M, N);
  mTmp.resize(M, N);
  mMask.resize(M, N);
  mStd.resize(N);
  mCentroid.resize(N);
  mMean.resize(N);
  mMinVal.resize(N);
  mMaxVal.resize(N);
  mResult.resize(N);

  Eigen::Map<Eigen::MatrixXcf, Eigen::Aligned>
    raw(reinterpret_cast<std::complex<float>*>(b.mDatum->data()+sizeof(output_header_t)),
         NUM_CHANNELS,
         NUM_BASELINES);

  // Compute statistics only when we have enough channels
  if (M >= 3)
  {
    // Compute power of visibilities
    mAbs = raw.array().abs();
    mTmp = mAbs;

    // computes the exact median
    if (M & 1)
    {
      for (int i = 0; i < N; i++)
      {
        vector<float> row(mTmp.data() + i * M, mTmp.data() + (i + 1) * M);
        nth_element(row.begin(), row.begin() + HALF_M, row.end());
        mCentroid(i) = row[HALF_M];
      }
    }
      // nth_element guarantees x_0,...,x_{n-1} < x_n
    else
    {
      for (int i = 0; i < N; i++)
      {
        vector<float> row(mTmp.data() + i * M, mTmp.data() + (i + 1) * M);
        nth_element(row.begin(), row.begin() + HALF_M, row.end());
        mCentroid(i) = row[HALF_M];
        mCentroid(i) += *max_element(row.begin(), row.begin() + HALF_M);
        mCentroid(i) *= 0.5f;
      }
    }

    // compute the mean
    mMean = mAbs.colwise().mean();

    // compute std (x) = sqrt ( 1/M SUM_i (x(i) - mean(x))^2 )
    mStd =
      (((mAbs.rowwise() - mMean.transpose()).array().square()).colwise().sum() *
       (1.0f / M))
        .array()
        .sqrt();


    // compute n sigmas from centroid
    mStd *= mVisSigma;
    mMinVal = mCentroid - mStd;
    mMaxVal = mCentroid + mStd;

    // compute clip mask
    for (int i = 0; i < N; i++)
    {
      mMask.col(i) =
        (mAbs.col(i).array() > mMinVal(i)).select(Eigen::VectorXf::Ones(M), 0.0f);
      mMask.col(i) =
        (mAbs.col(i).array() < mMaxVal(i)).select(Eigen::VectorXf::Ones(M), 0.0f);
    }

    // apply clip mask to data
    raw.array() *= mMask.array();

    // compute mean such that we ignore clipped data, this is our final result
    mResult = raw.colwise().sum().array() / mMask.colwise().sum().array();
  }
  else
  {
    mResult = raw.colwise().mean();
  }

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

  // Create "dipoles" by computing the absolute mean of the visibilities
  mAntennas = b.mACM.array().abs().colwise().mean();
  for (int a = 0; a < mAntennas.size(); a++)
  {
    if (mAntennas(a) <= 1e-5f &&
        std::find(b.mFlagged.begin(), b.mFlagged.end(), a) == b.mFlagged.end())
    {
      b.mMask.col(a).setOnes();
      b.mMask.row(a).setOnes();
      b.mFlagged.push_back(a);
    }
  }

  // Sigma clip the remaining dipoles
  mAntTmp = mAntennas;
  float mean = mAntennas.mean();
  float std = sqrtf((1.0f / mAntennas.size()) *
                    (mAntennas.array() - mean).array().square().sum());
  float centroid;
  vector<float> ant(mAntTmp.data(), mAntTmp.data()+mAntTmp.size());
  nth_element(ant.begin(), ant.begin() + ant.size()/2, ant.end());
  centroid = ant[ant.size()/2];
  centroid += *max_element(ant.begin(), ant.begin() + ant.size()/2);
  centroid *= 0.5f;

  // Now we can determine bad antennas
  std *= mAntSigma;
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    if (mAntennas(a) < (centroid - std) || mAntennas(a) > (centroid + std))
    {
      b.mMask.col(a).setOnes();
      b.mMask.row(a).setOnes();
      b.mFlagged.push_back(a);
    }
  }

  for (auto i : b.mFlagged)
    b.mHdr->flagged_dipoles[i] = true;
}
