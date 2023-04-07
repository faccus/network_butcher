#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_HELPERS_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_HELPERS_H

#include <utility>

#include "Basic_traits.h"
#include "Computer_memory.h"
#include "Graph_traits.h"
#include "Parameters.h"

#if PYBIND_ACTIVE
#  include "CMake_directives.h"
#  include <pybind11/embed.h>
#  include <pybind11/pybind11.h>

#  if PLATFORM_SPECIFIC_CONFIG
#    include "platform_specific_config.h"
#  endif

#endif

namespace network_butcher
{
  namespace io
  {
    namespace Weight_importer_helpers
    {
      template <typename T>
      using csv_result_type = std::map<std::string, std::vector<T>>;

      enum Index_Type
      {
        Edge,
        Cloud,
        Operation
      };

      struct onnx_tool_output
      {
        std::string name;

        std::size_t macs;
        std::size_t memory;
        std::size_t params;
      };

      /// It will read a .csv file containing numbers
      /// \param path The file path
      /// \param separator The column separator character
      /// \param columns_to_read The (numeric) columns to read
      /// \return The numeric columns in a map
      static csv_result_type<double>
      read_csv_numerics(std::string const              &path,
                        char                            separator,
                        std::vector<std::string> const &columns_to_read,
                        std::string const              &column_suffix     = "",
                        bool                            only_non_negative = false);

      /// It will read a .csv file
      /// \param path The file path
      /// \param separator The column separator character
      /// \param columns_to_read The columns to read
      /// \return The columns in a map
      csv_result_type<std::string>
      read_csv(std::string const       &path,
               char                     separator       = ',',
               std::vector<std::string> columns_to_read = {},
               std::string const       &column_suffix   = "");
    } // namespace Weight_importer_helpers


    class Weight_Importer
    {
    protected:
    public:
      Weight_Importer() = default;

      virtual void
      import_weights() = 0;

      virtual ~Weight_Importer() = default;
    };


    class Csv_Weight_Importer : public Weight_Importer
    {
    protected:
      graph_type &graph;

      std::vector<std::string> paths;
      std::vector<std::size_t> devices;
      std::vector<std::string> relevant_entries;
      char const               separator;
      bool                     only_non_negative;

    public:
      Csv_Weight_Importer(graph_type                     &graph,
                          std::vector<std::string> const &paths,
                          std::vector<std::string> const &relevant_entries,
                          std::vector<std::size_t> const &devices,
                          char                            separator         = ',',
                          bool                            only_non_negative = false)
        : Weight_Importer()
        , graph{graph}
        , paths{paths}
        , devices{devices}
        , relevant_entries{relevant_entries}
        , separator{separator}
        , only_non_negative{only_non_negative} {};


      Csv_Weight_Importer(graph_type                                             &graph,
                          std::vector<std::string> const                         &paths,
                          std::vector<std::string> const                         &relevant_entries,
                          std::vector<network_butcher::parameters::Device> const &devices,
                          char                                                    separator         = ',',
                          bool                                                    only_non_negative = false)
        : Weight_Importer()
        , graph{graph}
        , paths{paths}
        , devices{}
        , relevant_entries{relevant_entries}
        , separator{separator}
        , only_non_negative{only_non_negative}
      {
        for (auto const &device : devices)
          {
            this->devices.push_back(device.id);
          }
      };


      Csv_Weight_Importer(graph_type                                             &graph,
                          std::vector<network_butcher::parameters::Device> const &devices,
                          char                                                    separator         = ',',
                          bool                                                    only_non_negative = false)
        : Weight_Importer()
        , graph{graph}
        , separator{separator}
        , only_non_negative{only_non_negative}
      {
        paths.reserve(devices.size());
        relevant_entries.reserve(devices.size());
        this->devices.reserve(devices.size());

        for (auto const &device : devices)
          {
            this->devices.push_back(device.id);
            paths.push_back(device.weights_path);
            relevant_entries.push_back(device.relevant_entry);
          }
      };


      virtual void
      import_weights() override;


      void
      import_weights(std::function<bool(graph_type::Node_Type const &)> const &extra_condition);


      virtual ~Csv_Weight_Importer() override = default;
    };


    class basic_aMLLibrary_Weight_Importer : public Weight_Importer
    {
    protected:
      network_butcher::parameters::Parameters const params;


      /// It will check if the aMLLibrary is available
      void
      check_aMLLibrary() const;


      /// It will add the python packages to the python path
      void
      add_python_packages() const;


      /// It will create a .ini file in order to use aMLLibrary
      /// \param inference_variable The inference variable
      /// \param input_path The .csv input file
      /// \param output_path The output path of the file
      void
      prepare_predict_file(std::string const &inference_variable,
                           std::string const &input_path,
                           std::string        output_path = "") const;


      /// It will execute the weight generator
      /// \param regressor_file The regressor file
      /// \param config_file The configuration file
      /// \param output_path The output path
      void
      execute_weight_generator(const std::string &regressor_file,
                               const std::string &config_file,
                               const std::string &output_path) const;


      std::string
      network_info_onnx_tool() const;


      std::map<std::string, Weight_importer_helpers::onnx_tool_output>
      read_network_info_onnx_tool(const std::string &path) const;


      void
      csv_assembler(const std::vector<std::vector<std::string>> &content, const std::string &path) const;

    public:
      basic_aMLLibrary_Weight_Importer(network_butcher::parameters::Parameters const &params)
        : Weight_Importer()
        , params{params}
      {
        check_aMLLibrary();
      };

      virtual ~basic_aMLLibrary_Weight_Importer() = default;
    };


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


    class block_aMLLibrary_Weight_Importer : public basic_aMLLibrary_Weight_Importer
    {
    protected:
      graph_type const &graph;
      block_graph_type &new_graph;

      /// It will produce a row of the aMLLibrary_prediction.csv file
      /// \param entries The entries to insert
      /// \param params The collection of parameters
      /// \param new_graph The block graph
      /// \param graph The graph
      /// \param id The node id in the block graph
      /// \param map_onnx_tool The output of onnx_tool
      /// \return The relevant row
      std::vector<std::string>
      generate_entry(std::vector<std::string> const                                         &entries,
                     std::size_t                                                             id,
                     std::map<std::string, Weight_importer_helpers::onnx_tool_output> const &map_onnx_tool) const;

      void
      import(std::size_t                                                     device,
             std::string const                                              &path,
             std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition);

    public:
      block_aMLLibrary_Weight_Importer(graph_type const                              &graph,
                                       block_graph_type                              &new_graph,
                                       network_butcher::parameters::Parameters const &params)
        : basic_aMLLibrary_Weight_Importer{params}
        , graph{graph}
        , new_graph{new_graph} {};


      void
      import_weights();

      void
      import_weights(std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition);

      virtual ~block_aMLLibrary_Weight_Importer() = default;
    };

  } // namespace io
} // namespace network_butcher
#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_HELPERS_H