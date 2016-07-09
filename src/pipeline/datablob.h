#pragma once

#include <Eigen/Dense>
#include <pipeline/output_module_interface.h>
#include "../server/packet.h"

class DataBlob
{
public:
  DataBlob();
  Datum Serialize();
  void Reset(Datum &datum);
  std::string Name();

  output_header_t *mHdr;
  Datum *mDatum;
  Eigen::MatrixXcf mACM;
  Eigen::MatrixXf mMask;
  std::vector<int> mFlagged;
};

