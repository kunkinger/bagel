//
// Author: Toru Shiozaki
// Machine Generated Code in NewInt
//

#include "vrrlist.h"

// returns double array of length 30
void VRRList::_vrr_1040(double* data_, const double* C00_, const double* D00_, const double* B00_, const double* B01_, const double* B10_) {
  data_[0] = 1.0;
  data_[1] = 1.0;
  data_[2] = 1.0;

  data_[3] = C00_[0];
  data_[4] = C00_[1];
  data_[5] = C00_[2];

  data_[6] = D00_[0];
  data_[7] = D00_[1];
  data_[8] = D00_[2];

  double cB00_current[3];
  cB00_current[0] = B00_[0];
  cB00_current[1] = B00_[1];
  cB00_current[2] = B00_[2];

  data_[9] = C00_[0] * data_[6] + cB00_current[0];
  data_[10] = C00_[1] * data_[7] + cB00_current[1];
  data_[11] = C00_[2] * data_[8] + cB00_current[2];

  double B01_current[3];
  B01_current[0] = B01_[0];
  B01_current[1] = B01_[1];
  B01_current[2] = B01_[2];

  data_[12] = D00_[0] * data_[6] + B01_current[0];
  data_[13] = D00_[1] * data_[7] + B01_current[1];
  data_[14] = D00_[2] * data_[8] + B01_current[2];

  cB00_current[0] += B00_[0];
  cB00_current[1] += B00_[1];
  cB00_current[2] += B00_[2];

  data_[15] = C00_[0] * data_[12] + cB00_current[0] * data_[6];
  data_[16] = C00_[1] * data_[13] + cB00_current[1] * data_[7];
  data_[17] = C00_[2] * data_[14] + cB00_current[2] * data_[8];

  B01_current[0] += B01_[0];
  B01_current[1] += B01_[1];
  B01_current[2] += B01_[2];

  data_[18] = D00_[0] * data_[12] + B01_current[0] * data_[6];
  data_[19] = D00_[1] * data_[13] + B01_current[1] * data_[7];
  data_[20] = D00_[2] * data_[14] + B01_current[2] * data_[8];

  cB00_current[0] += B00_[0];
  cB00_current[1] += B00_[1];
  cB00_current[2] += B00_[2];

  data_[21] = C00_[0] * data_[18] + cB00_current[0] * data_[12];
  data_[22] = C00_[1] * data_[19] + cB00_current[1] * data_[13];
  data_[23] = C00_[2] * data_[20] + cB00_current[2] * data_[14];

  B01_current[0] += B01_[0];
  B01_current[1] += B01_[1];
  B01_current[2] += B01_[2];

  data_[24] = D00_[0] * data_[18] + B01_current[0] * data_[12];
  data_[25] = D00_[1] * data_[19] + B01_current[1] * data_[13];
  data_[26] = D00_[2] * data_[20] + B01_current[2] * data_[14];

  cB00_current[0] += B00_[0];
  cB00_current[1] += B00_[1];
  cB00_current[2] += B00_[2];

  data_[27] = C00_[0] * data_[24] + cB00_current[0] * data_[18];
  data_[28] = C00_[1] * data_[25] + cB00_current[1] * data_[19];
  data_[29] = C00_[2] * data_[26] + cB00_current[2] * data_[20];
}

