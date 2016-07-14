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
  Datum d(mACM.size() * sizeof(std::complex<float>) + sizeof(output_header_t));
  memcpy(d.data(), mHdr, sizeof(output_header_t));
  memcpy(d.data()+sizeof(output_header_t), mACM.data(), mACM.size() * sizeof(std::complex<float>));
  return d;
}

std::string DataBlob::Name()
{
  std::stringstream ss;
  ss << mHdr->subband << " " << std::fixed << std::setprecision(1) << CentralFrequency();
  ss << " " << std::setprecision(2) << CentralTimeMJD();
  ss << " " << (mHdr->polarization ? "YY" : "XX");

  if (IsValid())
  {
    int fdips = mHdr->flagged_dipoles.count();
    int fchans = mHdr->flagged_channels.count();
    ss << " " << fdips << " " << fchans << mHdr->ateam;
  }

  return ss.str();
}

float DataBlob::CentralFrequency()
{
  return utils::Subband2Frequency(mHdr->subband);
}

double DataBlob::CentralTimeMJD()
{
  return utils::UnixTime2MJD(0.5*(mHdr->start_time+mHdr->end_time));
}

bool DataBlob::IsValid()
{
  return mHdr->flagged_dipoles.count() < NUM_ANTENNAS*0.2;
}