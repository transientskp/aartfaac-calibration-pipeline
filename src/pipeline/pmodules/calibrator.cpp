#include <glog/logging.h>
#include "calibrator.h"

#include "../../utils/antenna_positions.h"
#include "../../utils/NMSMax.h"

#define MAX_MAJOR_CYCLES 10
#define MAX_MINOR_CYCLES 30
#define C_MS 299792458.0f

std::string Calibrator::Name()
{
  return "Calibrator";
}

void Calibrator::Initialize()
{
  mAntennaLocalPosReshaped = ANT_ITRF();

  mMask.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mSpatialFilterMask.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mNoiseCovMatrix.resize(NUM_ANTENNAS, NUM_ANTENNAS);

  MatrixXd u = ANT_U(), v = ANT_V(), w = ANT_W();
  mUVDist = (u.array().square() + v.array().square() + w.array().square()).sqrt();

  mRaSources.resize(5);
  mDecSources.resize(5);
  mFluxes.resize(5);

  // Cassiopeia A (11000 flux)
  mRaSources(0)   = 6.113786558863104;
  mDecSources(0)  = 1.021919365863547;

  // Cygnus A (8100 flux)
  mRaSources(1)   = 5.226192095556169;
  mDecSources(1)  = 0.7086036763096979;

  // Tauras A (1420 flux)
  mRaSources(2)   = 1.446441617590301;
  mDecSources(2)  = 0.3835070143048873;

  // Virgo A (970 flux)
  mRaSources(3)   = 3.265074698168392;
  mDecSources(3)  = 0.221104127406815;

  mEpoch.resize(5);
  mEpoch.setOnes();
  mEpoch(4) = 0;

  mNormalizedData.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mMajorCycleResidue = mMinorCycleResidue = 0.0f;
}

void Calibrator::Run(DataBlob &blob)
{
  static const double min_restriction = 10.0;                 ///< avoid vis. below this wavelength
  static const double max_restriction = 350.0;                ///< avoid vis. above this much meters
  static const Vector3d normal(0.598753, 0.072099, 0.797682); ///< Normal to CS002 (central antenna)

  if (!blob.IsValid())
    return;

  mHasConverged = true;
  mFrequency = blob.CentralFrequency();
  double uvdist_cutoff = std::min(min_restriction*(C_MS/mFrequency), max_restriction);

  // =====================================
  // ==== 0. Prepare/Reshape matrices ====
  // =====================================
  int num_antennas = NUM_ANTENNAS - blob.mHdr->flagged_dipoles.count();
  mNormalizedData.resize(num_antennas, num_antennas);
  mSpatialFilterMask.resize(num_antennas, num_antennas);
  mMask.resize(num_antennas, num_antennas);
  mNoiseCovMatrix.resize(num_antennas, num_antennas);
  mAntennaLocalPosReshaped.resize(num_antennas, 3);
  mGains.resize(num_antennas);

  std::vector<int> I(num_antennas);
  for (int i = 0, j = 0; i < NUM_ANTENNAS; i++)
    if (!blob.mHdr->flagged_dipoles[i])
      I[j++] = i;

  for (int i = 0; i < num_antennas; i++)
  {
    mAntennaLocalPosReshaped.row(i) = ANT_ITRF().row(I[i]);
    for (int j = 0; j < num_antennas; j++)
    {
      mNormalizedData(i, j) = blob.mACM(I[i], I[j]);
      mSpatialFilterMask(i, j) = mUVDist(I[i], I[j]) < uvdist_cutoff ? 1.0f : 0.0f;
      mMask(i, j) = blob.mMask(I[i], I[j]);
    }
  }

  double time = blob.CentralTimeMJD() / 86400.0 + 2400000.5;
  utils::sunRaDec(time, mRaSources(4), mDecSources(4));

  // ========================================================================
  // ==== 1. Whitening of the array covariance matrix for DOA estimation ====
  // ========================================================================
  mNormalizedData.array() /= (mNormalizedData.diagonal() * mNormalizedData.diagonal().transpose()).array().sqrt();

  // ================================
  // ==== 2. Initial calibration ====
  // ================================
  MatrixXd src_pos(mRaSources.rows(), 3);
  utils::radec2itrf<double>(mRaSources, mDecSources, mEpoch, time, src_pos);
  VectorXd up = src_pos * normal;
  mSelection.resize((up.array() > 0.1).count(), 3);

  std::vector<int> src_indices;
  for (int i = 0, j = 0, n = src_pos.rows(); i < n; i++)
  {
    if (up(i) > 0.1)
    {
      mSelection.row(j) = src_pos.row(i);
      blob.mHdr->ateam[i] = true;
      src_indices.push_back(i);
      j++;
    }
  }
  statCal(mNormalizedData, mFrequency, mMask, mGains, mFluxes, mNoiseCovMatrix);
  if (!mHasConverged)
    return;

  // ====================================
  // ==== 3. WSF Position Estimation ====
  // ====================================
  MatrixXd selection((mFluxes.array() > mFluxes(0)*0.01).count(), 3);
  VectorXf fluxes(selection.rows());
  for (int i = 0, j = 0, n = mFluxes.rows(); i < n; i++)
  {
    if (mFluxes(i) > mFluxes(0)*0.01)
    {
      selection.row(j) = mSelection.row(i);
      fluxes(j) = mFluxes(i);
      j++;
    }
    else
      blob.mHdr->ateam[src_indices[i]] = false;
  }
  wsfSrcPos(mNormalizedData, mNoiseCovMatrix, mGains, mFrequency, selection);

  // ==============================
  // ==== 4. Final calibration ====
  // ==============================
  std::complex<double> i1(0.0, 1.0);
  i1 *= 2.0 * M_PI * mFrequency / C_MS;
  MatrixXcf A = (-i1 * (mAntennaLocalPosReshaped * selection.transpose())).array().exp().cast<std::complex<float>>();
  MatrixXf inv_mask = (mNoiseCovMatrix.array().abs() > 0.0).select(MatrixXf::Zero(mNoiseCovMatrix.rows(), mNoiseCovMatrix.cols()), 1.0f);
  walsCalibration(A, mNormalizedData, fluxes, inv_mask, mGains, mFluxes, mNoiseCovMatrix);
  mGains = (1.0/mGains.array());
  mGains.adjointInPlace();
  mNormalizedData = (mGains.transpose().adjoint() * mGains.transpose()).array() * (mNormalizedData.array() - mNoiseCovMatrix.array()).array();

  // ===============================
  // ==== 5. A-team subtraction ====
  // ===============================
  MatrixXcf ATeam = A * mFluxes.asDiagonal() * A.adjoint();
  mNormalizedData.array() -= ATeam.array();

  // store fluxes
  CHECK(blob.mHdr->ateam.count() == mFluxes.size());
  for (int i = 0, j = 0; i < 5; i++)
  {
    if (blob.mHdr->ateam[i])
    {
      blob.mHdr->ateam_flux[i] = mFluxes(j);
      j++;
    }
  }

  // ================================================================
  // ==== 6. Reconstruct the full ACM from the reshaped matrices ====
  // ================================================================
  for (int i = 0; i < num_antennas; i++)
    for (int j = 0; j < num_antennas; j++)
      blob.mACM(I[i], I[j]) = mNormalizedData(i, j);
}

