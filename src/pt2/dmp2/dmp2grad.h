//
// BAGEL - Parallel electron correlation program.
// Filename: dmp2grad.h
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


#ifndef __SRC_REL_DMP2GRAD_H
#define __SRC_REL_DMP2GRAD_H

#include <src/pt2/dmp2/dmp2.h>
#include <src/wfn/reference.h>

namespace bagel {

class DMP2Grad : public DMP2 {
  protected:

  public:
    DMP2Grad(std::shared_ptr<const PTree>, std::shared_ptr<const Geometry>, std::shared_ptr<const Reference>);

    void compute() override;

    std::shared_ptr<const Reference> conv_to_ref() const override { return ref_; }
};

}

#endif
