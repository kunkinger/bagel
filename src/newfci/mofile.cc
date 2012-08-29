//
// BAGEL - Parallel electron correlation program.
// Filename: mofile.cc
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

//#define __NEWFCI_DEBUGGING


#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <src/util/f77.h>
#include <src/newfci/mofile.h>
#include <src/scf/scf.h>

using namespace std;
using namespace bagel;


NewMOFile::NewMOFile(const shared_ptr<const Reference> ref, const int nstart, const int nfence)
: geom_(ref->geom()), ref_(ref), core_fock_(new Matrix1e(geom_)), coeff_(ref_->coeff()) {

  do_df_ = geom_->df().get();
  if (!do_df_) throw runtime_error("for the time being I gave up maintaining non-DF codes.");

}


NewMOFile::NewMOFile(const shared_ptr<const Reference> ref, const int nstart, const int nfence, const shared_ptr<const Coeff> c)
: geom_(ref->geom()), ref_(ref), core_fock_(new Matrix1e(geom_)), coeff_(c) {

  do_df_ = geom_->df().get();
  if (!do_df_) throw runtime_error("for the time being I gave up maintaining non-DF codes.");

}


NewMOFile::~NewMOFile() {
}


double NewMOFile::create_Jiiii(const int nstart, const int nfence) {

  // first compute all the AO integrals in core
  nocc_ = nfence - nstart;
  nbasis_ = geom_->nbasis();
  const int nbasis = nbasis_;

  // one electron part
  double core_energy;
  unique_ptr<double[]> buf1e;
  tie(buf1e, core_energy) = compute_mo1e(nstart, nfence);

  // two electron part.
  // this fills mo2e_1ext_ and returns buf2e which is an ii/ii quantity
  unique_ptr<double[]> buf2e = compute_mo2e(nstart, nfence);

  compress(buf1e, buf2e);
  return core_energy;
}


void NewMOFile::compress(unique_ptr<double[]>& buf1e, unique_ptr<double[]>& buf2e) {

  // mo2e is compressed
  const int nocc = nocc_;
  #ifndef __NEWFCI_DEBUGGING
  sizeij_ = nocc*(nocc+1)/2;
  #else
  sizeij_ = nocc*nocc;
  #endif
  mo2e_ = unique_ptr<double[]>(new double[sizeij_*sizeij_]);

  #ifndef __NEWFCI_DEBUGGING
  int ijkl = 0;
  for (int i = 0; i != nocc; ++i) {
    for (int j = 0; j <= i; ++j) {
      const int ijo = (j + i*nocc)*nocc*nocc;
      for (int k = 0; k != nocc; ++k)
        for (int l = 0; l <= k; ++l, ++ijkl)
          mo2e_[ijkl] = buf2e[l+k*nocc+ijo];
    }
  }
  #else
  int iljk = 0;
  // In this case, there is no compression
  // Is currently ordered like (ij|kl), should be ordered like (il|jk), with the last index moving the fastest
  for (int i = 0; i != nocc; ++i) {
    for (int l = 0; l != nocc; ++l) {
      for (int j = 0; j != nocc; ++j) {
        for (int k = 0; k != nocc; ++k, ++iljk) {
          /* Yes, this is horribly inefficient use of the indices, but it's only for debugging, so meh! */
          mo2e_[iljk] = buf2e[l + k*nocc + j*nocc*nocc + i*nocc*nocc*nocc];
        }
      }
    }
  }
  #endif

  // h'kl = hkl - 0.5 sum_j (kj|jl)
  mo1e_ = unique_ptr<double[]>(new double[sizeij_]);
  int ij = 0;
  for (int i=0; i!=nocc; ++i) {
    for (int j=0; j<=i; ++j, ++ij) {
      mo1e_[ij] = buf1e[j+i*nocc];
      #ifndef __NEWFCI_DEBUGGING
      for (int k=0; k!=nocc; ++k)
        mo1e_[ij] -= 0.5*buf2e[(k+j*nocc)*nocc*nocc+(k+i*nocc)];
      #endif
    }
  }
}


void NewMOFile::update_1ext_ints(const vector<double>& coeff) {
  // in the case of no DF
  shared_ptr<DF_Half> buf = mo2e_1ext_->clone();

  // half transformed DF is rotated.
  const int naux = geom_->df()->naux();
  for (int i = 0; i != nbasis_; ++i)
    dgemm_("N", "N", naux, nocc_, nocc_, 1.0, mo2e_1ext_->data()+i*naux*nocc_, naux, &(coeff[0]), nocc_, 0.0, buf->data()+i*naux*nocc_, naux);
  mo2e_1ext_ = buf;

}


tuple<unique_ptr<double[]>, double> NewJop::compute_mo1e(const int nstart, const int nfence) {

  const int ncore = nstart;
  double core_energy = 0.0;

  unique_ptr<double[]> aobuff(new double[nbasis_*nbasis_]);
  shared_ptr<Fock<1> > fock0(new Fock<1>(geom_, ref_->hcore()));
  const double* cdata = coeff_->data() + nstart*nbasis_;
  // if core fock operator is not the same as hcore...
  if (nstart != 0) {
    shared_ptr<Matrix1e> den = coeff_->form_density_rhf(ncore);
    fock0 = shared_ptr<Fock<1> >(new Fock<1>(geom_, fock0, den, ref_->schwarz()));
    core_energy = (*den * (*ref_->hcore()+*fock0)).trace() * 0.5;
    *core_fock_ = *fock0;
  }
  fock0->fill_upper();
  dgemm_("n","n",nbasis_,nocc_,nbasis_,1.0,fock0->data(),nbasis_,cdata,nbasis_,0.0,aobuff.get(),nbasis_);

  unique_ptr<double[]> buf(new double[nocc_*nocc_]);
  unique_ptr<double[]> out(new double[nocc_*nocc_]);
  dgemm_("t","n",nocc_,nocc_,nbasis_,1.0,cdata,nbasis_,aobuff.get(),nbasis_,0.0,out.get(),nocc_);

  return make_tuple(move(out), core_energy);
}


unique_ptr<double[]> NewJop::compute_mo2e(const int nstart, const int nfence) {

  const int nocc = nfence - nstart;
  double* cdata = coeff_->data() + nstart*nbasis_;

  // first half transformation
  shared_ptr<DF_Half> half = geom_->df()->compute_half_transform(cdata, nocc);

  // second index transformation and (D|ii) = J^-1/2_DE (E|ii)
  shared_ptr<DF_Full> buf = half->compute_second_transform(cdata, nocc)->apply_J();

  // we want to store half-transformed quantity for latter convenience
  mo2e_1ext_size_ = nocc*geom_->df()->naux()*nbasis_;
  mo2e_1ext_ = half;

  // assembles (ii|ii) = (ii|D)(D|ii)
  return buf->form_4index();

}

