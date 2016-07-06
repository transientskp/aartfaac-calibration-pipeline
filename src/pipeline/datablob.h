#pragma once

#include <pipeline/blob_interface.h>
#include "../server/packet.h"

class DataBlob : public BlobInterface
{
public:
  DataBlob();
  void Swap(DataVector &data);
  void Serialise(std::ostream &stream);
  void Deserialise(std::ostream &stream);
  std::string Name();

private:
  DataVector mData;
  output_header_t *mHdr;
};

