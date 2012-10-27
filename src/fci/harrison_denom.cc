//
// BAGEL - Parallel electron correlation program.
// Filename: fci_init.cc
// Copyright (C) 2011 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and\/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The BAGEL package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the BAGEL package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <iomanip>
#include <stdexcept>
#include <bitset>
#include <src/fci/harrison.h>
#include <src/fci/mofile.h>
#include <src/rysint/eribatch.h>
#include <src/util/combination.hpp>
#include <src/util/constants.h>
#include <iostream>

using namespace std;
using namespace std::chrono;
using namespace bagel;

void HarrisonZarrabian::const_denom() {
  vector<double> jop, kop;
  jop.resize(norb_*norb_);
  kop.resize(norb_*norb_);
  for (int i = 0; i != norb_; ++i) {
    for (int j = 0; j <= i; ++j) {
      jop[i*norb_+j] = jop[j*norb_+i] = 0.5*jop_->mo2e_hz(j, i, j, i);
    }
  }
  for (int i = 0; i != norb_; ++i) {
    for (int j = 0; j <= i; ++j) {
      kop[i*norb_+j] = kop[j*norb_+i] = 0.5*jop_->mo2e_hz(j, i, i, j);
    }
  }

  shared_ptr<Civec> tmp(new Civec(det()));
  denom_ = tmp;
  const int nspin = det()->nspin(); 
  const int nspin2 = nspin*nspin;

  double* iter = denom_->data();
  for (auto ia = det()->stringa().begin(); ia != det()->stringa().end(); ++ia) {
    for (auto ib = det()->stringb().begin(); ib != det()->stringb().end(); ++ib, ++iter) {
      const int nopen = ((*ia)^(*ib)).count();
      const double F = (nopen >> 1) ? (static_cast<double>(nspin2 - nopen)/(nopen*(nopen-1))) : 0.0;
      *iter = 0.0;
      for (int i = 0; i != norb_; ++i) {
        const int nia = (*ia)[i];
        const int nib = (*ib)[i];
        const int niab = nia + nib;
        const int Ni = (nia ^ nib);
        for (int j = 0; j != i; ++j) {
          const int nja = (*ia)[j];
          const int njb = (*ib)[j];
          const int Nj = (nja ^ njb);
          const int addj = niab * (nja + njb); 
          *iter += jop[j+norb_*i] * 2.0 * addj - kop[j+norb_*i] * (F*Ni*Nj + addj);
        }
        *iter += jop_->mo1e(i,i) * niab - kop[i+norb_*i] * 0.5 * (Ni - niab*niab);
      }
    }
  }
}

void HarrisonZarrabian::update(shared_ptr<const Coeff> c) {
  // iiii file to be created (MO transformation).
  // now jop_->mo1e() and jop_->mo2e() contains one and two body part of Hamiltonian
  auto tp1 = high_resolution_clock::now();
  jop_ = shared_ptr<MOFile>(new Jop(ref_, ncore_, ncore_+norb_, c, string("HZ")));

  // right now full basis is used. 
  auto tp2 = high_resolution_clock::now();
  cout << "    * Integral transformation done. Elapsed time: " << setprecision(2) <<
          duration_cast<milliseconds>(tp2-tp1).count()*0.001 << endl << endl;

  const_denom();
}