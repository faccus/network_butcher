//
// Created by faccus on 12/20/22.
//

#ifndef NETWORK_BUTCHER_COMPUTER_FLOPS_H
#define NETWORK_BUTCHER_COMPUTER_FLOPS_H

#include "Factory.h"
#include "WGraph.h"
#include "Hardware_traits.h"

namespace network_butcher_computer
{
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
    static bool factory_initialized;

    using FactoryType = GenericFactory::FunctionFactory<
      double,
      std::string,
      std::function<double(
        const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &)>>;

    static void
    generate_maps_flops_func();

    static FactoryType &
    get_factory();

    static std::size_t
    get_volume(std::vector<network_butcher_types::DynamicType> const &);

  public:
    static std::pair<double, std::size_t>
    compute_macs_flops(Content_Type<type_info_pointer> const &content);
  };
} // namespace network_butcher_computer

#endif // NETWORK_BUTCHER_COMPUTER_FLOPS_H
