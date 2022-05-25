#include "Omega_h_curve_coarsen.hpp"

#include "Omega_h_curve_validity_3d.hpp"

namespace Omega_h {

void correct_curved_edges(Mesh *new_mesh) {

  auto const edge_crvtoBdryFace = new_mesh->get_array<I8>(1, "edge_crvtoBdryFace");
  auto const new_rv2v = new_mesh->ask_down(3, 0).ab2b;
  auto const new_re2e = new_mesh->ask_down(3, 1).ab2b;
  auto const new_rf2f = new_mesh->ask_down(3, 2).ab2b;
  auto const new_ev2v = new_mesh->ask_down(1, 0).ab2b;
  auto const new_e2r = new_mesh->ask_up(1, 3);
  auto const new_e2er = new_e2r.a2ab;
  auto const new_er2r = new_e2r.ab2b;
  auto const nnew_edge = new_mesh->nedges();
  auto const n_edge_pts = new_mesh->n_internal_ctrlPts(1);
  auto const dim = new_mesh->dim();
  auto const new_coords = new_mesh->coords();

  auto const vertCtrlPts = new_mesh->get_ctrlPts(0);
  auto const edgeCtrlPts = new_mesh->get_ctrlPts(1);
  auto const faceCtrlPts = new_mesh->get_ctrlPts(2);
  Write<Real> edge_ctrlPts_new(nnew_edge*n_edge_pts*dim, INT8_MAX);

  auto edge_correct = OMEGA_H_LAMBDA(LO i) {
    LO has_invalid_tet = -1;
    if (edge_crvtoBdryFace[i] == 1) {
      for (LO er = new_e2er[i]; er < new_e2er[i+1]; ++er) {
        if (has_invalid_tet > 0) break;
        LO adj_tet = new_er2r[er];
        Few<Real, 60> tet_pts = collect_tet_pts(3, adj_tet, new_ev2v, new_rv2v, vertCtrlPts
            , edgeCtrlPts, faceCtrlPts, new_re2e, new_rf2f);

        Few<Real, 84> nodes_det = getTetJacDetNodes<84>(3, tet_pts);

        has_invalid_tet = checkMinJacDet_3d(nodes_det, n_edge_pts+1);
        if (has_invalid_tet > 0) {
          printf(" correcting invalid tet %d for bdry edge %d\n", adj_tet, i);
          auto v0 = new_ev2v[i*2 + 0];
          auto v1 = new_ev2v[i*2 + 1];
          for (LO j=0; j<dim; ++j) {
            edge_ctrlPts_new[i*n_edge_pts*dim + j] = new_coords[v0*dim + j] +
              (new_coords[v1*dim + j] - new_coords[v0*dim + j])*(1.0/3.0);
            edge_ctrlPts_new[i*n_edge_pts*dim + dim + j] = new_coords[v0*dim + j] +
              (new_coords[v1*dim + j] - new_coords[v0*dim + j])*(2.0/3.0);
          }
        }
      }
    }
    else {
      for (LO j = 0; j<n_edge_pts*dim; ++j) {
        edge_ctrlPts_new[i*n_edge_pts*dim + j] = edgeCtrlPts[i*n_edge_pts*dim + j];
      }
    }
  };
  parallel_for(nnew_edge, std::move(edge_correct), "edge_correct");
  new_mesh->set_tag_for_ctrlPts(1, Reals(edge_ctrlPts_new));
  //pfor over all edges
  //if it was curved before, check validity of all its adjacent tets
  //  if any adjacent tet is invalid, straighten this edge
  //else keep pts as is
  return;
}
void check_validity_new_curved_edges(Mesh *new_mesh) {

  auto const new_rv2v = new_mesh->ask_down(3, 0).ab2b;
  auto const new_re2e = new_mesh->ask_down(3, 1).ab2b;
  auto const new_rf2f = new_mesh->ask_down(3, 2).ab2b;
  auto const new_ev2v = new_mesh->ask_down(1, 0).ab2b;
  auto const new_e2r = new_mesh->ask_up(1, 3);
  auto const new_e2er = new_e2r.a2ab;
  auto const new_er2r = new_e2r.ab2b;
  auto const nnew_edge = new_mesh->nedges();
  auto const n_edge_pts = new_mesh->n_internal_ctrlPts(1);
  auto const new_coords = new_mesh->coords();

  auto const vertCtrlPts = new_mesh->get_ctrlPts(0);
  auto const edgeCtrlPts = new_mesh->get_ctrlPts(1);
  auto const faceCtrlPts = new_mesh->get_ctrlPts(2);
  //Write<Real> edge_ctrlPts_new(nnew_edge*n_edge_pts*dim, INT8_MAX);
  auto const edge_dualCone = new_mesh->get_array<I8>(1, "edge_dualCone");

  auto edge_correct = OMEGA_H_LAMBDA(LO i) {
    LO has_invalid_tet = -1;
    if (edge_dualCone[i] == 1) {
      for (LO er = new_e2er[i]; er < new_e2er[i+1]; ++er) {
        LO adj_tet = new_er2r[er];
        Few<Real, 60> tet_pts = collect_tet_pts(3, adj_tet, new_ev2v, new_rv2v, vertCtrlPts
            , edgeCtrlPts, faceCtrlPts, new_re2e, new_rf2f);

        Few<Real, 84> nodes_det = getTetJacDetNodes<84>(3, tet_pts);

        has_invalid_tet = checkMinJacDet_3d(nodes_det, n_edge_pts+1);
        if (has_invalid_tet > 0) {
          printf(" edge %d creates invalid tet with code %d\n", i, has_invalid_tet);
          break;
        }
      }
    }
  };
  parallel_for(nnew_edge, std::move(edge_correct), "edge_correct");
  //new_mesh->set_tag_for_ctrlPts(1, Reals(edge_ctrlPts_new));
  //pfor over all edges
  //check validity of all its adjacent tets
  return;
}


} // namespace Omega_h
