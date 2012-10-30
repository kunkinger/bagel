//
// BAGEL - Parallel electron correlation program.
// Filename: cphf.cc
// Copyright (C) 2012 Toru Shiozaki
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


#include <stddef.h>
#include <src/grad/cphf.h>
#include <src/util/matrix.h>

#define CPHF_MAX_ITER 100
#define CPHF_THRESH 1.0e-8

using namespace std;
using namespace bagel;

CPHF::CPHF(const shared_ptr<const Matrix1e> grad, const vector<double>& eig, const shared_ptr<const DF_Half> h,
           const shared_ptr<const Reference> r)
: solver_(new LinearRM<Matrix1e>(CPHF_MAX_ITER, grad)), grad_(grad), eig_(eig), halfjj_(h), ref_(r), geom_(r->geom()) {

}


shared_ptr<Matrix1e> CPHF::solve() const {

  const size_t naux = geom_->naux();
  const size_t nocca = ref_->nocc();
  const size_t nvirt = geom_->nbasis() - nocca;

  const size_t nbasis = geom_->nbasis();

  Matrix ocoeff(nbasis, nocca);
  copy_n(ref_->coeff()->data(), nbasis*nocca, ocoeff.data());
  
  Matrix vcoeff(nbasis, nvirt);
  copy_n(ref_->coeff()->data()+nocca*nbasis, nbasis*nvirt, vcoeff.data());

  shared_ptr<Matrix1e> t(new Matrix1e(geom_));
  for (int i = 0; i != nocca; ++i)
    for (int a = nocca; a != nvirt+nocca; ++a)
      t->element(a,i) = grad_->element(a,i) / (eig_[a]-eig_[i]);

  Matrix jri(nbasis, nocca);
  Matrix jai(nvirt, nocca);
  Matrix kia(nocca, nvirt);

  cout << "  === CPHF iteration ===" << endl << endl;

  // TODO Max iter to be controlled by the input
  for (int iter = 0; iter != CPHF_MAX_ITER; ++iter) {
    solver_->orthog(t);

    shared_ptr<Matrix1e> sigma(new Matrix1e(geom_));
    // one electron part
    for (int i = 0; i != nocca; ++i)
      for (int a = nocca; a != nocca+nvirt; ++a)
        (*sigma)(a,i) = (eig_[a]-eig_[i]) * t->element(a,i);

    // J part
    Matrix pbmao(nbasis, nbasis);
    {
      Matrix pms(nocca, nbasis);
      dgemm_("T", "T", nocca, nbasis, nvirt, 1.0, t->element_ptr(nocca, 0), nbasis, vcoeff.data(), nbasis, 0.0, pms.data(), nocca);
      pbmao = ocoeff * pms;
    }
    pbmao.symmetrize();
    {
      Matrix jrs(nbasis, nbasis);
      copy_n(geom_->df()->compute_Jop(pbmao.data()).get(), nbasis*nbasis, jrs.data());
      jri = jrs * ocoeff;
    }
    jai = (vcoeff % jri) * 4.0;

    // K part
    {
      // halfjj is an half transformed DF integral with J^{-1}_{DE}, given by the constructor
      Matrix kir(nocca, nbasis);
      copy_n(halfjj_->compute_Kop_1occ(pbmao.data()).get(), nocca*nbasis, kir.data());
      kia = (kir * vcoeff) * (-2.0);
    }
    for (int i = 0; i != nocca; ++i)
      for (int a = 0; a != nvirt; ++a)
        (*sigma)(a+nocca,i) += jai[a+nvirt*i] + kia[i+nocca*a];

    t = solver_->compute_residual(t, sigma);

    // TODO to be controlled by the input
    if (t->norm() < CPHF_THRESH) break;

    for (int i = 0; i != nocca; ++i)
      for (int a = nocca; a != nvirt+nocca; ++a)
        t->element(a,i) /= (eig_[a]-eig_[i]);

    cout << setw(6) << iter << setw(20) << setprecision(10) << t->norm() << endl;

  }

  cout << endl;
  t = solver_->civec();
  t->fill_upper();
  return t;

}
