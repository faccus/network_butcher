#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_H

#include <network_butcher/Network/graph_traits.h>
#include <network_butcher/Traits/traits.h>
#include <network_butcher/Types/parameters.h>

namespace network_butcher::io
{
  /// This (pure virtual) class will be used to import weights into a graph
  /// \tparam GraphType The graph type
  template <typename GraphType>
  class Weight_Importer
  {
  protected:
    /// The graph (stored as a reference)
    GraphType &graph;

  public:
    /// Constructor for weight importer of graph
    /// \param in The input graph (stored as reference)
    explicit Weight_Importer(GraphType &in)
      : graph(in){};

    /// Pure virtual method. It will import the weights on the graph (not stored in the base class. The storage is
    ///
    virtual void
    import_weights() = 0;

    virtual ~Weight_Importer() = default;
  };
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_H
