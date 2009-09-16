//
// Author: Toru Shiozaki
// Machine Generated Code
//

#include "svrrlist.h"

// returns double array of length 12
void SVRRList::_svrr_0030(double* data_, const double* C00_, const double* D00_, const double* B00_, const double* B01_, const double* B10_) {
  data_[0] = 1.0;
  data_[1] = 1.0;
  data_[2] = 1.0;

  data_[3] = D00_[0];
  data_[4] = D00_[1];
  data_[5] = D00_[2];

  double B01_current[3];
  B01_current[0] = B01_[0];
  B01_current[1] = B01_[1];
  B01_current[2] = B01_[2];

  data_[6] = D00_[0] * data_[3] + B01_current[0];
  data_[7] = D00_[1] * data_[4] + B01_current[1];
  data_[8] = D00_[2] * data_[5] + B01_current[2];

  B01_current[0] += B01_[0];
  B01_current[1] += B01_[1];
  B01_current[2] += B01_[2];

  data_[9] = D00_[0] * data_[6] + B01_current[0] * data_[3];
  data_[10] = D00_[1] * data_[7] + B01_current[1] * data_[4];
  data_[11] = D00_[2] * data_[8] + B01_current[2] * data_[5];
}

