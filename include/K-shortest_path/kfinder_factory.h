//
// Created by faccus on 22/04/23.
//

#ifndef NETWORK_BUTCHER_KFINDER_FACTORY_H
#define NETWORK_BUTCHER_KFINDER_FACTORY_H

#include "Factory.h"

#include "ksp_method.h"

#include "keppstein.h"
#include "keppstein_lazy.h"

namespace network_butcher::kfinder
{
  /// A factory to create KFinder objects
  /// \tparam T The graph type. To be able to use the class, you must specialize Weighted_Graph<T>
  /// \tparam Only_Distance If true, the KFinder will only return the distance of the paths
  /// \tparam t_Weighted_Graph_Complete_Type The complete type of the weighted graph
  template <typename T,
            bool                 Only_Distance                  = false,
            Valid_Weighted_Graph t_Weighted_Graph_Complete_Type = Weighted_Graph<T>>
  class KFinder_Factory
  {
  private:
    using kfinder_type = KFinder<T, Only_Distance, t_Weighted_Graph_Complete_Type>;

    using Builder_type = std::function<std::unique_ptr<kfinder_type>(T const &, Node_Id_Type, Node_Id_Type)>;

    using Factory = GenericFactory::Factory<kfinder_type, std::string, Builder_type>;

    KFinder_Factory();

  public:
    /// Returns the instance of the factory
    /// \return The instance of the factory
    static KFinder_Factory &
    Instance();

    /// Adds a new KFinder builder to the factory
    /// \param entry_name The name of the builder
    /// \param func The builder
    void
    add(std::string entry_name, Builder_type const &func);

    /// Creates a new KFinder object
    /// \param name The name of the builder
    /// \param graph The graph
    /// \param root The root node id
    /// \param sink The sink node id
    /// \return The KFinder object
    std::unique_ptr<kfinder_type>
    create(std::string const &name, T const &graph, Node_Id_Type root, Node_Id_Type sink) const;

    /// Creates a new KFinder object
    /// \param method Enum for the method to use
    /// \param graph The graph
    /// \param root The root node id
    /// \param sink The sink node id
    /// \return The KFinder object
    std::unique_ptr<kfinder_type>
    create(parameters::KSP_Method method, T const &graph, Node_Id_Type root, Node_Id_Type sink) const;

    /// Returns the vector of registered builders
    /// \return The vector of registered builders
    [[nodiscard]] std::vector<std::string>
    registered() const;

    /// Returns true if the builder is registered
    /// \param name The name of the builder
    /// \return True if the builder is registered
    [[nodiscard]] bool
    registered(std::string const &name) const;

    /// Unregisters a builder
    /// \param name The name of the builder
    void
    unregister(std::string const &name);
  };


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  std::unique_ptr<typename KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::kfinder_type>
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::create(const std::string &name,
                                                                            const T           &graph,
                                                                            Node_Id_Type       root,
                                                                            Node_Id_Type       sink) const
  {
    return Factory::Instance().create(name, graph, root, sink);
  }


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  std::unique_ptr<typename KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::kfinder_type>
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::create(parameters::KSP_Method method,
                                                                            const T               &graph,
                                                                            Node_Id_Type           root,
                                                                            Node_Id_Type           sink) const
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


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  std::vector<std::string>
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::registered() const
  {
    return Factory::Instance().registered();
  }


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  bool
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::registered(const std::string &name) const
  {
    return Factory::Instance().registered(name);
  }


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  void
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::unregister(const std::string &name)
  {
    Factory::Instance().unregister(name);
  }


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  void
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::add(std::string entry_name,
                                                                         const KFinder_Factory::Builder_type &func)
  {
    Factory::Instance().add(entry_name, func);
  }


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::KFinder_Factory()
  {
    auto &factory = Factory::Instance();

    factory.add("eppstein", [](T const &graph, Node_Id_Type root, Node_Id_Type sink) {
      return std::make_unique<KFinder_Eppstein<T, Only_Distance, t_Weighted_Graph_Complete_Type>>(graph, root, sink);
    });
    factory.add("lazy_eppstein", [](T const &graph, Node_Id_Type root, Node_Id_Type sink) {
      return std::make_unique<KFinder_Lazy_Eppstein<T, Only_Distance, t_Weighted_Graph_Complete_Type>>(graph,
                                                                                                       root,
                                                                                                       sink);
    });
  }


  template <typename T, bool Only_Distance, Valid_Weighted_Graph t_Weighted_Graph_Complete_Type>
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type> &
  KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type>::Instance()
  {
    static KFinder_Factory<T, Only_Distance, t_Weighted_Graph_Complete_Type> factory;
    return factory;
  }

} // namespace network_butcher::kfinder

#endif // NETWORK_BUTCHER_KFINDER_FACTORY_H