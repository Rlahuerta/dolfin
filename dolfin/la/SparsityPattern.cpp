// Copyright (C) 2007-2011 Garth N. Wells
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
// Modified by Magnus Vikstrom, 2008.
// Modified by Anders Logg, 2008-2009.
// Modified by Ola Skavhaug, 2009.
//
// First added:  2007-03-13
// Last changed: 2014-11-26

#include <algorithm>

#include <dolfin/common/MPI.h>
#include <dolfin/log/log.h>
#include <dolfin/log/LogStream.h>
#include <dolfin/la/IndexMap.h>
#include "SparsityPattern.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
SparsityPattern::SparsityPattern(std::size_t primary_dim)
  : GenericSparsityPattern(primary_dim), _mpi_comm(MPI_COMM_NULL)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
SparsityPattern::SparsityPattern(
  const MPI_Comm mpi_comm,
  const std::vector<std::size_t>& dims,
  const std::vector<std::shared_ptr<const IndexMap>> index_maps,
  std::size_t primary_dim)
  : GenericSparsityPattern(primary_dim), _mpi_comm(MPI_COMM_NULL)
{
  init(mpi_comm, dims, index_maps);
}
//-----------------------------------------------------------------------------
void SparsityPattern::init(
  const MPI_Comm mpi_comm,
  const std::vector<std::size_t>& dims,
  const std::vector<std::shared_ptr<const IndexMap>> index_maps)
{
  // Only rank 2 sparsity patterns are supported
  dolfin_assert(dims.size() == 2);
  dolfin_assert(index_maps.size() == 2);

  _mpi_comm = mpi_comm;
  _index_maps = index_maps;

  const std::size_t _primary_dim = primary_dim();

  // Clear sparsity pattern data
  diagonal.clear();
  off_diagonal.clear();
  non_local.clear();

  // Check that primary dimension is valid
  if (_primary_dim > 1)
   {
    dolfin_error("SparsityPattern.cpp",
                 "primary dimension for sparsity pattern storage",
                 "Primary dimension must be less than 2 (0=row major, 1=column major");
  }

  const std::size_t local_size = index_maps[_primary_dim]->size();

  // Resize diagonal block
  diagonal.resize(local_size);

  // Resize off-diagonal block (only needed when local range != global
  // range)
  off_diagonal.resize(local_size);
}
//-----------------------------------------------------------------------------
void SparsityPattern::insert_global(dolfin::la_index i, dolfin::la_index j)
{
  dolfin::la_index i_index = i;
  dolfin::la_index j_index = j;

  const std::size_t _primary_dim = primary_dim();

  if (_primary_dim != 0)
  {
    i_index = j;
    j_index = i;
  }

  // Check local range
  if (MPI::size(_mpi_comm) == 1)
  {
    // Sequential mode, do simple insertion
    diagonal[i_index].insert(j_index);
  }
  else
  {
    const std::pair<dolfin::la_index, dolfin::la_index>
      local_range0 = _index_maps[_primary_dim]->local_range();
    const std::pair<dolfin::la_index, dolfin::la_index>
      local_range1 = _index_maps[1 - _primary_dim]->local_range();

    if (local_range0.first <= i_index && i_index < local_range0.second)
      {
        // Subtract offset
        const std::size_t I = i_index - local_range0.first;

        // Store local entry in diagonal or off-diagonal block
        if (local_range1.first <= j_index && j_index < local_range1.second)
        {
          dolfin_assert(I < diagonal.size());
          diagonal[I].insert(j_index);
        }
        else
        {
          dolfin_assert(I < off_diagonal.size());
          off_diagonal[I].insert(j_index);
        }
      }
    else
    {
      dolfin_error("SparsityPattern.cpp",
                   "insert using global indices",
                   "Index must be in the process range");
    }
  }
}
//-----------------------------------------------------------------------------
void SparsityPattern::insert_global(
  const std::vector<ArrayView<const dolfin::la_index>>& entries)
{
  dolfin_assert(entries.size() == 2);

  const std::size_t _primary_dim = primary_dim();

  ArrayView<const dolfin::la_index> map_i;
  ArrayView<const dolfin::la_index> map_j;
  std::size_t primary_codim;
  dolfin_assert(_primary_dim < 2);
  if (_primary_dim == 0)
  {
    primary_codim = 1;
    map_i = entries[0];
    map_j = entries[1];
  }
  else
  {
    primary_codim = 0;
    map_i = entries[1];
    map_j = entries[0];
  }

  dolfin_assert(_primary_dim < _index_maps.size());
  dolfin_assert(primary_codim < _index_maps.size());
  const std::pair<dolfin::la_index, dolfin::la_index>
    local_range0 = _index_maps[_primary_dim]->local_range();
  const std::pair<dolfin::la_index, dolfin::la_index>
    local_range1 = _index_maps[primary_codim]->local_range();

  // Check local range
  if (MPI::size(_mpi_comm) == 1)
  {
    // Sequential mode, do simple insertion
    for (const auto &i_index : map_i)
    {
      dolfin_assert((int) i_index < (int) diagonal.size());
      diagonal[i_index].insert(map_j.begin(), map_j.end());
    }
  }
  else
  {
    // Parallel mode, use either diagonal, off_diagonal or non_local
    for (const auto &i_index : map_i)
    {
      if (local_range0.first <= i_index && i_index < local_range0.second)
      {
        // Subtract offset
        const std::size_t I = i_index - local_range0.first;

        // Store local entry in diagonal or off-diagonal block
        for (const auto &j_index : map_j)
        {
          if (local_range1.first <= j_index && j_index < local_range1.second)
          {
            dolfin_assert(I < diagonal.size());
            diagonal[I].insert(j_index);
          }
          else
          {
            dolfin_assert(I < off_diagonal.size());
            off_diagonal[I].insert(j_index);
          }
        }
      }
      else
      {
        dolfin_error("SparsityPattern.cpp",
                     "insert using global indices",
                     "Index must be in the process range");
      }
    }
  }
}
//-----------------------------------------------------------------------------
void SparsityPattern::insert_local(
  const std::vector<ArrayView<const dolfin::la_index>>& entries)
{
  dolfin_assert(entries.size() == 2);
  const std::size_t _primary_dim = primary_dim();

  ArrayView<const dolfin::la_index> map_i;
  ArrayView<const dolfin::la_index> map_j;
  std::size_t primary_codim;
  dolfin_assert(_primary_dim < 2);
  if (_primary_dim == 0)
  {
    primary_codim = 1;
    map_i = entries[0];
    map_j = entries[1];
  }
  else
  {
    primary_codim = 0;
    map_i = entries[1];
    map_j = entries[0];
  }

  std::shared_ptr<const IndexMap> index_map0 = _index_maps[ _primary_dim];
  std::shared_ptr<const IndexMap> index_map1 = _index_maps[primary_codim];
  const la_index local_size0 = index_map0->size();
  const la_index local_size1 = index_map1->size();

  // Check local range
  if (MPI::size(_mpi_comm) == 1)
  {
    // Sequential mode, do simple insertion
    for (const auto &i_index : map_i)
      diagonal[i_index].insert(map_j.begin(), map_j.end());
  }
  else
  {
    // Parallel mode, use either diagonal, off_diagonal or non_local

    for (const auto &i_index : map_i)
    {
      if (i_index < local_size0)
      {
        // Store local entry in diagonal or off-diagonal block
        for (const auto &j_index : map_j)
        {
          const std::size_t J = index_map1->local_to_global(j_index);
          if (j_index < local_size1)
          {
            dolfin_assert(i_index < (int)diagonal.size());
            diagonal[i_index].insert(J);
          }
          else
          {
            dolfin_assert(i_index < (int)off_diagonal.size());
            off_diagonal[i_index].insert(J);
          }
        }
      }
      else
      {
        // Store non-local entry (communicated later during apply())
        for (const auto &j_index : map_j)
        {
          const std::size_t J = index_map1->local_to_global(j_index);
          // Store indices
          non_local.push_back(i_index);
          non_local.push_back(J);
        }
      }
    }
  }
}
//-----------------------------------------------------------------------------
std::size_t SparsityPattern::rank() const
{
  return 2;
}
//-----------------------------------------------------------------------------
std::pair<std::size_t, std::size_t>
  SparsityPattern::local_range(std::size_t dim) const
{
  dolfin_assert(dim < 2);
  return _index_maps[dim]->local_range();
}
//-----------------------------------------------------------------------------
std::size_t SparsityPattern::num_nonzeros() const
{
  std::size_t nz = 0;
  for (auto slice = diagonal.begin(); slice != diagonal.end(); ++slice)
    nz += slice->size();
  for (auto slice = off_diagonal.begin(); slice != off_diagonal.end(); ++slice)
    nz += slice->size();

  return nz;
}
//-----------------------------------------------------------------------------
void SparsityPattern::num_nonzeros_diagonal(std::vector<std::size_t>& num_nonzeros) const
{
  // Resize vector
  num_nonzeros.resize(diagonal.size());

  // Get number of nonzeros per generalised row
  for (auto slice = diagonal.begin(); slice != diagonal.end(); ++slice)
    num_nonzeros[slice - diagonal.begin()] = slice->size();
}
//-----------------------------------------------------------------------------
void SparsityPattern::num_nonzeros_off_diagonal(std::vector<std::size_t>& num_nonzeros) const
{
  // Resize vector
  num_nonzeros.resize(off_diagonal.size());

  // Compute number of nonzeros per generalised row
  for (auto slice = off_diagonal.begin(); slice != off_diagonal.end(); ++slice)
    num_nonzeros[slice - off_diagonal.begin()] = slice->size();
}
//-----------------------------------------------------------------------------
void SparsityPattern::num_local_nonzeros(std::vector<std::size_t>& num_nonzeros) const
{
  num_nonzeros_diagonal(num_nonzeros);
  if (!off_diagonal.empty())
  {
    std::vector<std::size_t> tmp;
    num_nonzeros_off_diagonal(tmp);
    dolfin_assert(num_nonzeros.size() == tmp.size());
    std::transform(num_nonzeros.begin(), num_nonzeros.end(), tmp.begin(),
                   num_nonzeros.begin(), std::plus<std::size_t>());
  }
}
//-----------------------------------------------------------------------------
void SparsityPattern::apply()
{
  const std::size_t _primary_dim = primary_dim();

  std::size_t primary_codim;
  dolfin_assert(_primary_dim < 2);
  if (_primary_dim == 0)
    primary_codim = 1;
  else
    primary_codim = 0;

  const std::pair<dolfin::la_index, dolfin::la_index>
    local_range0 = _index_maps[_primary_dim]->local_range();
  const std::pair<dolfin::la_index, dolfin::la_index>
    local_range1 = _index_maps[primary_codim]->local_range();
  const std::size_t local_size0 = _index_maps[_primary_dim]->size();
  const std::size_t offset0 = local_range0.first;

  const std::size_t num_processes = MPI::size(_mpi_comm);
  const std::size_t proc_number = MPI::rank(_mpi_comm);

  // Print some useful information
  if (get_log_level() <= DBG)
    info_statistics();

  // Communicate non-local blocks if any
  if (MPI::size(_mpi_comm) > 1)
  {
    // Figure out correct process for each non-local entry
    dolfin_assert(non_local.size() % 2 == 0);
    std::vector<std::vector<std::size_t>> non_local_send(num_processes);

    const std::vector<int>& off_process_owner
      = _index_maps[_primary_dim]->off_process_owner();

    const std::vector<std::size_t>& local_to_global
      = _index_maps[_primary_dim]->local_to_global_unowned();

    std::size_t dim_block_size = _index_maps[_primary_dim]->block_size();
    for (std::size_t i = 0; i < non_local.size(); i += 2)
    {
      // Get local indices of off-process dofs
      const std::size_t i_index = non_local[i];
      const std::size_t J = non_local[i + 1];

      // Figure out which process owns the row
      dolfin_assert(i_index >= local_size0);
      const std::size_t i_offset = (i_index - local_size0)/dim_block_size;
      dolfin_assert(i_offset < off_process_owner.size());
      const std::size_t p = off_process_owner[i_offset];

      dolfin_assert(p < num_processes);
      dolfin_assert(p != proc_number);

      // Get global I index
      la_index I = 0;
      if (i_index < local_size0)
        I = i_index + offset0;
      else
      {
        std::size_t tmp = i_index - local_size0;
        const std::div_t div = std::div((int) tmp, (int) dim_block_size);
        const int i_node = div.quot;
        const int i_component = div.rem;

        const std::size_t I_node = local_to_global[i_node];
        I = dim_block_size*I_node + i_component;
      }

      // Buffer local/global index pair to send
      non_local_send[p].push_back(I);
      non_local_send[p].push_back(J);
    }

    // Communicate non-local entries to other processes
    std::vector<std::vector<std::size_t>> non_local_received;
    MPI::all_to_all(_mpi_comm, non_local_send, non_local_received);

    // Insert non-local entries received from other processes
    for (std::size_t p = 0; p < num_processes; ++p)
    {
      const std::vector<std::size_t>& non_local_received_p
        = non_local_received[p];
      dolfin_assert(non_local_received_p.size() % 2 == 0);

      for (std::size_t i = 0; i < non_local_received_p.size(); i += 2)
      {
        // Get global row and column
        const dolfin::la_index I = non_local_received_p[i];
        const dolfin::la_index J = non_local_received_p[i + 1];

        // Sanity check
        if (I < local_range0.first
            || I >= local_range0.second)
        {
          dolfin_error("SparsityPattern.cpp",
                       "apply changes to sparsity pattern",
                       "Received illegal sparsity pattern entry for row/column %d, not in range [%d, %d]",
                       I, local_range0.first,
                       local_range0.second);
        }

        // Get local I index
        const std::size_t i_index = I - offset0;

        // Insert in diagonal or off-diagonal block
        if (local_range1.first <= J &&
            J < local_range1.second)
        {
          dolfin_assert(i_index < diagonal.size());
          diagonal[i_index].insert(J);
        }
        else
        {
          dolfin_assert(i_index < off_diagonal.size());
          off_diagonal[i_index].insert(J);
        }
      }
    }
  }

  // Clear non-local entries
  non_local.clear();
}
//-----------------------------------------------------------------------------
std::string SparsityPattern::str(bool verbose) const
{
  // Print each row
  std::stringstream s;
  for (std::size_t i = 0; i < diagonal.size(); i++)
  {
    if (primary_dim() == 0)
      s << "Row " << i << ":";
    else
      s << "Col " << i << ":";

    for (auto entry = diagonal[i].begin(); entry != diagonal[i].end(); ++entry)
      s << " " << *entry;
    s << std::endl;
  }

  return s.str();
}
//-----------------------------------------------------------------------------
std::vector<std::vector<std::size_t>>
SparsityPattern::diagonal_pattern(Type type) const
{
  std::vector<std::vector<std::size_t>> v(diagonal.size());
  for (std::size_t i = 0; i < diagonal.size(); ++i)
    v[i].insert(v[i].begin(), diagonal[i].begin(), diagonal[i].end());

  if (type == sorted)
  {
    for (std::size_t i = 0; i < v.size(); ++i)
      std::sort(v[i].begin(), v[i].end());
  }

  return v;
}
//-----------------------------------------------------------------------------
std::vector<std::vector<std::size_t>>
  SparsityPattern::off_diagonal_pattern(Type type) const
{
  std::vector<std::vector<std::size_t>> v(off_diagonal.size());
  for (std::size_t i = 0; i < off_diagonal.size(); ++i)
    v[i].insert(v[i].begin(), off_diagonal[i].begin(), off_diagonal[i].end());

  if (type == sorted)
  {
    for (std::size_t i = 0; i < v.size(); ++i)
      std::sort(v[i].begin(), v[i].end());
  }

  return v;
}
//-----------------------------------------------------------------------------
void SparsityPattern::info_statistics() const
{
  // Count nonzeros in diagonal block
  std::size_t num_nonzeros_diagonal = 0;
  for (std::size_t i = 0; i < diagonal.size(); ++i)
    num_nonzeros_diagonal += diagonal[i].size();

  // Count nonzeros in off-diagonal block
  std::size_t num_nonzeros_off_diagonal = 0;
  for (std::size_t i = 0; i < off_diagonal.size(); ++i)
    num_nonzeros_off_diagonal += off_diagonal[i].size();

  // Count nonzeros in non-local block
  const std::size_t num_nonzeros_non_local = non_local.size()/2;

  // Count total number of nonzeros
  const std::size_t num_nonzeros_total = num_nonzeros_diagonal
    + num_nonzeros_off_diagonal + num_nonzeros_non_local;

  std::size_t size0 = _index_maps[0]->size_global();
  std::size_t size1 = _index_maps[1]->size_global();

  // Return number of entries
  cout << "Matrix of size " << size0 << " x " << size1 << " has "
       << num_nonzeros_total << " (" << 100.0*num_nonzeros_total/(size0*size1)
        << "%)" << " nonzero entries." << endl;
  if (num_nonzeros_total != num_nonzeros_diagonal)
  {
    cout << "Diagonal: " << num_nonzeros_diagonal << " ("
         << (100.0*static_cast<double>(num_nonzeros_diagonal) / static_cast<double>(num_nonzeros_total))
         << "%), ";
    cout << "off-diagonal: " << num_nonzeros_off_diagonal << " ("
         << (100.0*static_cast<double>(num_nonzeros_off_diagonal)/static_cast<double>(num_nonzeros_total))
         << "%), ";
    cout << "non-local: " << num_nonzeros_non_local << " ("
         << (100.0*static_cast<double>(num_nonzeros_non_local)/static_cast<double>(num_nonzeros_total))
         << "%)";
    cout << endl;
  }

  MPI::barrier(MPI_COMM_WORLD);
}
//-----------------------------------------------------------------------------
