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
  template <typename T>
  class KFinder_Factory
  {
  private:
    using Builder_type = std::function<std::unique_ptr<KFinder<T>>(T const &)>;

    using Factory = GenericFactory::Factory<KFinder<T>, std::string, Builder_type>;

    KFinder_Factory()
    {
      auto &factory = Factory::Instance();

      factory.add("eppstein", [](T const &graph) { return std::make_unique<KFinder_Eppstein<T>>(graph); });
      factory.add("lazy_eppstein", [](T const &graph) { return std::make_unique<KFinder_Lazy_Eppstein<T>>(graph); });
    };

  public:
    static KFinder_Factory &
    Instance();

    void
    add(std::string entry_name, Builder_type const &func)
    {
      Factory::Instance().add(entry_name, func);
    }

    std::unique_ptr<KFinder<T>>
    create(std::string const &name, T const &graph) const
    {
      return Factory::Instance().create(name, graph);
    }

    std::unique_ptr<KFinder<T>>
    create(parameters::KSP_Method method, T const &graph) const
    {
      switch (method)
        {
          case parameters::KSP_Method::Eppstein:
            return create("eppstein", graph);
          case parameters::KSP_Method::Lazy_Eppstein:
            return create("lazy_eppstein", graph);
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


  template <typename T>
  KFinder_Factory<T> &
  KFinder_Factory<T>::Instance()
  {
    static KFinder_Factory<T> factory;
    return factory;
  }

} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_KFINDER_FACTORY_H