#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_H

#include <network_butcher/Network/graph_traits.h>
#include <network_butcher/Types/parameters.h>
#include <network_butcher/Traits/traits.h>

namespace network_butcher::io
{
  /// This (pure virtual) class will be used to import weights into a graph
  class Weight_Importer
  {
  protected:
  public:
    Weight_Importer() = default;

    virtual void
    import_weights() = 0;

    virtual ~Weight_Importer() = default;
  };
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_H
