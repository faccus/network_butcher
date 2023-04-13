//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_H

#include "Basic_traits.h"
#include "Graph_traits.h"
#include "Parameters.h"

namespace network_butcher::io
{
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
