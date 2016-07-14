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
  char buf[256];
  int fdips = mHdr->flagged_dipoles.count();
  int fchans = mHdr->flagged_channels.count();
  std::snprintf(buf, 256, "%i %0.2f %s %i %i %0.6f %0.6f %0.6f %0.6f %0.6f",
                mHdr->subband,
                CentralTimeMJD(),
                mHdr->polarization ? "YY" : "XX",
                fdips,
                fchans,
                mHdr->ateam_flux[0],
                mHdr->ateam_flux[1],
                mHdr->ateam_flux[2],
                mHdr->ateam_flux[3],
                mHdr->ateam_flux[4]
  );
  return buf;
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