/**
 * Copyright (C) 2013 F. Huizinga
 *
 * Find a value of x which minimizes the function fun.  The search begins at
 * the point x0 and iterates using the Nelder & Mead Simplex algorithm (a
 * derivative-free method).  This algorithm is better-suited to functions which
 * have discontinuities or for which a gradient-based search fails.
 *
 * On exit, the function returns x, the minimum point
 *
 * NOTE: This code is a direct translation of nmsmax.m from Octave, created
 *       by N.J.Higham and Andy Adler.
 */

#include <algorithm>
#include <limits>
#include <vector>
#include <utility>
#include <string>

#include <Eigen/Dense>

namespace NM
{

using namespace Eigen;

enum How{INITIAL, REFLECT, EXPAND, CONTRACT, SHRINK, NUM_POS};
static std::vector<std::string> HowStr;

template<typename D>
bool Sorter(const std::pair<D, int> &a, const std::pair<D, int> &b)
{
  return a.first > b.first;
}

template<typename D>
VectorXi sort(Matrix<D, Dynamic, 1> &V)
{
  VectorXi I(V.size());
  std::vector<std::pair<D, int> > P(V.size());

  for (int i = 0; i < V.size(); i++)
    P[i] = std::make_pair<D, int>(V(i), i);

  std::sort(P.begin(), P.end(), Sorter<D>);

  for (int i = 0; i < V.size(); i++)
  {
    I(i) = P[i].second;
    V(i) = P[i].first;
  }

  return I;
}

template<typename D, class F>
Matrix<D, Dynamic, 1> Simplex(
        F &fun,                      // target function
        Matrix<D, Dynamic, 1> x,     // initial params
        int &iteration,              // iterations used
  const D tolerance = 1e-5,          // termination criteria
  const int iterations = 1e3         // max iterations
)
{
  typedef Matrix<D, Dynamic, Dynamic> MatX; // Matrix
  typedef Matrix<D, Dynamic, 1> VecX;       // Vector
  typedef Matrix<D, 2, 1> Vec2;             // Vector of size 2

  if (HowStr.empty())
  {
    HowStr.resize(NUM_POS);

    HowStr[0] = "initial";
    HowStr[1] = "reflect";
    HowStr[2] = "expand";
    HowStr[3] = "contract";
    HowStr[4] = "shrink";
  }

  static const D dirn = -1;
  static How how = INITIAL;
  int n = x.size();

  VecX x0 = x;
  MatX V(n, n+1);
  V.block(0,1,n,n).setIdentity();
  VecX f(n + 1);
  f(0) = dirn * fun(x);
  V.col(0) = x0;
#ifdef NM_TRACE
  D fmax_old = f(0);
#else // suppress warning on unused variable
  (void) how;
#endif

  // Setup initial regular shaped simplex
  D scale = std::max(x0.template lpNorm<Infinity>(), 1.0);
  Vec2 Alpha = scale / (n*sqrt(2)) * Vec2(sqrt(n+1.0)-1.0+n, sqrt(n+1.0)-1.0);
  V.block(0,1,n,n) = (x0 + Alpha(1)*VecX::Constant(n,1.0)) * VecX::Constant(n,1.0).transpose();

  for (int i = 1; i < n+1; i++)
  {
    V(i-1, i) = x0(i-1) + Alpha(0);
    f(i) = dirn * fun(V.col(i));
  }

  int nf = n+1;
  VectorXi I = sort(f);
  MatX T = V;
  for (int i = 0; i < I.size(); i++)
    V.col(i) = T.col(I(i));

  D alpha = 1.0, beta = 0.5, gamma = 2.0;

  iteration = 0;
  while (true)
  {
    iteration++;

    // Max iterations reached, break from the loop
    if (iteration > iterations)
      break;

    VecX v1 = V.col(0);
    MatX X = V.block(0,1,n,n).colwise() - v1;
    D size_simplex = X.template lpNorm<1>();
    size_simplex /= std::max(v1.template lpNorm<1>(), 1.0);

    // Test for convergence
    if (size_simplex <= tolerance)
      break;

    VecX vbar = (V.block(0,0,n,n).rowwise().sum().array() / n);
    VecX vr = (1.0 + alpha)*vbar.array() - alpha*V.col(n).array();
    x = vr;
    D fr = dirn * fun(x);
    nf++;
    VecX vk = vr;
    D fk = fr;
    how = REFLECT;
    if (fr > f(n-1))
    {
      if (fr > f(0))
      {
        VecX ve = gamma*vr.array() + (1.0-gamma)*vbar.array();
        x = ve;
        D fe = dirn * fun(x);
        nf++;
        if (fe > f(0))
        {
          vk = ve;
          fk = fe;
          how = EXPAND;
        }
      }
    }
    else
    {
      VecX vt = V.col(n);
      D ft = f(n);
      if (fr > ft)
      {
        vt = vr;
        ft = fr;
      }
      VecX vc = beta*vt.array() + (1.0-beta)*vbar.array();
      x = vc;
      D fc = dirn * fun(x);
      nf++;
      if (fc > f(n))
      {
        vk = vc;
        fk = fc;
        how = CONTRACT;
      }
      else
      {
        for (int i = 1; i < n; i++)
        {
          V.col(i) = (V.col(0) + V.col(i)).array() / 2.0;
          x = V.col(i);
          f(i) = dirn * fun(x);
        }
        nf += n-1;
        vk = (V.col(0) + V.col(n)).array() / 2.0;
        x = vk;
        fk = dirn * fun(x);
        nf++;
        how = SHRINK;
      }
    }

    V.col(n) = vk;
    f(n) = fk;
    VectorXi I = sort(f);
    T = V;
    for (int i = 0; i < I.size(); i++)
      V.col(i) = T.col(I(i));
  }

  return x;
}

} // namespace NM
