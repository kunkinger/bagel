//
// Author : Toru Shiozaki
// Date   : August 2009
//

#include <cstring>
#include <iostream>
#include <algorithm>
#include <src/pmp2/pmp2.h>
#include <src/macros.h>
#include <src/scf/scf_macros.h>
#include <src/slater/slaterbatch.h>
#include <src/util/paircompfile.h>
#include <src/util/pcompcabsfile.h>
#include <src/util/pmofile.h>

typedef boost::shared_ptr<Atom> RefAtom;
typedef boost::shared_ptr<PGeometry> RefGeom;
typedef boost::shared_ptr<PCoeff> RefPCoeff;
typedef boost::shared_ptr<Shell> RefShell;
typedef boost::shared_ptr<PMOFile<std::complex<double> > > RefPMOFile;
typedef boost::shared_ptr<PMatrix1e> RefMatrix;


////////////////////////////////////////////////////////////////////////////
// toggles whether we symmetrize explicitly or not (12|34)<->(21|43)
//#define EXPLICITLY_HERMITE
////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace boost;

PMP2::PMP2(const RefGeom g, const RefPCoeff co, const vector<double> eg, const shared_ptr<PCompFile<ERIBatch> > fl)
 : geom_(g), coeff_(co), eig_(eg.begin(), eg.end()), eri_obs_(fl) {

  nfrc_ = geom_->nfrc() / 2;
  nocc_ = geom_->nocc() / 2;
  nocc_act_ = nocc_ - nfrc_;
  nbasis_ = geom_->nbasis();
  nvir_ = nbasis_ - nocc_;
  noovv_ = nocc_act_ * nocc_act_ * nbasis_ * nbasis_;
  ncabs_ = geom_->ncabs();

  assert(geom_->ncabs() != 0);

}


PMP2::~PMP2() {

}


