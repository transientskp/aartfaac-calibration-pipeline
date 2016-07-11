#pragma once

#include <pipeline/processing_module_interface.h>
#include <Eigen/Dense>

#include "../datablob.h"

using namespace Eigen;

class Calibrator : public ProcessingModuleInterface<DataBlob>
{
public:
  Calibrator() {}

  virtual std::string Name();
  virtual void Initialize();
  virtual void Run(DataBlob &blob);

private:
  void statCal(const MatrixXcf &inData,
               const double inFrequency,
               MatrixXf &ioMask,
               VectorXcf &outCalibrations,
               VectorXf &outSigmas,
               MatrixXcf &outVisibilities);

  int gainSolv(const MatrixXcf &inModel,
               const MatrixXcf &inData,
               const VectorXcf &inEstimatedGains,
               VectorXcf &outGains);

  int walsCalibration(const MatrixXcf &inModel,
                      const MatrixXcf &inData,
                      const VectorXf  &inFluxes,
                      const MatrixXf  &inInvMask,
                      VectorXcf &outGains,
                      VectorXf  &outSourcePowers,
                      MatrixXcf &outNoiseCovMatrix);

  void wsfSrcPos(const MatrixXcf &inData,
                 const MatrixXcf &inSigma1,
                 const VectorXcf &inGains,
                 const double inFreq,
                 MatrixXd &ioPositions);

  class WSFCost
  {
  public:
    WSFCost(const MatrixXcf &inW, const MatrixXcf &inG, const double inFreq, const MatrixXd &inP, const int n);

    float operator()(const VectorXd &theta);

  private:
    const MatrixXcf &W;
    const MatrixXcf &G;
    const MatrixXd &P;
    const double freq;
    const int nsrc;

    MatrixXd src_pos;
    MatrixXcf T;
    MatrixXcf A;
    MatrixXcf Eye;
    MatrixXcf PAperp;
    std::complex<double> i1;
  };

  /// Initialized in the constructor and const
  MatrixXd mUVDist;
  VectorXd mRaSources;
  VectorXd mDecSources;
  VectorXi mEpoch;

  /// Changed when new antenna are (un)flagged
  std::vector<int> mFlagged;
  MatrixXf mSpatialFilterMask;
  MatrixXd mAntennaLocalPosReshaped;
  MatrixXf mMask;
  MatrixXd mSelection;
  MatrixXcf mNormalizedData;
  MatrixXcf mNoiseCovMatrix;

  VectorXcf mGains;
  VectorXf mFluxes;
  double mFrequency;
  float mMajorCycleResidue;
  float mMinorCycleResidue;
  bool mHasConverged;
  int mMajorCycles;
  int mSimplexCycles;
};



