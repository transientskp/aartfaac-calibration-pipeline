#include <glog/logging.h>
#include <iomanip>
#include <sstream>
#include "datablob.h"

#include "../config.h"
#include "../utils/utils.h"

DataBlob::DataBlob()
{
  mACM.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mMask.resize(NUM_ANTENNAS, NUM_ANTENNAS);
}

void DataBlob::Reset(Datum &data)
{
  mDatum = &data;
  mHdr = reinterpret_cast<output_header_t*>(data.data());
  mACM.setZero();
  mMask.setIdentity();
  mHdr->flagged_dipoles.reset();
  mHdr->ateam.reset();
  for (int i = 0; i < 5; i++)
    mHdr->ateam_flux[i] = 0.0f;
}

Datum DataBlob::Serialize()
{
  Datum d(NUM_BASELINES * sizeof(std::complex<float>) + sizeof(output_header_t));
  memcpy(d.data(), mHdr, sizeof(output_header_t));

  std::complex<float> *c(reinterpret_cast<std::complex<float>*>(d.data()+sizeof(output_header_t)));

  for (int a1 = 0, i = 0; a1 < NUM_ANTENNAS; a1++)
    for (int a2 = 0; a2 < a1+1; a2++, i++)
      c[i] = mACM(a1, a2);

  return d;
}

inline double GetRealTime()
{
  timespec ts; 
  clock_gettime(CLOCK_REALTIME, &ts);
  return double(ts.tv_sec) + 1e-9 * double(ts.tv_nsec);
}


std::string DataBlob::Name()
{
  auto unix_timestamp = GetRealTime();
  std::stringstream ss;
  ss << mHdr->subband << " " << std::fixed << std::setprecision(1) << CentralFrequency();
  ss << " " << std::setprecision(2) << CentralTimeUnix();
  ss << " " << (mHdr->polarization ? "YY" : "XX");

  if (IsValid())
  {
    int fdips = mHdr->flagged_dipoles.count();
    int fchans = mHdr->flagged_channels.count();
    ss << " " << fdips << " " << fchans << " " << mHdr->ateam;
  }

  auto t = unix_timestamp - CentralTimeUnix();
  if (t < 300.0)
    ss << " lat " << t << " s";

  return ss.str();
}

float DataBlob::CentralFrequency()
{
  int start = 0, end = mHdr->num_channels-1;

  for (int i = 0; i < mHdr->num_channels; i++)
    if (!mHdr->flagged_channels[i])
    {
      start = i;
      break;
    }
  for (int i = mHdr->num_channels-1; i >= 0; i--)
    if (!mHdr->flagged_channels[i])
    {
      end = i;
      break;
    }
  return utils::Range2Frequency(mHdr->subband, start, end);
}

double DataBlob::CentralTimeMJD()
{
  return utils::UnixTime2MJD(CentralTimeUnix());
}

double DataBlob::CentralTimeUnix()
{
  return 0.5*(mHdr->start_time+mHdr->end_time);
}

bool DataBlob::IsValid()
{
  return mHdr->flagged_dipoles.count() < NUM_ANTENNAS*0.2;
}