void PMP2::compute() {
  const double gamma = geom_->gamma();

  // AO ERI has been computed in the SCF class.

  cout << "  === Periodic MP2 calculation ===" << endl << endl;
  // Fully transform aa/ii integrals and dump them to disk (... focus is on MP2-R12).
  eri_ii_pp_ = eri_obs_->mo_transform(coeff_, coeff_, coeff_, coeff_,
                                      nfrc_, nocc_, nfrc_, nocc_,
                                      0, nbasis_, 0, nbasis_, "ERI (pp/ii)");
  eri_ii_pp_->sort_inside_blocks();

  ///////////////////////////////////////////////
  // Compute the conventional MP2 contribution
  /////////////////////////////////////////////
  compute_conv_mp2();

  cout << "  === Periodic MP2-R12 calculation ===" << endl << endl;


  ////////// //////////////////////////////
  // Slater & Yukawa potential integrals
  ///////////////////////////////////////
  shared_ptr<PairCompFile<SlaterBatch> > stg_yp(new PairCompFile<SlaterBatch>(geom_, gamma, "Slater and Yukawa ints"));
  stg_yp->store_integrals();
  stg_yp->reopen_with_inout();
  shared_ptr<PCompFile<SlaterBatch> > stg = stg_yp->first();
  shared_ptr<PCompFile<SlaterBatch> > yp  = stg_yp->second();


  //////////////////////////////////////
  // Yukawa ii/ii for V intermediate
  ////////////////////////////////////
  {
    RefPMOFile yp_ii_ii = yp->mo_transform(coeff_, coeff_, coeff_, coeff_,
                                           nfrc_, nocc_, nfrc_, nocc_,
                                           nfrc_, nocc_, nfrc_, nocc_, "Yukawa (ii/ii)");
    yp_ii_ii->sort_inside_blocks();
    yp_ii_ii_ = yp_ii_ii;
    RefPMOFile stg_ii_pp = stg->mo_transform(coeff_, coeff_, coeff_, coeff_,
                                             nfrc_, nocc_, nfrc_, nocc_,
                                             0, nbasis_, 0, nbasis_, "Slater (pp/ii)");
    stg_ii_pp->sort_inside_blocks();
    stg_ii_pp_ = stg_ii_pp;
  }


  ////////////////////
  // CABS integrals
  //////////////////
  {
    shared_ptr<PCompCABSFile<ERIBatch> >
      eri_cabs(new PCompCABSFile<ERIBatch>(geom_, gamma, false, false, false, true, false, "ERI CABS"));
    eri_cabs->store_integrals();
    eri_cabs->reopen_with_inout();
    eri_cabs_ = eri_cabs;
  }

  shared_ptr<PCompCABSFile<SlaterBatch> >
    stg_cabs(new PCompCABSFile<SlaterBatch>(geom_, gamma, false, false, false, true, false, "Slater CABS"));
  stg_cabs->store_integrals();
  stg_cabs->reopen_with_inout();


  ///////////////////////////
  // Coefficients of CABS;
  /////////////////////////
  pair<RefCoeff, RefCoeff> cabs_pairs = generate_CABS();
  cabs_obs_ = cabs_pairs.first;
  cabs_aux_ = cabs_pairs.second;
  // Setting actual number of CABS.
  ncabs_ = cabs_obs_->mdim();


  ///////////////////////////////
  // MO transformation for ERI
  /////////////////////////////
  {
    RefPMOFile eri_ii_ip = eri_obs_->mo_transform(coeff_, coeff_, coeff_, cabs_obs_,
                                                  nfrc_, nocc_, nfrc_, nocc_,
                                                  0, nocc_, 0, ncabs_, "v^ia'_ii, OBS part");
    eri_ii_ip->sort_inside_blocks();

    RefPMOFile eri_ii_ix = eri_cabs_->mo_transform_cabs_aux(coeff_, coeff_, coeff_, cabs_aux_,
                                                           nfrc_, nocc_, nfrc_, nocc_,
                                                           0, nocc_, 0, ncabs_, "v^ia'_ii, auxiliary functions");
    eri_ii_ix->sort_inside_blocks();
    RefPMOFile eri_ii_iA(new PMOFile<complex<double> >(*eri_ii_ix + *eri_ii_ip));
    eri_ii_iA_ = eri_ii_iA;
  }

  ///////////////////////////////
  // MO transformation for STG
  /////////////////////////////
  {
    RefPMOFile stg_ii_ip = stg->mo_transform(coeff_, coeff_, coeff_, cabs_obs_,
                                             nfrc_, nocc_, nfrc_, nocc_,
                                             0, nocc_, 0, ncabs_, "F^ia'_ii, OBS part");
    stg_ii_ip->sort_inside_blocks();
    RefPMOFile stg_ii_ix = stg_cabs->mo_transform_cabs_aux(coeff_, coeff_, coeff_, cabs_aux_,
                                                           nfrc_, nocc_, nfrc_, nocc_,
                                                           0, nocc_, 0, ncabs_, "F^ia'_ii, auxiliary functions");
    stg_ii_ix->sort_inside_blocks();
    RefPMOFile stg_ii_iA(new PMOFile<complex<double> >(*stg_ii_ix + *stg_ii_ip));
    stg_ii_iA_ = stg_ii_iA;
  }


  //////////////////////
  // V intermediate
  ////////////////////
  {
    RefPMOFile vF = stg_ii_pp_->contract(eri_ii_pp_, "F * v (ii/ii) OBS");
    RefPMOFile V_obs(new PMOFile<complex<double> >(*yp_ii_ii_ - *vF));

    RefPMOFile V_cabs = stg_ii_iA_->contract(eri_ii_iA_, "F * v (ii/ii) CABS");

    V_cabs->flip_symmetry();
    RefPMOFile V_pre(new PMOFile<complex<double> >(*V_obs - *V_cabs));

    V_ = V_pre;
  }
  complex<double> en_vt = V_->get_energy_one_amp();


  ///////////////////////////////////////
  // Direct contribution to R12 energy
  /////////////////////////////////////
  cout << "  * Built V intermediate." << endl;
  cout << "  F12 energy (Vt): " << setprecision(10) << en_vt.real() << endl << endl;


  /////////////////////////////////////
  // Slater & Yukawa ints with 2gamma
  ///////////////////////////////////
  shared_ptr<PairCompFile<SlaterBatch> >
    stg_yp2(new PairCompFile<SlaterBatch>(geom_, 2.0 * gamma, "Slater and Yukawa ints (2gamma)"));
  stg_yp2->store_integrals();
  stg_yp2->reopen_with_inout();
  shared_ptr<PCompFile<SlaterBatch> > stg2 = stg_yp2->first();
  shared_ptr<PCompFile<SlaterBatch> > yp2  = stg_yp2->second();

  shared_ptr<PCompCABSFile<SlaterBatch> >
    stg2_cabs(new PCompCABSFile<SlaterBatch>(geom_, 2.0 * gamma, false, false, false, true,
                                             false, "Slater CABS (2gamma)"));
  stg2_cabs->store_integrals();
  stg2_cabs->reopen_with_inout();


  ////////////////////
  // X intermediate
  //////////////////
  {
    RefPMOFile stg2_ii_ii = stg2->mo_transform(coeff_, coeff_, coeff_, coeff_,
                                               nfrc_, nocc_, nfrc_, nocc_,
                                               nfrc_, nocc_, nfrc_, nocc_, "Slater (ii/ii) 2gamma");
    stg2_ii_ii->sort_inside_blocks();
    stg2_ii_ii_ = stg2_ii_ii;
    RefPMOFile FF = stg_ii_pp_->contract(stg_ii_pp_, "F * F (ii/ii) OBS");
    RefPMOFile X_obs(new PMOFile<complex<double> >(*stg2_ii_ii - *FF));

    RefPMOFile X_cabs = stg_ii_iA_->contract(stg_ii_iA_, "F * F (ii/ii) CABS");

    X_cabs->flip_symmetry();
    RefPMOFile X_pre(new PMOFile<complex<double> >(*X_obs - *X_cabs));

    X_ = X_pre;
  }


  /////////////////////
  // B intermediate
  ///////////////////
  {
    // T intermediate (direct)
    RefPMOFile T(new PMOFile<complex<double> >(*stg2_ii_ii_ * (gamma*gamma)));

    // Q intermediate (made of X * h)
    RefPMOFile Q;
    {
      // Hartree builder (needs modification!!!)
      // nbasis * nbasis size
      RefMatrix hj_obs = generate_hJ_obs_obs();
      // nbasis * ncabs size
      RefMatrix hj_cabs = generate_hJ_obs_cabs();


      // Hartree weighted index
      RefMatrix hj_ip(new PMatrix1e(hj_obs, make_pair(0, nocc_)));
      RefPCoeff chj_ip(new PCoeff(*coeff_ * *hj_ip));

      RefMatrix hj_iA(new PMatrix1e(hj_cabs, make_pair(0, nocc_)));
      RefPCoeff chj_iA_comb(new PCoeff(*coeff_cabs_ * *hj_iA));

      pair<RefMatrix, RefMatrix> chj_iA = chj_iA_comb->split(geom_->nbasis(), geom_->ncabs());
      RefMatrix chj_iA_obs = chj_iA.first;
      RefCoeff chj_iA_cabs(new PCoeff(*chj_iA.second));

      *chj_ip += *chj_iA_obs;
      chj_ip->scale(0.5);
      chj_iA_cabs->scale(0.5);

      // MO transform using Hartree-weighted index
      RefPMOFile X_ii_ih = stg2->mo_transform(coeff_, coeff_, coeff_, chj_ip,
                                              nfrc_, nocc_, nfrc_, nocc_,
                                              nfrc_, nocc_, nfrc_, nocc_, "Q intermediate: stg2 (OBS) 1/2");
      X_ii_ih->sort_inside_blocks();
      RefPMOFile X_ii_ih_cabs = stg2_cabs->mo_transform_cabs_aux(coeff_, coeff_, coeff_, chj_iA_cabs,
                                                                 nfrc_, nocc_, nfrc_, nocc_,
                                                                 nfrc_, nocc_, nfrc_, nocc_, "Q intermediate: stg2 (CABS) 1/2");
      X_ii_ih_cabs->sort_inside_blocks();

      *X_ii_ih += *X_ii_ih_cabs;

// might not be needed since we are using fixed amplitudes.
#ifdef EXPLICITLY_HERMITE
      RefPMOFile X_ih_ii = stg2->mo_transform(coeff_, chj_ip, coeff_, coeff_,
                                              nfrc_, nocc_, nfrc_, nocc_,
                                              nfrc_, nocc_, nfrc_, nocc_, "Q intermediate: stg2 (OBS) 2/2");
      X_ih_ii->sort_inside_blocks();
      *X_ii_ih += *X_ih_ii;
#else
      X_ii_ih->scale(2.0);
#endif

      X_ii_ih->flip_symmetry();
      RefPMOFile Qtmp(new PMOFile<complex<double> >(*X_ii_ih));
      Q = Qtmp;
    } // end of Q intermediate construction.

    Q->rprint();

  } // end of B intermediate construction.

#ifdef LOCAL_DEBUG_PMP2
  V->print();
  cout << endl;
  X->print();
#endif

}


