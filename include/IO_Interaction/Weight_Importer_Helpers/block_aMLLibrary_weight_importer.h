//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H

#include "abstract_aMLLibrary_weight_importer.h"

namespace network_butcher::io
{
  /// This class will be used to generate and import weights with aMLLibrary into the block graph
  class block_aMLLibrary_Weight_Importer : public Abstract_aMLLibrary_Weight_Importer
  {
  protected:
    using base = Abstract_aMLLibrary_Weight_Importer;

    Converted_Onnx_Graph_Type const &graph;
    Block_Graph_Type                &new_graph;

    /// It will generate the relevant entry given its name and the node
    /// \param lower_case The entry name in lower case
    /// \param id The node id in the block graph
    /// \param map_onnx_tool The output of onnx_tool
    /// \param previous_entries_info The info of the previous entries elements in the .csv
    /// \param original_ids The ids of the original graph of the node
    [[nodiscard]] auto
    generate_entry(std::string const                                                      &lower_case,
                   std::size_t                                                             id,
                   std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type> const &map_onnx_tool,
                   std::map<std::string, std::size_t>                                     &previous_entries_info,
                   Node_Id_Collection_Type const                                          &original_ids,
                   Node_Id_Collection_Type const &node_output_ids) const -> std::string;

    /// It will produce a row of the aMLLibrary_prediction.csv file
    /// \param entries The entries to insert
    /// \param id The node id in the block graph
    /// \param map_onnx_tool The output of onnx_tool
    /// \return The relevant row in the .csv file
    [[nodiscard]] auto
    generate_entries(std::vector<std::string> const                                         &entries,
                     std::size_t                                                             id,
                     std::map<std::string, Weight_importer_helpers::Onnx_Tool_Output_Type> const &map_onnx_tool) const
    -> std::vector<std::string>;

  public:
    block_aMLLibrary_Weight_Importer(Converted_Onnx_Graph_Type const               &graph,
                                     Block_Graph_Type                              &new_graph,
                                     network_butcher::parameters::Parameters const &params)
      : Abstract_aMLLibrary_Weight_Importer{params}
      , graph{graph}
      , new_graph{new_graph} {};

    block_aMLLibrary_Weight_Importer(
      Converted_Onnx_Graph_Type const                                       &graph,
      Block_Graph_Type                                                      &new_graph,
      network_butcher::parameters::Parameters::Block_Graph_Generation const &block_graph_generation_params,
      network_butcher::parameters::Parameters::aMLLibrary const             &aMLLibrary_params,
      parameters::Parameters::Weights const                                 &weights_params,
      network_butcher::parameters::Parameters::Model const                  &model_params,
      parameters::Parameters::Devices const                                 &devices)
      : Abstract_aMLLibrary_Weight_Importer{block_graph_generation_params,
                                            aMLLibrary_params,
                                            weights_params,
                                            model_params,
                                            devices}
      , graph{graph}
      , new_graph{new_graph} {};


    /// It will import the weights into the block graph
    void
    import_weights() override;

    /// It will import the weights into the block graph
    /// \param extra_condition A function that will be used to filter the nodes to import (it will skip the nodes that
    /// do not satisfy the condition)
    void
    import_weights(std::function<bool(Block_Graph_Type::Node_Type const &)> const &extra_condition);

    virtual ~block_aMLLibrary_Weight_Importer() = default;
  };

} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_BLOCK_AMLLIBRARY_WEIGHT_IMPORTER_H
