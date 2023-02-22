//
// Created by faccus on 12/20/22.
//

#ifndef NETWORK_BUTCHER_COMPUTER_FLOPS_H
#define NETWORK_BUTCHER_COMPUTER_FLOPS_H

#include "Factory.h"
#include "Hardware_traits.h"
#include "General_Manager.h"

namespace network_butcher_computer
{
  class Computer_flops
  {

    /*static bool factory_initialized;

    using FactoryType = GenericFactory::FunctionFactory<
      double,
      std::string,
      std::function<double(
        const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &)>>;

    static void
    generate_maps_flops_func();

    static FactoryType &
    get_factory();*/
  private:

    static std::size_t
    get_volume(std::vector<network_butcher_types::DynamicType> const &);

  public:

    static std::pair<double, std::size_t>
    compute_macs_flops(network_butcher_types::Content<type_info_pointer> const &);
  };
} // namespace network_butcher_computer

#endif // NETWORK_BUTCHER_COMPUTER_FLOPS_H
