//
// BAGEL - Parallel electron correlation program.
// Filename: asd_base.h
// Copyright (C) 2014 Shane Parker
//
// Author: Shane Parker <shane.parker@u.northwestern.edu>
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

#ifndef __ASD_ASD_BASE_H
#define __ASD_ASD_BASE_H

#include <src/asd/dimer/dimer.h>
#include <src/asd/dimer/dimer_jop.h>
#include <src/asd/asd_spin.h>
#include <src/asd/gamma_tensor.h>
#include <src/asd/coupling.h>

namespace bagel {

/// Specifies a single block of a model Hamiltonian
struct ModelBlock {
  std::pair<int, int> S_;
  std::pair<int, int> charge_;
  std::pair<int, int> M_;
  int nstates_;

  ModelBlock(std::pair<int, int> S, std::pair<int, int> q, std::pair<int, int> M, const int nstates) :
    S_(S), charge_(q), M_(M), nstates_(nstates) {}
};

/// Base class for ASD. Contains everything that doesn't need to be templated.
class ASD_base {
  protected:
    std::shared_ptr<const Dimer> dimer_;

    std::shared_ptr<DimerJop> jop_;

    std::shared_ptr<Matrix> hamiltonian_; ///< if stored_ is true, Hamiltonian is stored here
    std::shared_ptr<Matrix> adiabats_; ///< Eigenvectors of adiabatic states
    std::vector<std::pair<std::string, std::shared_ptr<Matrix>>> properties_;

    std::unique_ptr<double[]> denom_; ///< denominator used for Davidson

    int max_spin_;
    std::shared_ptr<ASDSpin> spin_; ///< Sparsely stored spin matrix

    std::vector<double> energies_; ///< Adiabatic energies

    // Total system quantities
    int dimerstates_; ///< Total size of dimer Hamiltonian. Counted up during initialization

    // Options
    int nstates_;
    int nspin_;
    int charge_;
    int max_iter_;
    int nguess_;
    int davidson_subspace_;

    bool store_matrix_;
    bool dipoles_;

    double thresh_;
    double print_thresh_;

    virtual std::vector<DimerSubspace_base> subspaces_base() const = 0;

    // Gamma Tensor
    std::array<std::shared_ptr<const GammaTensor>,2> gammatensor_;
    std::shared_ptr<GammaTensor> worktensor_;

    std::vector<std::vector<ModelBlock>> models_to_form_; ///< Contains specifications to construct model spaces
    std::vector<std::pair<std::shared_ptr<Matrix>, std::shared_ptr<Matrix>>> models_; ///< models that have been built

    std::shared_ptr<Matrix> apply_hamiltonian(const Matrix& o, const std::vector<DimerSubspace_base>& subspaces);
    std::vector<double> diagonalize(std::shared_ptr<Matrix>& cc, const std::vector<DimerSubspace_base>& subspace, const bool mute = false);

    // Off-diagonal stuff
    template <bool _N, typename return_type = typename std::conditional<_N, Matrix, RDM<2>>::type>
    std::shared_ptr<return_type> couple_blocks(const DimerSubspace_base& AB, const DimerSubspace_base& ApBp) const; // Off-diagonal driver for H

    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type>
      compute_offdiagonal_1e(const std::array<MonomerKey,4>&, std::shared_ptr<const Matrix> h) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_inter_2e(const std::array<MonomerKey,4>&) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_aET(const std::array<MonomerKey,4>&) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_bET(const std::array<MonomerKey,4>&) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_abFlip(const std::array<MonomerKey,4>&) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_abET(const std::array<MonomerKey,4>&) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_aaET(const std::array<MonomerKey,4>&) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_bbET(const std::array<MonomerKey,4>&) const { assert(false); return nullptr; }
    template <bool _N> std::shared_ptr<typename std::conditional<_N, Matrix, RDM<2>>::type> compute_diagonal_block(const DimerSubspace_base& subspace) const { assert(false); return nullptr; }

