//
// Created by faccus on 12/20/22.
//

#ifndef NETWORK_BUTCHER_COMPUTER_FLOPS_H
#define NETWORK_BUTCHER_COMPUTER_FLOPS_H

#include "../Network/WGraph.h"
#include "../Traits/Hardware_traits.h"

namespace network_butcher_computer {
  class Computer_flops
  {
  public:
    template <class T>
    using Standard_Graph_Type = network_butcher_types::Graph<T>;
    template <class T>
    using Node_Type = network_butcher_types::Node<T>;

    template <class T>
    using Content_Type = network_butcher_types::Content<T>;

    template <class T>
    using Contented_Graph_Type = network_butcher_types::Graph<Content_Type<T>>;
    template <class T>
    using Content_Node_Type = network_butcher_types::Node<network_butcher_types::Content<T>>;

  private:

    static std::unordered_map<std::string, std::function<std::pair<double, std::size_t>(Content_Type<type_info_pointer> const &)>> generate_maps_flops_func();
    static std::size_t get_volume(std::vector<network_butcher_types::DynamicType> const &);

  public:

    static std::pair<double, std::size_t> compute_macs_flops(Content_Type<type_info_pointer> const &content);
  };
}

#endif // NETWORK_BUTCHER_COMPUTER_FLOPS_H
