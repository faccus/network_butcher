//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H

#include "basic_aMLLibrary_weight_importer.h"

namespace network_butcher::io
{
  /// This class will be used to generate and import weights with aMLLibrary into the block graph
  class block_aMLLibrary_Weight_Importer : public basic_aMLLibrary_Weight_Importer
  {
  protected:
    using base = basic_aMLLibrary_Weight_Importer;

    graph_type const &graph;
    block_graph_type &new_graph;

    /// It will produce a row of the aMLLibrary_prediction.csv file
    /// \param entries The entries to insert
    /// \param id The node id in the block graph
    /// \param map_onnx_tool The output of onnx_tool
    /// \return The relevant row
    [[nodiscard]] std::vector<std::string>
    generate_entry(std::vector<std::string> const                                         &entries,
                   std::size_t                                                             id,
                   std::map<std::string, Weight_importer_helpers::onnx_tool_output> const &map_onnx_tool) const;

  public:
    block_aMLLibrary_Weight_Importer(graph_type const                              &graph,
                                     block_graph_type                              &new_graph,
                                     network_butcher::parameters::Parameters const &params)
      : basic_aMLLibrary_Weight_Importer{params}
      , graph{graph}
      , new_graph{new_graph} {};

    block_aMLLibrary_Weight_Importer(
      graph_type const                                                      &graph,
      block_graph_type                                                      &new_graph,
      network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params,
      network_butcher::parameters::Parameters::aMLLibrary const             &aMLLibrary_params,
      parameters::Parameters::Weights const                                 &weights_params,
      network_butcher::parameters::Parameters::Model const                  &model_params,
      parameters::Parameters::Devices const                                 &devices)
      : basic_aMLLibrary_Weight_Importer{block_graph_generation_params,
                                         aMLLibrary_params,
                                         weights_params,
                                         model_params,
                                         devices}
      , graph{graph}
      , new_graph{new_graph} {};


    void
    import_weights() override;

    void
    import_weights(std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition);

    virtual ~block_aMLLibrary_Weight_Importer() = default;
  };

} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H
