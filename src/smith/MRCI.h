//
// BAGEL - Parallel electron correlation program.
// Filename: MRCI.h
// Copyright (C) 2014 Shiozaki group
//
// Author: Shiozaki group <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
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


#ifndef __SRC_SMITH_MRCI_H
#define __SRC_SMITH_MRCI_H

#include <iostream>
#include <tuple>
#include <iomanip>
#include <src/smith/spinfreebase.h>
#include <src/smith/futuretensor.h>
#include <src/scf/hf/fock.h>
#include <src/util/f77.h>
#include <src/smith/queue.h>
#include <src/smith/smith_info.h>

namespace bagel {
namespace SMITH {
namespace MRCI{

class MRCI : public SpinFreeMethod {
  protected:
    using SpinFreeMethod::ref_;
    using SpinFreeMethod::e0_;
    using SpinFreeMethod::closed_;
    using SpinFreeMethod::active_;
    using SpinFreeMethod::virt_;
    using SpinFreeMethod::ci_;
    using SpinFreeMethod::rclosed_;
    using SpinFreeMethod::ractive_;
    using SpinFreeMethod::rvirt_;
    using SpinFreeMethod::rci_;
    using SpinFreeMethod::h1_;
    using SpinFreeMethod::f1_;
    using SpinFreeMethod::v2_;
    using SpinFreeMethod::rdm1_;
    using SpinFreeMethod::rdm2_;
    using SpinFreeMethod::rdm3_;
    using SpinFreeMethod::rdm4_;
    using SpinFreeMethod::rdm1deriv_;
    using SpinFreeMethod::rdm2deriv_;
    using SpinFreeMethod::rdm3deriv_;
    using SpinFreeMethod::rdm4deriv_;

    std::shared_ptr<Tensor> t2;
    std::shared_ptr<Tensor> r;
    std::shared_ptr<Tensor> s;
    std::shared_ptr<Tensor> n;
    std::shared_ptr<Tensor> den1;
    std::shared_ptr<Tensor> den2;
    std::shared_ptr<Tensor> Den1;
    double correlated_norm_;
    std::shared_ptr<Tensor> deci;


    std::shared_ptr<FutureTensor> Gamma0_();
    std::shared_ptr<FutureTensor> Gamma2_();
    std::shared_ptr<FutureTensor> Gamma7_();
    std::shared_ptr<FutureTensor> Gamma1_();
    std::shared_ptr<FutureTensor> Gamma3_();
    std::shared_ptr<FutureTensor> Gamma4_();
    std::shared_ptr<FutureTensor> Gamma5_();
    std::shared_ptr<Queue> make_residualq();
    std::shared_ptr<Queue> make_sourceq();
    std::shared_ptr<Queue> make_normq();

  public:
    MRCI(std::shared_ptr<const SMITH_Info> ref);
    ~MRCI() {}

    void solve();
    void solve_deriv();

    double accumulate(std::shared_ptr<Queue> queue) {
      double sum = 0.0;
      while (!queue->done())
        sum += queue->next_compute()->target();
      return sum;
    }

    std::shared_ptr<const Matrix> rdm11() const { return den1->matrix(); }
    std::shared_ptr<const Matrix> rdm12() const { return den2->matrix(); }
    std::shared_ptr<const Matrix> rdm21() const { return Den1->matrix2(); }

    double correlated_norm() const { return correlated_norm_; }

    std::shared_ptr<const Civec> ci_deriv() const { return deci->civec(this->det_); }

};

}
}
}
#endif

