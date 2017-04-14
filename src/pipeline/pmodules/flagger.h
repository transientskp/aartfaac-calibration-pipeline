#pragma once

#include <pipeline/processing_module_interface.h>
#include <Eigen/Dense>

#include "../datablob.h"

class Flagger : public ProcessingModuleInterface<DataBlob>
{
public:
  Flagger(){}

  virtual std::string Name();
  virtual void Initialize(DataBlob &blob);
  virtual void Run(DataBlob &blob);

private:
  DataBlob *mBlob;
  float mAntSigma;
  float mVisSigma;
  Eigen::VectorXf mChannels;
  Eigen::VectorXf mChannelMask;
  Eigen::VectorXf mAntMask;
  Eigen::MatrixXf mVisMask;
  Eigen::VectorXf mAntennas;
  Eigen::VectorXcf mResult;
};
