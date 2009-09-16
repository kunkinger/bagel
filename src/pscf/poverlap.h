//
// Author : Toru Shiozaki
// Date   : July 2009
//

#ifndef __src_pscf_poverlap_h
#define __src_pscf_poverlap_h

#include <src/util/pmatrix1e.h>
#include <src/pscf/pgeometry.h>
#include <boost/shared_ptr.hpp>

class POverlap : public PMatrix1e {
  protected:
    void computebatch(const std::vector<boost::shared_ptr<Shell> >&, const int, const int, const int, const int);

  public:
    POverlap(const boost::shared_ptr<PGeometry>);
    ~POverlap();

    const int calculate_thresh() const;

};

#endif

