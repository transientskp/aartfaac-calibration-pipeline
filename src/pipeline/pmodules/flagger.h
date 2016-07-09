#pragma once

#include <pipeline/processing_module_interface.h>
#include <Eigen/Dense>

#include "../datablob.h"

class Flagger : public ProcessingModuleInterface<DataBlob>
{
public:
  Flagger(){}

  virtual std::string Name();
  virtual void Initialize();
  virtual void Run(DataBlob &blob);

private:
  float mAntSigma;
  float mVisSigma;
  Eigen::MatrixXf mAbs;
  Eigen::MatrixXf mTmp;
  Eigen::MatrixXf mMask;
  Eigen::VectorXf mStd;
  Eigen::VectorXf mCentroid;
  Eigen::VectorXf mMean;
  Eigen::VectorXf mMinVal;
  Eigen::VectorXf mMaxVal;
  Eigen::VectorXf mAntennas;
  Eigen::VectorXf mAntTmp;
  Eigen::VectorXcf mResult;
};
