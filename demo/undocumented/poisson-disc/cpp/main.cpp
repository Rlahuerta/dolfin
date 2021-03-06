// Copyright (C) 2006-2015 Anders Logg, Martin Sandve Alnæs
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
// This demo program solves Poisson's equation
//
//     - div grad u(x, y) = f(x, y)
//
// on the unit disk with source f given by
//
//     f(x, y) = 1.0
//
// and boundary conditions given by
//
//     u(x, y) = 0        for x^2 + y^2 = 1

#include <dolfin.h>
#include "PoissonDisc.h"

using namespace dolfin;

// Source term (right-hand side)
class Source : public Expression
{
  void eval(Array<double>& values, const Array<double>& x) const
  {
    values[0] = 1.0;
  }
};

// Sub domain for Dirichlet boundary condition
class DirichletBoundary : public SubDomain
{
  bool inside(const Array<double>& x, bool on_boundary) const
  {
    return on_boundary;
  }
};

int main()
{
  // Create mesh and function space
  int degree = 2;
  int gdim = 2;
  UnitDiscMesh mesh(MPI_COMM_WORLD, 32, degree, gdim);
  //UnitSquareMesh mesh(32, 32);

  PoissonDisc::FunctionSpace V(mesh);

  // Define boundary condition
  Constant u0(0.0);
  DirichletBoundary boundary;
  DirichletBC bc(V, u0, boundary);

  // Define variational forms
  PoissonDisc::BilinearForm a(V, V);
  PoissonDisc::LinearForm L(V);

  Source f;
  L.f = f;

  // Compute solution
  Function u(V);
  solve(a == L, u, bc);

  // Error norm functional
  PoissonDisc::Functional M(mesh);
  M.uh = u;
  double uerror = assemble(M);
  std::cout << uerror << std::endl;

  // Save solution in VTK format
  //File file("poisson.pvd");
  //file << u;

  // Plot solution
  //plot(u);
  //interactive();

  return 0;
}
