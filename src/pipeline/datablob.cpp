#include <glog/logging.h>
#include "datablob.h"

#include "../config.h"
#include "../utils/utils.h"

DataBlob::DataBlob()
{
  mData.resize(NUM_BASELINES*NUM_CHANNELS*sizeof(std::complex<float>) + sizeof(output_header_t), 0);
  mHdr = reinterpret_cast<output_header_t*>(mData.data());
}

void DataBlob::Swap(DataVector &data)
{
  CHECK(data.size() == mData.size());
  std::swap(data, mData);
  mHdr = reinterpret_cast<output_header_t*>(mData.data());
}

void DataBlob::Serialise(std::ostream &stream)
{

}

void DataBlob::Deserialise(std::ostream &stream)
{

}

std::string DataBlob::Name()
{
  char buf[256];
  int fdips = 0;
  for (int i = 0; i < 9; i++)
    fdips += utils::PopCount(mHdr->flagged_dipoles[i]);
  int fchans = utils::PopCount(mHdr->flagged_channels);
  std::snprintf(buf, 256, "%i %0.1f %s %i %i", mHdr->subband, mHdr->end_time, mHdr->polarization ? "YY" : "XX", fdips, fchans);
  return buf;
}