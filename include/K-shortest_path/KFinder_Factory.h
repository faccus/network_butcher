//
// Created by faccus on 22/04/23.
//

#ifndef NETWORK_BUTCHER_KFINDER_FACTORY_H
#define NETWORK_BUTCHER_KFINDER_FACTORY_H

#include "Factory.h"

#include "KSP_Method.h"

#include "KEppstein.h"
#include "KEppstein_lazy.h"

namespace network_butcher::kfinder
{
  template <typename T, bool Only_Distance = false>
  class KFinder_Factory
  {
  private:
    using Builder_type =
      std::function<std::unique_ptr<KFinder<T, Only_Distance>>(T const &, node_id_type, node_id_type)>;

    using Factory = GenericFactory::Factory<KFinder<T, Only_Distance>, std::string, Builder_type>;

    KFinder_Factory()
    {
      auto &factory = Factory::Instance();

      factory.add("eppstein", [](T const &graph, node_id_type root, node_id_type sink) {
        return std::make_unique<KFinder_Eppstein<T, Only_Distance>>(graph, root, sink);
      });
      factory.add("lazy_eppstein", [](T const &graph, node_id_type root, node_id_type sink) {
        return std::make_unique<KFinder_Lazy_Eppstein<T, Only_Distance>>(graph, root, sink);
      });
    };

  public:
    static KFinder_Factory &
    Instance();

    void
    add(std::string entry_name, Builder_type const &func)
    {
      Factory::Instance().add(entry_name, func);
    }

    std::unique_ptr<KFinder<T, Only_Distance>>
    create(std::string const &name, T const &graph, node_id_type root, node_id_type sink) const
    {
      return Factory::Instance().create(name, graph, root, sink);
    }

    std::unique_ptr<KFinder<T, Only_Distance>>
    create(parameters::KSP_Method method, T const &graph, node_id_type root, node_id_type sink) const
    {
      switch (method)
        {
          case parameters::KSP_Method::Eppstein:
            return create("eppstein", graph, root, sink);
          case parameters::KSP_Method::Lazy_Eppstein:
            return create("lazy_eppstein", graph, root, sink);
          default:
            throw std::logic_error("The specified KSP_Method is not implemented.");
        }
    }

    [[nodiscard]] std::vector<std::string>
    registered() const
    {
      return Factory::Instance().registered();
    }

    [[nodiscard]] bool
    registered(std::string const &name) const
    {
      return Factory::Instance().registered(name);
    }

    void
    unregister(std::string const &name)
    {
      Factory::Instance().unregister(name);
    }
  };


  template <typename T, bool Only_Distance>
  KFinder_Factory<T, Only_Distance> &
  KFinder_Factory<T, Only_Distance>::Instance()
  {
    static KFinder_Factory<T, Only_Distance> factory;
    return factory;
  }

} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_KFINDER_FACTORY_H