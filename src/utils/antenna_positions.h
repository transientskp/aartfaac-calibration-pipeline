#pragma once

#include <Eigen/Dense>
#include <string>

#include "../config.h"

using namespace Eigen;

class AntennaPositions
{
public:
  static AntennaPositions *CreateInstance(const std::string &filename);
  static AntennaPositions *Instance() { return sInstance; }

  Vector3d GetPosLocal(const int a);
  Vector3d GetITRF(const int a);
  Vector3d GetUVW(const int a1, const int a2);

  MatrixXd& GetAllU() { return mUCoords; }
  MatrixXd& GetAllV() { return mVCoords; }
  MatrixXd& GetAllW() { return mWCoords; }

  MatrixXd& GetAllITRF() { return mPosItrf; }
  MatrixXd& GetAllLocal() { return mPosLocal; }

private:
  AntennaPositions(const std::string &filename);
  ~AntennaPositions() {}

  MatrixXd mPosItrf;  ///< ITRF positions (a1,...,an)
  MatrixXd mPosLocal; ///< Local antenna positions (ITRF * R)
  MatrixXd mUCoords;  ///< a1_x - a2_x;
  MatrixXd mVCoords;  ///< a1_y - a2_y;
  MatrixXd mWCoords;  ///< a1_z - a2_z;

  static AntennaPositions *sInstance;
};

#define ANT_UVW(a, b) AntennaPositions::Instance()->GetUVW(a, b)
#define ANT_U() AntennaPositions::Instance()->GetAllU()
#define ANT_V() AntennaPositions::Instance()->GetAllV()
#define ANT_W() AntennaPositions::Instance()->GetAllW()
#define ANT_ITRF() AntennaPositions::Instance()->GetAllITRF()
#define ANT_LOCAL() AntennaPositions::Instance()->GetAllLocal()
#define ANT_XYZ(a) AntennaPositions::Instance()->GetPosLocal(a)