void Calibrator::statCal(const MatrixXcf &inData,
                         const double inFrequency,
                         MatrixXf &ioMask,
                         VectorXcf &outCalibrations,
                         VectorXf &outSigmas,
                         MatrixXcf &outVisibilities)
{
  std::complex<double> i1(0.0, 1.0);
  i1 *= 2.0 * M_PI * inFrequency / C_MS;
  MatrixXcf A = (-i1 * (mAntennaLocalPosReshaped * mSelection.transpose())).array().exp().cast<std::complex<float>>();

  MatrixXcf KA(A.rows()*A.rows(), A.cols());
  utils::khatrirao<std::complex<float>>(A.conjugate(), A, KA);

  MatrixXf AA = (A.adjoint() * A).array().abs().square();

  MatrixXf mask = 1.0f - (ioMask.array() > mSpatialFilterMask.array()).select(ioMask, mSpatialFilterMask).array();
  MatrixXcf data = inData.array() * mask.array();
  data.resize(inData.rows()*inData.cols(), 1);
  VectorXf flux = (AA.inverse() * KA.adjoint() * data).array().real();
  mMajorCycles = walsCalibration(A, inData, flux, mask, outCalibrations, outSigmas, outVisibilities);
  outCalibrations = (1.0f/outCalibrations.array()).conjugate();
}


