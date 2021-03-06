//
// BAGEL - Parallel electron correlation program.
// Filename: zharrison.h
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

// Desc :: The implementation closely follows Harrison and Zarrabian
//

#ifndef __BAGEL_ZFCI_ZHARRISON_H
#define __BAGEL_ZFCI_ZHARRISON_H

#include <src/wfn/method.h>
#include <src/wfn/relreference.h>
#include <src/ci/zfci/relmofile.h>
#include <src/ci/zfci/reldvec.h>
#include <src/util/math/davidson.h>

namespace bagel {

class ZHarrison : public Method {

  protected:
    // max #iteration
    int max_iter_;
    int davidson_subspace_;

    // threshold for variants
    double thresh_;
    double print_thresh_;

    // numbers of electrons
    int nele_;
    int ncore_;
    int norb_;
    int charge_;

    // breit and gaunt
    bool gaunt_;
    bool breit_;

    // enforce time-reversal symmetry
    bool tsymm_;

    // number of states
    int nstate_;
    std::vector<int> states_;

    // total energy
    std::vector<double> energy_;

    // CI vector at convergence
    std::shared_ptr<RelZDvec> cc_;

    // MO integrals
    std::shared_ptr<RelMOFile> jop_;

    // Determinant space
    std::shared_ptr<const RelSpace> space_;
    std::shared_ptr<const RelSpace> int_space_;

    // denominator
    std::shared_ptr<RelDvec> denom_;

    // RDMs
    std::vector<std::unordered_map<std::bitset<2>, std::shared_ptr<ZRDM<1>>>> rdm1_;
    std::vector<std::unordered_map<std::bitset<4>, std::shared_ptr<ZRDM<2>>>> rdm2_;
    // state averaged RDMs
    std::unordered_map<std::bitset<2>, std::shared_ptr<ZRDM<1>>> rdm1_av_;
    std::unordered_map<std::bitset<4>, std::shared_ptr<ZRDM<2>>> rdm2_av_;

    std::shared_ptr<DavidsonDiag<RelZDvec, ZMatrix>> davidson_;

    // restart
    bool restart_;
    bool restarted_;

  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      boost::serialization::split_member(ar, *this, version);
    }
    template<class Archive>
    void save(Archive& ar, const unsigned int) const {
      ar << boost::serialization::base_object<Method>(*this);
      ar << max_iter_ << davidson_subspace_ << thresh_ << print_thresh_ << nele_ << ncore_ << norb_ << charge_ << gaunt_ << breit_ << tsymm_
         << nstate_ << states_ << energy_ << cc_ << space_ << int_space_ << denom_ << rdm1_ << rdm2_ << rdm1_av_ << rdm2_av_ << davidson_ << restart_ << restarted_;
      // for jop_
      std::shared_ptr<const ZMatrix> coeff = jop_->coeff_input();
      ar << coeff;
    }
    template<class Archive>
    void load(Archive& ar, const unsigned int) {
      ar >> boost::serialization::base_object<Method>(*this);
      ar >> max_iter_ >> davidson_subspace_ >> thresh_ >> print_thresh_ >> nele_ >> ncore_ >> norb_ >> charge_ >> gaunt_ >> breit_ >> tsymm_
         >> nstate_ >> states_ >> energy_ >> cc_ >> space_ >> int_space_ >> denom_ >> rdm1_ >> rdm2_ >> rdm1_av_ >> rdm2_av_ >> davidson_ >> restart_ >> restarted_;
      std::shared_ptr<const ZMatrix> coeff;
      ar >> coeff;
      update(coeff);
      restarted_ = true;
    }

  protected:
    // obtain determinants for guess generation
    void generate_guess(const int nelea, const int neleb, const int nstate, std::shared_ptr<RelZDvec> out, const int offset);
    // generate spin-adapted guess configurations
    virtual std::vector<std::pair<std::bitset<nbit__>, std::bitset<nbit__>>> detseeds(const int ndet, const int nelea, const int neleb);

    // print functions
    void print_header() const;

    void const_denom();

    // just to move negative energy states into the virtual space
    std::shared_ptr<const ZMatrix> swap_pos_neg(std::shared_ptr<const ZMatrix> coeffin) const;

    // run-time functions.
    // aaaa and bbbb
    void sigma_aa(std::shared_ptr<const ZCivec> cc, std::shared_ptr<ZCivec> sigma, std::shared_ptr<const RelMOFile> jop, const bool trans = false) const;
    void sigma_1e_ab(std::shared_ptr<const ZCivec> cc, std::shared_ptr<ZCivec> sigma, std::shared_ptr<const RelMOFile> jop, const bool trans = false) const;

    void sigma_2e_annih_ab(std::shared_ptr<const ZCivec> cc, std::shared_ptr<ZDvec> d) const;
    void sigma_2e_annih_aa(std::shared_ptr<const ZCivec> cc, std::shared_ptr<ZDvec> d) const;

