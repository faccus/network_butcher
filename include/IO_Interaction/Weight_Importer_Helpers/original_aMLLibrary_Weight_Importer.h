//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_ORIGINAL_AMLLIBRARY_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_ORIGINAL_AMLLIBRARY_WEIGHT_IMPORTER_H

#include "Csv_Weight_Importer.h"
#include "basic_aMLLibrary_Weight_Importer.h"

namespace network_butcher::io
{
  class original_aMLLibrary_Weight_Importer : public basic_aMLLibrary_Weight_Importer
  {
  protected:
    graph_type &graph;

    std::string
    generate_entry(std::string const &entry, graph_type::Node_Type const &node) const;

    std::string
    generate_entry(std::string const                               &entry,
                   Weight_importer_helpers::onnx_tool_output const &basic_info,
                   graph_type::Node_Type const                     &node) const;

  public:
    original_aMLLibrary_Weight_Importer(graph_type &graph, network_butcher::parameters::Parameters const &params)
      : basic_aMLLibrary_Weight_Importer{params}
      , graph{graph} {};


    void
    import_weights(std::function<bool(graph_type::Node_Type const &)> const &extra_condition);

    virtual void
    import_weights() override;

    virtual ~original_aMLLibrary_Weight_Importer() override = default;
  };
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_ORIGINAL_AMLLIBRARY_WEIGHT_IMPORTER_H
