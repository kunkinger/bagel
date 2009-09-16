//
// Author : Toru Shiozaki
// Date   : May 2009
//

#include "carsphlist.h"
#include "f77.h"


void CarSphList::carsph_31(const int nloop, const double* source, double* target) {
  const double c3 = 3.872983346207417;
  const double c4 = 2.4494897427831779;
  const double c1 = 2.3717082451262845;
  const double c2 = 1.9364916731037085;
  const double c6 = 1.5;
  const double c0 = 0.79056941504209488;
  const double c5 = 0.61237243569579447;
  for (int iloop = 0; iloop != nloop; ++iloop, target += 21, source += 30) {
    target[0] =  c0 * source[0] - c1 * source[6];
    target[1] =  c0 * source[1] - c1 * source[7];
    target[2] =  c0 * source[2] - c1 * source[8];
    target[3] =  c1 * source[3] - c0 * source[9];
    target[4] =  c1 * source[4] - c0 * source[10];
    target[5] =  c1 * source[5] - c0 * source[11];
    target[6] =  c2 * source[12] - c2 * source[18];
    target[7] =  c2 * source[13] - c2 * source[19];
    target[8] =  c2 * source[14] - c2 * source[20];
    target[9] =  c3 * source[15];
    target[10] =  c3 * source[16];
    target[11] =  c3 * source[17];
    target[12] =  c4 * source[21] - c5 * source[0] - c5 * source[6];
    target[13] =  c4 * source[22] - c5 * source[1] - c5 * source[7];
    target[14] =  c4 * source[23] - c5 * source[2] - c5 * source[8];
    target[15] =  c4 * source[24] - c5 * source[3] - c5 * source[9];
    target[16] =  c4 * source[25] - c5 * source[4] - c5 * source[10];
    target[17] =  c4 * source[26] - c5 * source[5] - c5 * source[11];
    target[18] =  source[27] - c6 * source[12] - c6 * source[18];
    target[19] =  source[28] - c6 * source[13] - c6 * source[19];
    target[20] =  source[29] - c6 * source[14] - c6 * source[20];
  }
}