    void sigma_2e_create_bb(std::shared_ptr<ZCivec> sigma, std::shared_ptr<const ZDvec> e) const;
    void sigma_2e_create_ab(std::shared_ptr<ZCivec> sigma, std::shared_ptr<const ZDvec> e) const;

    void sigma_2e_h0101_h1001(std::shared_ptr<const ZDvec> d, std::shared_ptr<ZDvec> e, std::shared_ptr<const RelMOFile> jop) const;

    template<int i, int j, int k, int l>
    void sigma_2e_h(std::shared_ptr<const ZDvec> d, std::shared_ptr<ZDvec> e, std::shared_ptr<const RelMOFile> jop, const bool trans, const std::complex<double> fac = 1.0) const {
      static_assert(!((i|1)^1) && !((j|1)^1) && !((k|1)^1) && !((l|1)^1), "illegal call of sigma_2e_h");
      const int ij = d->ij();
      const int lenab = d->lena()*d->lenb();
      std::stringstream ss; ss << i << j << k << l;
      std::bitset<4> bit4(ss.str());
      if (trans) bit4 = ~bit4;
      zgemm3m_("n", "t", lenab, ij, ij, fac, d->data(), lenab, jop->mo2e(bit4)->data(), ij, 0.0, e->data(), lenab);
    }

    void sigma_one(std::shared_ptr<const ZCivec> cc, std::shared_ptr<RelZDvec> sigmavec, std::shared_ptr<const RelMOFile> jop,
                   const int istate, const bool diag, const bool trans) const;
#ifdef HAVE_MPI_H
    int sigma_one_parallel(const int icnt, std::shared_ptr<const ZCivec> cc, std::shared_ptr<RelZDvec> sigmavec, std::shared_ptr<const RelMOFile> jop,
                           const int istate, const bool diag, const bool trans) const;
#endif

  public:
    ZHarrison() { }
    // this constructor is ugly... to be fixed some day...
    ZHarrison(std::shared_ptr<const PTree> a, std::shared_ptr<const Geometry> g, std::shared_ptr<const Reference> b,
              const int ncore = -1, const int nocc = -1, const int nstate = -1, std::shared_ptr<const ZMatrix> coeff_zcas = nullptr, const bool restricted = false);

    std::shared_ptr<RelZDvec> form_sigma(std::shared_ptr<const RelZDvec> c, std::shared_ptr<const RelMOFile> jop, const std::vector<int>& conv) const;

    // "restricted" refers to whether the coefficient matrix is already Kramers-adapted
    void update(std::shared_ptr<const ZMatrix> coeff, const bool restricted = false) {
      Timer timer;
      jop_ = std::make_shared<RelJop>(geom_, ncore_*2, (ncore_+norb_)*2, coeff, charge_, gaunt_, breit_, restricted, tsymm_);

      std::cout << "    * Integral transformation done. Elapsed time: " << std::setprecision(2) << timer.tick() << std::endl << std::endl;
      const_denom();
    }

    void compute() override;

    // returns members
    int norb() const { return norb_; }
    int ncore() const { return ncore_; }
    double core_energy() const { return jop_->core_energy(); }

    int nij() const { return norb_*norb_; }

    // TODO
    std::shared_ptr<const Reference> conv_to_ref() const override { return nullptr; }

    std::vector<double> energy() const { return energy_; }

    void compute_rdm12();

    std::shared_ptr<const ZMatrix> mo2e_full() const;
    std::shared_ptr<const ZMatrix> rdm1_av() const;
    std::shared_ptr<const ZMatrix> rdm2_av() const;
    std::shared_ptr<const ZRDM<1>> rdm1_av_kramers(std::string&& b) const { return rdm1_av_.at(std::bitset<2>(b)); }
    std::shared_ptr<const ZRDM<2>> rdm2_av_kramers(std::string&& b) const { return rdm2_av_.at(std::bitset<4>(b)); }
    std::shared_ptr<const ZRDM<1>> rdm1_av_kramers(const std::bitset<2>& b) const { return rdm1_av_.at(b); }
    std::shared_ptr<const ZRDM<2>> rdm2_av_kramers(const std::bitset<4>& b) const { return rdm2_av_.at(b); }
    std::unordered_map<std::bitset<4>, std::shared_ptr<ZRDM<2>>> rdm2_av_kramers() { return rdm2_av_; }

    std::shared_ptr<const RelMOFile> jop() const { return jop_; }
    std::shared_ptr<const ZMatrix> coeff() const { return jop_->coeff(); }
    std::array<std::shared_ptr<const ZMatrix>,2> kramers_coeff() const { return jop_->kramers_coeff(); }
    void update_kramers_coeff(std::shared_ptr<ZMatrix> natorb_transform) {
      jop_->update_kramers_coeff(natorb_transform);
    }
};

}

#include <src/util/archive.h>
BOOST_CLASS_EXPORT_KEY(bagel::ZHarrison)

#endif

