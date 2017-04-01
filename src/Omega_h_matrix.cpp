#include "Omega_h_matrix.hpp"
#include "Omega_h_loop.hpp"

namespace Omega_h {

template <Int dim>
Reals repeat_symm(LO n, Matrix<dim, dim> symm) {
  Write<Real> symms(n * symm_ncomps(dim));
  auto f = OMEGA_H_LAMBDA(LO i) { set_symm(symms, i, symm); };
  parallel_for(n, f);
  return symms;
}

template Reals repeat_symm(LO n, Matrix<3, 3> symm);
template Reals repeat_symm(LO n, Matrix<2, 2> symm);

}  // end namespace Omega_h