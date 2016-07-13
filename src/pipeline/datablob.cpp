#include <glog/logging.h>
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
  char buf[256];
  int fdips = mHdr->flagged_dipoles.count();
  int fchans = mHdr->flagged_channels.count();
  std::snprintf(buf, 256, "%i %0.1f %s %i %i %s",
                mHdr->subband,
                mHdr->end_time,
                mHdr->polarization ? "YY" : "XX",
                fdips,
                fchans,
                mHdr->ateam.to_string().c_str());
  return buf;
}

float DataBlob::CentralFrequency()
{
  return utils::Subband2Frequency(mHdr->subband);
}

double DataBlob::CentralTime()
{
  return utils::UnixTime2MJD(0.5*(mHdr->start_time+mHdr->end_time)) / 86400.0 + 2400000.5;
}

bool DataBlob::IsValid()
{
  return mHdr->flagged_dipoles.count() < NUM_ANTENNAS*0.2;
}