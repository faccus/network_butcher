#ifndef NETWORK_BUTCHER_GRAPH_DEV_TRAITS
#define NETWORK_BUTCHER_GRAPH_DEV_TRAITS

#include "Basic_traits.h"

namespace network_butcher
{
  namespace types
  {
    template <typename T>
    using Node_Type = Node<T>;

    template <typename T>
    using Node_Collection_Type = std::vector<Node_Type<T>>;

    using Dependencies_Type = std::vector<std::pair<node_id_collection_type, node_id_collection_type>>;
  } // namespace types

} // namespace network_butcher

#endif