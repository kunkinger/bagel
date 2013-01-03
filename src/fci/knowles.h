//
// BAGEL - Parallel electron correlation program.
// Filename: fci.h
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

// Desc :: The implementation closely follows Knowles and Handy 1984 CPL.
//         It is amazing how easy it is to implement FCI !!
//

#ifndef __NEWINT_FCI_KNOWLESHANDY_H
#define __NEWINT_FCI_KNOWLESHANDY_H

#include <tuple>
#include <src/scf/scf.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <bitset>
#include <src/util/input.h>
#include <src/util/constants.h>
#include <src/fci/dvec.h>
#include <src/fci/mofile.h>
#include <src/fci/fci.h>
#include <src/wfn/rdm.h>
#include <src/wfn/reference.h>

namespace bagel {

class KnowlesHandy : public FCI {

  protected:
    // denominator
    void const_denom() override;

    // virtual application of Hamiltonian
    std::shared_ptr<Dvec> form_sigma(std::shared_ptr<const Dvec> c, std::shared_ptr<const MOFile> jop, const std::vector<int>& conv) const override;

    // run-time functions
    void sigma_1(std::shared_ptr<const Civec> cc, std::shared_ptr<Civec> sigma, std::shared_ptr<const MOFile> jop) const;
    void sigma_3(std::shared_ptr<const Civec> cc, std::shared_ptr<Civec> sigma, std::shared_ptr<const MOFile> jop) const;
    void sigma_2b (std::shared_ptr<Dvec> d, std::shared_ptr<Dvec> e, std::shared_ptr<const MOFile> jop) const;
    void sigma_2c1(std::shared_ptr<Civec> sigma, std::shared_ptr<const Dvec> e) const;
    void sigma_2c2(std::shared_ptr<Civec> sigma, std::shared_ptr<const Dvec> e) const;

  public:
    // this constructor is ugly... to be fixed some day...
    KnowlesHandy(const std::multimap<std::string, std::string> a, std::shared_ptr<const Reference> b,
        const int ncore = -1, const int nocc = -1, const int nstate = -1) : FCI( a, b, ncore, nocc, nstate){
      update(ref_->coeff());
    }

    void update(std::shared_ptr<const Coeff>) override;
};

}

#endif

