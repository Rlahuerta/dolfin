// Auto generated SWIG file for Python interface of DOLFIN
//
// Copyright (C) 2012 Kristian B. Oelgaard
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

// Autogenerated docstrings file, extracted from the DOLFIN source C++ files.

// Documentation extracted from: (module=graph, header=Graph.h)
// Documentation extracted from: (module=graph, header=GraphBuilder.h)
%feature("docstring")  dolfin::GraphBuilder "
This class builds a Graph corresponding to various objects
";

%feature("docstring")  dolfin::GraphBuilder::local_graph "
**Overloaded versions**

* local_graph\ (mesh, dofmap0, dofmap1)

  Build local graph from dofmap

* local_graph\ (mesh, coloring_type)

  Build local graph from mesh (general version)
";

%feature("docstring")  dolfin::GraphBuilder::compute_dual_graph "
Build distributed dual graph for mesh
";

// Documentation extracted from: (module=graph, header=BoostGraphOrdering.h)
%feature("docstring")  dolfin::BoostGraphOrdering "
This class computes graph re-orderings. It uses Boost Graph.
";

%feature("docstring")  dolfin::BoostGraphOrdering::compute_cuthill_mckee "
Compute re-ordering (map[old] -> new) using Cuthill-McKee algorithm
";

%feature("docstring")  dolfin::BoostGraphOrdering::compute_king "
**Overloaded versions**

* compute_king\ (graph)

  Compute re-ordering (map[old] -> new) using King algorithm

* compute_king\ (graph)

  Compute re-ordering (map[old] -> new) using King algorithm
";

%feature("docstring")  dolfin::BoostGraphOrdering::compute_minimum_degree "
Compute re-ordering (map[old] -> new) using minimum degree algorithm
";

// Documentation extracted from: (module=graph, header=SCOTCH.h)
%feature("docstring")  dolfin::SCOTCH "
This class proivdes an interface to SCOTCH-PT (parallel version)
";

