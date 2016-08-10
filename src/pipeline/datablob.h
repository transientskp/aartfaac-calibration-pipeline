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
  float CentralFrequency();
  double CentralTimeMJD();
  double CentralTimeUnix();
  bool IsValid();

  output_header_t *mHdr;
  Datum *mDatum;
  Eigen::MatrixXcf mACM;
  Eigen::MatrixXf mMask;
};

