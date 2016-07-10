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
  mFlagged.clear();
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
  std::snprintf(buf, 256, "%i %0.1f %s %i %i", mHdr->subband, mHdr->end_time, mHdr->polarization ? "YY" : "XX", fdips, fchans);
  return buf;
}