#ifndef SUBTREE_EXTRACTION_IMPL_HPP
#define SUBTREE_EXTRACTION_IMPL_HPP

#include <terraces/subtree_extraction.hpp>

namespace terraces {

std::pair<bitmatrix, std::vector<index_t>> compute_node_occ(const tree& t, const bitmatrix& occ);

index_t induced_lca(const tree& t, const bitmatrix& node_occ, index_t column);

tree subtree(const tree& t, const bitmatrix& node_occ,
             const std::vector<index_t>& num_leaves_per_site, index_t site);

} // namespace terraces

#endif // SUBTREE_EXTRACTION_IMPL_HPP
