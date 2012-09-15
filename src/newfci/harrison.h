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

// Desc :: The implementation closely follows Harrison and Zarrabian
//

#ifndef __NEWINT_NEWFCI_HARRISONZARRABIAN_H
#define __NEWINT_NEWFCI_HARRISONZARRABIAN_H

#include <tuple>
#include <src/scf/scf.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <bitset>
#include <src/util/input.h>
#include <src/util/constants.h>
#include <src/newfci/dvec.h>
#include <src/newfci/mofile.h>
#include <src/newfci/fci.h>
#include <src/newfci/space.h>
#include <src/wfn/rdm.h>
#include <src/wfn/reference.h>
#include <src/newfci/determinants.h>

namespace bagel {

class HarrisonZarrabian : public NewFCI {

  protected:
    //void create_Jiiii();

    std::shared_ptr<Space> space_;

    void const_denom() override;
  
    std::shared_ptr<NewDvec> form_sigma(std::shared_ptr<const NewDvec> c, std::shared_ptr<const NewMOFile> jop, const std::vector<int>& conv) const override;

    // run-time functions
    void sigma_1(std::shared_ptr<const NewCivec> cc, std::shared_ptr<NewCivec> sigma, std::shared_ptr<const NewMOFile> jop) const;
    void sigma_3(std::shared_ptr<const NewCivec> cc, std::shared_ptr<NewCivec> sigma, std::shared_ptr<const NewMOFile> jop) const;
    void sigma_2aa  (std::shared_ptr<const NewCivec> cc, std::shared_ptr<NewCivec> sigma, std::shared_ptr<const NewMOFile> jop) const;
    void sigma_2bb  (std::shared_ptr<const NewCivec> cc, std::shared_ptr<NewCivec> sigma, std::shared_ptr<const NewMOFile> jop) const;
    void sigma_2ab_1(std::shared_ptr<const NewCivec> cc, std::shared_ptr<NewDvec> d) const;
    void sigma_2ab_2(std::shared_ptr<NewDvec> d, std::shared_ptr<NewDvec> e, std::shared_ptr<const NewMOFile> jop) const;
    void sigma_2ab_3(std::shared_ptr<NewCivec> sigma, std::shared_ptr<NewDvec> e) const;

  public:
    // this constructor is ugly... to be fixed some day...
    HarrisonZarrabian(const std::multimap<std::string, std::string> a, std::shared_ptr<const Reference> b,
        const int ncore = -1, const int nocc = -1, const int nstate = -1) : NewFCI(a, b, ncore, nocc, nstate) {
      space_ = std::shared_ptr<Space>(new Space(det_, 1));
      update(ref_->coeff());
    };
    ~HarrisonZarrabian(){};

    void update(std::shared_ptr<const Coeff>) override; 
};

}

#endif
