//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_ORIGINAL_AMLLIBRARY_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_ORIGINAL_AMLLIBRARY_WEIGHT_IMPORTER_H

#include "basic_aMLLibrary_weight_importer.h"

namespace network_butcher::io
{
  /// This class will be used to generate and import weights with aMLLibrary into a graph
  class original_aMLLibrary_Weight_Importer : public basic_aMLLibrary_Weight_Importer
  {
  protected:
    using base = basic_aMLLibrary_Weight_Importer;

    Converted_Onnx_Graph_Type &graph;

    /// It will generate the relevant entry given its name and the node
    /// \param entry The entry name
    /// \param node The node
    /// \return The resulting entry value
    [[nodiscard]] std::string
    generate_entry(std::string const &entry, Converted_Onnx_Graph_Type::Node_Type const &node) const;

    /// It will generate the relevant entry given its name and the node
    /// \param entry The entry name
    /// \param basic_info The relevant info from onnx_tool
    /// \param node The node
    /// \return The resulting entry value
    [[nodiscard]] std::string
    generate_entry(std::string const                               &entry,
                   Weight_importer_helpers::onnx_tool_output const &basic_info,
                   Converted_Onnx_Graph_Type::Node_Type const                     &node) const;

  public:
    original_aMLLibrary_Weight_Importer(Converted_Onnx_Graph_Type                     &graph, network_butcher::parameters::Parameters const &params)
      : basic_aMLLibrary_Weight_Importer{params}
      , graph{graph} {};

    explicit original_aMLLibrary_Weight_Importer(
      Converted_Onnx_Graph_Type                                             &graph,
      network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params,
      network_butcher::parameters::Parameters::aMLLibrary const             &aMLLibrary_params,
      parameters::Parameters::Weights const                                 &weights_params,
      network_butcher::parameters::Parameters::Model const                  &model_params,
      parameters::Parameters::Devices const                                 &devices)
      : basic_aMLLibrary_Weight_Importer(block_graph_generation_params,
                                         aMLLibrary_params,
                                         weights_params,
                                         model_params,
                                         devices)
      , graph{graph} {};


    void
    import_weights(std::function<bool(Converted_Onnx_Graph_Type::Node_Type const &)> const &extra_condition);

    void
    import_weights() override;

    ~original_aMLLibrary_Weight_Importer() override = default;
  };
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_ORIGINAL_AMLLIBRARY_WEIGHT_IMPORTER_H
