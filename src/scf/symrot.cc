//
// Author : Toru Shiozaki
// Date   : June 2009
//

#include <src/scf/symrot.h>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <src/rysint/carsphlist.h>
#include <src/rysint/macros.h>

using namespace std;

SymRotAbel::SymRotAbel(const vector<double>& xyz, const int lmax, const bool spherical) {
  assert(xyz.size() == 9);

  // real Abelian has only diagonal elements...
  const double movex = xyz[0]; 
  const double movey = xyz[4];
  const double movez = xyz[8];

  vector<double> p0(1, 1.0);
  primrot_.push_back(p0);

  // check if it works when higher than f

  for (int i = 1; i <= lmax; ++i) {
    const int dim = (i + 1) * (i + 2) / 2;
    double* pitmp = new double[dim * dim];
    fill(pitmp, pitmp + dim * dim, 0.0);

    int cnt = 0;
    for (int z = 0; z <= i; ++z) {
      for (int y = 0; y <= i - z; ++y) {
        const int x = i - y - z;
        if (x < 0) continue;
        double denom = 1.0;
        for (int p = 2; p <= x; ++p) denom *= p;
        for (int p = 2; p <= y; ++p) denom *= p;
        for (int p = 2; p <= z; ++p) denom *= p;
        const double factor = ::pow(movex, x) * ::pow(movey, y) * ::pow(movez, z) * denom;
        pitmp[cnt * dim + cnt] = factor;
        ++cnt;
      }
    }
    vector<double> pi;
    if (spherical && i > 1) {
      struct CarSphList carsph;
      const int dimsph = 2 * i + 1;
      double* pisph = new double[dimsph * dimsph];
      const int carsphindex = i * ANG_HRR_END + i;
      carsph.carsphfunc_call(carsphindex, 1, pitmp, pisph); 
      for (int j = 0; j != dimsph; ++j) {
        double norm = 0.0;
        for (int k = 0; k != dimsph; ++k)
          norm += pisph[k + j * dimsph] * pisph[k + j * dimsph];
        norm = 1.0 / ::sqrt(norm);
        for (int k = 0; k != dimsph; ++k)
          pisph[k + j * dimsph] *= norm;
      }
      pi.insert(pi.end(), pisph, pisph + dimsph * dimsph);
      delete[] pisph;
    } else {
      for (int j = 0; j != dim; ++j) {
        double norm = 0.0;
        for (int k = 0; k != dim; ++k)
          norm += pitmp[k + j * dim] * pitmp[k + j * dim];
        norm = 1.0 / ::sqrt(norm);
        for (int k = 0; k != dim; ++k)
          pitmp[k + j * dim] *= norm;
      }
      pi.insert(pi.end(), pitmp, pitmp + dim * dim);
    }
    delete[] pitmp;
    primrot_.push_back(pi);
  }

}


SymRotAbel::~SymRotAbel() {

}
