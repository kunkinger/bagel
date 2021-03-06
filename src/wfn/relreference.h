//
// BAGEL - Parallel electron correlation program.
// Filename: relreference.h
// Copyright (C) 2013 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
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

#ifndef __SRC_REL_RELREFERENCE_H
#define __SRC_REL_RELREFERENCE_H

#include <src/wfn/reference.h>

namespace bagel {

class RelReference : public Reference {
  protected:
    bool gaunt_;
    bool breit_;
    int nneg_;
    std::shared_ptr<const ZMatrix> relcoeff_;
    std::shared_ptr<const ZMatrix> relcoeff_full_;

    bool rel_;  // Non-relativistic GIAO wavefunctions also use this class
    bool kramers_;  // Indicates whether or not relcoeff_ has been kramers-adapted

  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int) {
      ar & boost::serialization::base_object<Reference>(*this) & gaunt_ & breit_ & nneg_ & relcoeff_ & relcoeff_full_ & rel_ & kramers_;
    }

  public:
    RelReference() { }
    RelReference(std::shared_ptr<const Geometry> g, std::shared_ptr<const ZMatrix> c, const double en,
                 const int nneg, const int nocc, const int nact, const int nvirt, const bool ga, const bool br, const bool rel = true, const bool kram = false)
     : Reference(g, nullptr, nocc, nact, nvirt, en), gaunt_(ga), breit_(br), nneg_(nneg), relcoeff_(c->slice_copy(nneg_, c->mdim())), relcoeff_full_(c), rel_(rel), kramers_(kram) {
    }

    std::shared_ptr<const Coeff> coeff() const override { throw std::logic_error("RelReference::coeff() should not be called"); }
    std::shared_ptr<const ZMatrix> relcoeff() const { return relcoeff_; }
    std::shared_ptr<const ZMatrix> relcoeff_full() const { return relcoeff_full_; }

    bool gaunt() const { return gaunt_; }
    bool breit() const { return breit_; }
    int nneg() const { return nneg_; }

    bool rel() const { return rel_; }
    bool kramers() const { return kramers_; }

    std::shared_ptr<Reference> project_coeff(std::shared_ptr<const Geometry> geomin, const bool check_geom_change = true) const override;

};

}

#include <src/util/archive.h>
BOOST_CLASS_EXPORT_KEY(bagel::RelReference)

#endif
