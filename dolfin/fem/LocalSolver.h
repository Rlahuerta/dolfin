// Copyright (C) 2013-2015 Garth N. Wells
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Steven Vandekerckhove, 2014.

#ifndef __LOCAL_SOLVER_H
#define __LOCAL_SOLVER_H

#include <memory>
#include <vector>
#include <Eigen/Dense>

namespace dolfin
{

  /// This class solves problems cell-wise. It computes the local
  /// left-hand side A_local and the local right-hand side b_local
  /// (for one cell), and solves A_local x_local = b_local. The result
  /// x_local is copied into the global vector x. The operator A_local
  /// must be square.
  ///
  /// For forms with no coupling across cell edges, this function is
  /// identical to a global solve. For problems with coupling across
  /// cells it is not.
  ///
  /// This class can be used for post-processing solutions, e.g. computing
  /// stress fields for visualisation, far more cheaply that using
  /// global projections.

  // Forward declarations
  class Form;
  class GenericVector;

  class LocalSolver
  {
  public:

    /// Constructor (shared pointer version)
    LocalSolver(std::shared_ptr<const Form> a,
                std::shared_ptr<const Form> L, bool SPD=false);

    /// Constructor
    //LocalSolver(std::shared_ptr<const Form> a);

    /// Solve local (cell-wise) problems A_e x_e = b_e, where A_e is
    /// the cell matrix LHS and b_e is the global RHS vector b
    /// restricted to the cell, i.e. b_e may contain contributions
    /// from neighbouring cells. The solution is exact for the case in
    /// which there is no coupling between cell contributions to the
    /// global matrix A, e.g. the discontinuous Galerkin matrix. The
    /// result is copied into x.
    void solve_global_rhs(GenericVector& x);

    /// Solve local (cell-wise) problems A_e x_e = b_e where A_e and
    /// b_e are the cell element tensors. This function is useful for
    /// computing (approximate) cell-wise projections, for example for
    /// post-processing. It much more efficient than computing global
    /// projections.
    void solve_local_rhs(GenericVector& x);

    /// Solve local (cell-wise) problem and copy result into global
    /// vector x, reusing factorizations of local matrices.
    //void solve(GenericVector& x, const GenericVector& b) const;

    /// Solve local (cell-wise) problem and copy result into global
    /// vector x.
    /*
    void solve(GenericVector& x, const Form& a, const Form& L,
               bool symmetric=false) const;
    */

    // Factorise LHS for all cells and store
    void factorize();

    /// Cache the LU factorisation for local matrices for re-use
    //void cache_factorization(bool cache);

    /// Reset (clear) any stored factorisations
    void clear_factorization();

  private:

    // Check forms
    void check_forms() const;

    // Bilinear and linear forms
    std::shared_ptr<const Form> _a, _L;

    const bool _spd;

    // Cached LU factorisations of matrices
    std::vector<Eigen::PartialPivLU<Eigen::Matrix<double, Eigen::Dynamic,
                                                  Eigen::Dynamic,
                                                  Eigen::RowMajor>>> _lu_cache;

    // Cached Cholesky factorisations of matrices
    std::vector<Eigen::LLT<Eigen::Matrix<double, Eigen::Dynamic,
                                         Eigen::Dynamic,
                                         Eigen::RowMajor>>> _cholesky_cache;
  };

}

#endif