int Calibrator::walsCalibration(const MatrixXcf &inModel,  					// A
                                const MatrixXcf &inData,   					// Rhat
                                const VectorXf  &inFluxes, 					// sigmas
                                const MatrixXf  &inInvMask,         // mask
                                VectorXcf &outGains,          // g
                                VectorXf  &outSourcePowers,   // sigmas
                                MatrixXcf &outNoiseCovMatrix) // Sigma_n
{
  static const float epsilon = 1e-3f;

  outNoiseCovMatrix.setZero();
  VectorXcf cur_gains(inData.rows());
  VectorXcf prev_gains(inData.rows());
  for (int i = 0; i < inData.rows(); i++)
    prev_gains(i) = std::complex<float>(0.0f, 1.0f);
  VectorXf cur_fluxes(inFluxes.rows());
  VectorXf prev_fluxes(inFluxes);

  int n = (inInvMask.array() > 0.5f).count();
  MatrixXcf rest(n, inFluxes.size());
  MatrixXcf data(n, 1);
  MatrixXcf pinv(inFluxes.size(), inFluxes.size());
  int i;
  for (i = 1; i <= MAX_MAJOR_CYCLES; i++)
  {
    // ======================================================
    // ==== 1. Per sensor gain estimation using gainSolv ====
    // ======================================================
    MatrixXcf M = inModel * prev_fluxes.asDiagonal() * inModel.adjoint();
    gainSolv(M.array() * inInvMask.array(), inData.array() * inInvMask.array(), prev_gains, cur_gains);

    float avg_gain = (cur_gains.array().abs()).mean();
    cur_gains.array() /= avg_gain;
    cur_gains.array() /= (cur_gains(0) / std::abs(cur_gains(0)));

    // =========================================
    // ==== 2. Model source flux estimation ====
    // =========================================
    MatrixXcf GA = cur_gains.asDiagonal() * inModel;
    M.resize(GA.rows()*GA.rows(), GA.cols());
    utils::khatrirao<std::complex<float> >(GA.conjugate(), GA, M);

    // Filter out masked data
    int j = 0, k = 0;
    while (j < inInvMask.size())
    {
      if (inInvMask(j) > 0.5f)
      {
        rest.row(k) = M.row(j);
        data(k) = inData(j);
        k++;
      }
      j++;
    }
    utils::pseudoInverse<std::complex<float> >(rest.adjoint()*rest, pinv);
    cur_fluxes = (pinv * rest.adjoint() * data).real();
    if ((cur_fluxes.array() == INFINITY).any())
      cur_fluxes = prev_fluxes;

    // ========================================
    // ==== 3. Noise covariance estimation ====
    // ========================================
    outNoiseCovMatrix = (inData - GA * cur_fluxes.asDiagonal() * GA.adjoint()).array() * (1.0f - inInvMask.array());

    // =================================
    // ==== 4. Test for convergence ====
    // =================================
    MatrixXcf prev_theta(prev_gains.size() + prev_fluxes.size(), 1);
    MatrixXcf cur_theta(cur_gains.size() + cur_fluxes.size(), 1);
    for (int j = 0; j < prev_gains.size(); j++)
    {
      prev_theta(j) = prev_gains(j);
      cur_theta(j) = cur_gains(j);
    }
    for (int j = 0; j < prev_fluxes.size(); j++)
    {
      prev_theta(j+prev_gains.size()) = std::complex<float>(prev_fluxes(j), 0.0f);
      cur_theta(j+prev_gains.size()) = std::complex<float>(cur_fluxes(j), 0.0f);
    }

    MatrixXcf pinv(prev_theta.rows(), prev_theta.cols());
    utils::pseudoInverse<std::complex<float> >(prev_theta, pinv);
    MatrixXcf X = pinv.transpose() * cur_theta;

    mMajorCycleResidue = std::abs(X(0) - 1.0f);
    if (mMajorCycleResidue < epsilon)
    {
//      qDebug("[%s] Convergence after %d iterations", __FUNCTION__, i);
      break;
    }

    // ================================
    // ==== Prepare next iteration ====
    // ================================
    prev_gains = cur_gains;
    prev_fluxes = cur_fluxes;
  }

  outGains = cur_gains;
  outSourcePowers = cur_fluxes;

  if (i >= MAX_MAJOR_CYCLES)
  {
    mHasConverged = false;
//    qCritical("[%s] No convergence after %d iterations", __FUNCTION__, i);
  }

  return i;
}