    void generate_initial_guess(std::shared_ptr<Matrix> cc, const std::vector<DimerSubspace_base>& subspace, const int nstates);
    std::shared_ptr<Matrix> compute_intra(const DimerSubspace_base& subspace, std::shared_ptr<const DimerJop> jop, const double diag) const;

    void modelize();

    void print_hamiltonian(const std::string title = "MultiExciton Hamiltonian", const int nstates = 10) const;
    void print_states(const Matrix& cc, const std::vector<double>& energies, const double thresh = 0.01, const std::string title = "Adiabats") const;
    void print_property(const std::string label, std::shared_ptr<const Matrix>, const int size = 10) const ;
    void print(const double thresh = 0.01) const;

  public:
    ASD_base(const std::shared_ptr<const PTree> input, std::shared_ptr<const Dimer> dimer);

    virtual void compute() = 0;
    void compute_rdm();

    std::pair<std::shared_ptr<Matrix>, std::shared_ptr<Matrix>> model(const int i) { return models_[i]; }

    std::vector<double> energy() const { return energies_; }
    double energy(const int i) const { return energies_.at(i); }

};

template<> std::shared_ptr<Matrix> ASD_base::compute_offdiagonal_1e<true>(const std::array<MonomerKey,4>&, std::shared_ptr<const Matrix>) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_inter_2e<true>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_aET<true>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_bET<true>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_abFlip<true>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_abET<true>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_aaET<true>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_bbET<true>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<Matrix> ASD_base::compute_diagonal_block<true>(const DimerSubspace_base& subspace) const;

template<> std::shared_ptr<RDM<2>> ASD_base::compute_offdiagonal_1e<false>(const std::array<MonomerKey,4>&, std::shared_ptr<const Matrix>) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_inter_2e<false>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_aET<false>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_bET<false>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_abFlip<false>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_abET<false>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_aaET<false>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_bbET<false>(const std::array<MonomerKey,4>&) const;
template<> std::shared_ptr<RDM<2>> ASD_base::compute_diagonal_block<false>(const DimerSubspace_base& subspace) const;

namespace {
  template<typename T>
  void transpose_call(std::shared_ptr<T>& o) { assert(false); }
  template<>
  void transpose_call(std::shared_ptr<Matrix>& o) { o = o->transpose(); }
  template<>
  void transpose_call(std::shared_ptr<RDM<2>>& o) { /* doing nothing */ }
}

template <bool _N, typename return_type>
std::shared_ptr<return_type> ASD_base::couple_blocks(const DimerSubspace_base& AB, const DimerSubspace_base& ApBp) const {

  Coupling term_type = coupling_type(AB, ApBp);

  const DimerSubspace_base* space1 = &AB;
  const DimerSubspace_base* space2 = &ApBp;

  bool flip = (static_cast<int>(term_type) < 0);
  if (flip) {
    term_type = Coupling(-1*static_cast<int>(term_type));
    std::swap(space1,space2);
  }

  std::shared_ptr<return_type> out;
  std::array<MonomerKey,4> keys {{space1->template monomerkey<0>(), space1->template monomerkey<1>(), space2->template monomerkey<0>(), space2->template monomerkey<1>()}};

  switch(term_type) {
    case Coupling::none :
      out = nullptr; break;
    case Coupling::diagonal :
      out = compute_inter_2e<_N>(keys); break;
    case Coupling::aET :
      out = compute_aET<_N>(keys); break;
    case Coupling::bET :
      out = compute_bET<_N>(keys); break;
    case Coupling::abFlip :
      out = compute_abFlip<_N>(keys); break;
    case Coupling::abET :
      out = compute_abET<_N>(keys); break;
    case Coupling::aaET :
      out = compute_aaET<_N>(keys); break;
    case Coupling::bbET :
      out = compute_bbET<_N>(keys); break;
    default :
      throw std::logic_error("Asking for a coupling type that has not been written.");
  }

  /* if we are computing the Hamiltonian and flip = true, then we tranpose the output (see above) */
  if (flip) transpose_call(out);
  return out;
}

}

#endif
