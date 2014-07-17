//
// BAGEL - Parallel electron correlation program.
// Filename: meh_compute_rdm.cc
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: NU theory
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
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

#include <src/meh/meh_base.h>
#include <src/meh/state_tensor.h>

using namespace std;
using namespace bagel;
using namespace btas;

void MEH_base::compute_rdm() const {
  const int norbA = dimer_->active_refs().first->nact();
  const int norbB = dimer_->active_refs().second->nact();

  RDM<2> rdm(norbA+norbB);
  // compute transformed gammas

  StateTensor st(adiabats_, subspaces_base());
  st.print();

  // TODO parallelize
  // Loop over both tensors and mupltiply
  const int istate = 0;
  GammaTensor half;
  for (auto& i : *gammatensor_[0]) {
    for (auto& j : st) {
      // if the third index of the gamma tensor is identical to the first one of the state tensor we contract
      if (get<0>(j.first) == istate && get<2>(i.first) == get<1>(j.first)) {
        auto tag = make_tuple(get<0>(i.first), get<1>(i.first), get<2>(j.first));
        auto data = make_shared<Tensor3<double>>(get<0>(i.first).size, get<1>(i.first).nstates(), get<2>(j.first).nstates());
        contract(1.0, *i.second, {0,1,2}, j.second, {2,3}, 0.0, *data, {0,1,3});
        half.emplace(tag, data);
      }
    }
  }

#if 0
  // diagonal term
  for (auto& subspace : subspaces_) {
    compute_pure_terms(subspace, jop_);
    std::shared_ptr<Matrix> block = compute_diagonal_block<false>(subspace);
  }
#endif
  // off diagonal term
#if 0
  for (auto iAB = subspaces_.begin(); iAB != subspaces_.end(); ++iAB) {
    for (auto jAB = subspaces_.begin(); jAB != iAB; ++jAB) {
      rdm += *couple_blocks<false>(*iAB, *jAB);
    }
  }
#endif

}