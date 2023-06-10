#ifndef NETWORK_BUTCHER_NODE_TRAITS_H
#define NETWORK_BUTCHER_NODE_TRAITS_H

#include <memory>

#include "content.h"
#include "node.h"
#include "type_info.h"

namespace network_butcher
{
  /// Node of the graph that will contain the result of the conversion from the Onnx graph
  using Onnx_Converted_Node_Type = network_butcher::types::CNode<network_butcher::types::Content<Type_Info_Pointer>>;

  /// Node of the block graph
  using Block_Graph_Node_Type =
    network_butcher::types::CNode<std::pair<std::size_t, std::shared_ptr<Node_Id_Collection_Type>>>;
} // namespace network_butcher


#endif // NETWORK_BUTCHER_NODE_TRAITS_H
