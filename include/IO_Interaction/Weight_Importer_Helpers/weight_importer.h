#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_H

#include "basic_traits.h"
#include "graph_traits.h"
#include "parameters.h"

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