int Calibrator::gainSolv(const MatrixXcf &inModel,
                         const MatrixXcf &inData,
                         const VectorXcf &inEstimatedGains,
                         VectorXcf &outGains)
{
  int n = inModel.rows();

  // allocate once
  MatrixXcf data_normalised(n, n);
  MatrixXcf data_calibrated(n, n);
  VectorXcf estimated_calibration(n);
  VectorXcf tmp(n);

  static const float epsilon = 1e-6f;

  for (int i = 0; i < n; i++)
  {
    // Normalize the data
    data_normalised.col(i) = inData.col(i) / inData.col(i).dot(inData.col(i));

    // Initial calibration
    data_calibrated.col(i) = inEstimatedGains.array() * inModel.col(i).array();
  }

  int i;
  for (i = 1; i <= MAX_MINOR_CYCLES; i++)
  {
    for (int j = 0; j < n; j++)
    {
      estimated_calibration(j) = data_normalised.col(j).dot(data_calibrated.col(j));
      outGains(j) = 1.0f / std::conj(estimated_calibration(j));
    }

    if (i % 2 == 1)
      tmp = outGains;
    else
    {
      // Update gains and check for convergence
      estimated_calibration = outGains;
      outGains = (outGains.array() + tmp.array()) / 2.0f;
      float gains_normal = outGains.norm();
      tmp = outGains.array() - estimated_calibration.array();
      float delta_gains_normal = tmp.norm();
      mMinorCycleResidue = delta_gains_normal / gains_normal;
      if (mMinorCycleResidue <= epsilon)
      {
//        qDebug("[%s] Convergence after %d iterations", __FUNCTION__, i);
        break;
      }
    }

    // Update calibration of array covariance matrix
    for (int j = 0; j < n; j++)
      data_calibrated.col(j) = outGains.array() * inModel.col(j).array();
  }

//  if (i >= MAX_MINOR_CYCLES)
//    qCritical("[%s] No convergence after %d iterations", __FUNCTION__, i);

  return i;
}


void Calibrator::wsfSrcPos(const MatrixXcf &inData,
                           const MatrixXcf &inSigma1,
                           const VectorXcf &inGains,
                           const double inFreq,
                           MatrixXd &ioPositions)
{
  int nsrc = ioPositions.rows();
  VectorXd init(nsrc*2);
  for (int i = 0; i < nsrc; i++)
  {
    init(i) = atan2(ioPositions(i, 1), ioPositions(i, 0));
    init(i + nsrc) = asin(ioPositions(i, 2));
  }

  SelfAdjointEigenSolver<MatrixXcf> solver(inData);

  VectorXf eigenvalues_abs = solver.eigenvalues().array().abs();
  VectorXi I = NM::sort(eigenvalues_abs);

  MatrixXcf Es(solver.eigenvectors().rows(), nsrc);
  MatrixXf eigenmat_abs(nsrc, nsrc);
  eigenmat_abs.setZero();
  for (int i = 0; i < nsrc; i++)
  {
    eigenmat_abs(i,i) = eigenvalues_abs(i);
    Es.col(i) = solver.eigenvectors().col(I(i));
  }

  MatrixXf eye = MatrixXf::Identity(nsrc, nsrc);
  MatrixXf S = inSigma1.diagonal().array().real();
  MatrixXf A = (eigenmat_abs.array() - (S.mean() * eye).array());
  MatrixXf W = (A * A).array() / eigenmat_abs.array();
  W = (W.array() != W.array()).select(0,W);
  MatrixXcf EsWEs = Es * W * Es.adjoint();
  MatrixXcf T = 1.0f / inGains.array();
  MatrixXcf G = T.conjugate().asDiagonal().toDenseMatrix();

  WSFCost wsf_cost(EsWEs, G, inFreq, mAntennaLocalPosReshaped, nsrc);

  init = NM::Simplex(wsf_cost, init, mSimplexCycles, 1e-3, 1000);

  ioPositions.col(0) = init.head(nsrc).array().cos() * init.tail(nsrc).array().cos();
  ioPositions.col(1) = init.head(nsrc).array().sin() * init.tail(nsrc).array().cos();
  ioPositions.col(2) = init.tail(nsrc).array().sin();
}

Calibrator::WSFCost::WSFCost(const MatrixXcf &inW, const MatrixXcf &inG, const double inFreq, const MatrixXd &inP, const int n):
  W(inW),
  G(inG),
  P(inP),
  freq(inFreq),
  nsrc(n),
  i1(0.0, 1.0)
{
  i1 *= 2.0 * M_PI * freq / C_MS;
  T.resize(P.rows(), nsrc);
  A.resize(G.rows(), nsrc);
  src_pos.resize(nsrc, 3);
  PAperp.resize(P.rows(), P.rows());
  Eye = MatrixXcf::Identity(P.rows(), P.rows());
}

float Calibrator::WSFCost::operator()(const VectorXd &theta)
{
  src_pos.col(0) = theta.head(nsrc).array().cos() * theta.tail(nsrc).array().cos();
  src_pos.col(1) = theta.head(nsrc).array().sin() * theta.tail(nsrc).array().cos();
  src_pos.col(2) = theta.tail(nsrc).array().sin();

  T = (-i1 * (P * src_pos.transpose())).array().exp().cast<std::complex<float> >();
  A = G * T;
  PAperp = Eye.array() - (A * (A.adjoint() * A).lu().solve(A.adjoint())).array();

  return (PAperp * W).trace().real();
}
