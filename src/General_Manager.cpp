//
// Created by faccus on 7/12/22.
//
#include "General_Manager.h"

namespace network_butcher::io
{
  std::function<weight_type(const node_id_type &, size_t, size_t)>
  General_Manager::Helper_Functions::generate_bandwidth_transmission_function(
    const network_butcher::parameters::Parameters &params,
    const graph_type                              &graph)
  {
    std::function<weight_type(node_id_type const &, std::size_t, std::size_t)> transmission_weights =
      [&params, &graph](node_id_type const &node_id, std::size_t first_device, std::size_t second_device) {
        auto const it = params.weights_params.bandwidth.find({first_device, second_device});

        // If the bandwidth cannot be found, return 0
        if (it == params.weights_params.bandwidth.cend())
          {
            return .0;
          }
        else
          {
            // The memory dimension of the output tensor for the given node in bytes
            auto const mem = network_butcher::computer::Computer_memory::compute_memory_usage_output(graph[node_id]);

            // Bandwidth
            auto const bdw = it->second.first;

            // Access delay
            auto const acc = it->second.second;


            if (mem > 0)
              {
                // Conversion from MBit to Bytes
                constexpr double MBit_to_Bytes = 1000000. / 8;

                return (mem / MBit_to_Bytes) / bdw + acc;
              }
            else
              return acc;
          }
      };
    return transmission_weights;
  }


  void
  General_Manager::Helper_Functions::print_help()
  {
    std::cout << std::endl << "Command usage: " << std::endl;
    std::cout << "#1: ./network_butcher config_file=config.conf" << std::endl;
    std::cout << "#2: ./network_butcher annotations=annotations.yaml "
                 "candidate_deployments=candidate_deployments.yaml candidate_resources=candidate_resources.yaml"
              << std::endl;
  }


  void
  General_Manager::boot(std::string const &path, bool performance)
  {
    network_butcher::parameters::Parameters params = IO_Manager::read_parameters(path);
    boot(params, performance);
  }


  void
  General_Manager::boot(const network_butcher::parameters::Parameters &params, bool performance)
  {
    using namespace network_butcher::parameters;

    auto const &weight_params = params.weights_params;

    bool weight_performance = performance && weight_params.weight_import_mode != Weight_Import_Mode::aMLLibrary_block &&
                              weight_params.weight_import_mode != Weight_Import_Mode::block_single_direct_read &&
                              weight_params.weight_import_mode != Weight_Import_Mode::block_multiple_direct_read;

    Chrono crono;
    crono.start();

    // Import the onnx model and populate the graph
    auto [graph, model, link_graph_model] =
      IO_Manager::import_from_onnx(params.model_path, true, true, params.devices.size());
    crono.stop();

    double const import_time = crono.wallTime();
    if (performance)
      std::cout << "Network import: " << import_time / 1000. << " ms" << std::endl;

    if (weight_performance)
      {
        crono.start();
      }

    // Import the weights
    IO_Manager::import_weights(graph, params);
    crono.stop();

    double const import_weights_time = crono.wallTime();
    if (weight_performance)
      std::cout << "Weight import: " << import_weights_time / 1000. << " ms" << std::endl;

    // Prepare the butcher...
    Butcher butcher(std::move(graph));

    crono.start();

    // Start the butchering... (compute the k shortest paths)
    auto const paths = butcher.compute_k_shortest_path(
      Helper_Functions::generate_bandwidth_transmission_function(params, butcher.get_graph()), params);
    crono.stop();

    double const butcher_time = crono.wallTime();
    if (performance && weight_performance)
      {
        std::cout << "Butchering: " << butcher_time / 1000. << " ms" << std::endl;
      }
    else if (performance)
      {
        std::cout << "Weight import and Butchering: " << butcher_time / 1000. << " ms" << std::endl;
      }

    crono.start();
    // Export the butchered networks... (export the different partitions)
    IO_Manager::export_network_partitions(params, model, link_graph_model, paths);
    crono.stop();

    if (performance)
      {
        double const export_time = crono.wallTime();
        std::cout << "Export: " << export_time / 1000. << " ms" << std::endl;

        std::ofstream out_file;
        out_file.open(params.export_directory + "/report.txt");

        out_file << "Network import: " << import_time / 1000. << " ms" << std::endl;
        out_file << "Weight import: " << import_weights_time / 1000. << " ms" << std::endl;
        out_file << "Butchering: " << butcher_time / 1000. << " ms" << std::endl;
        out_file << "Export: " << export_time / 1000. << " ms" << std::endl;

        out_file << std::endl;

        out_file << "Number of found paths: " << paths.size() << std::endl;

        out_file.close();
      }
  }


  void
  General_Manager::read_command_line(int argc, char **argv)
  {
    GetPot command_line(argc, argv);

    std::cout << "Network Butcher" << std::endl;

    if (command_line.search(2, "--help", "-h"))
      {
        Helper_Functions::print_help();
      }
    else
      {
        if (command_line.vector_variable_size("config_file"))
          {
            std::string const config_path = command_line("config_file", "config.conf");

            General_Manager::boot(config_path, true);
          }
        else if (command_line.vector_variable_size("annotations") ||
                 command_line.vector_variable_size("candidate_deployments") ||
                 command_line.vector_variable_size("candidate_resources"))
          {
#if YAML_CPP_ACTIVE
            std::string const annotations_path = command_line("annotations", "annotations.yaml");
            std::string const candidate_deployments_path =
              command_line("candidate_deployments", "candidate_deployments.yaml");
            std::string const candidate_resources_path =
              command_line("candidate_resources", "candidate_resources.yaml");

            auto params =
              IO_Manager::read_parameters_yaml(candidate_resources_path, candidate_deployments_path, annotations_path);

            for (std::size_t i = 0; i < params.size(); ++i)
              {
                auto &param = params[i];

                param.export_directory = "ksp_result_yaml_" + std::to_string(i);
                General_Manager::boot(param, true);
              }
#else
            std::cout << "The library Yaml-Cpp is required to read .yaml files. Please, check the CMakeList.txt "
                         "configuration file."
                      << std::endl;
#endif
          }
        else
          {
            Helper_Functions::print_help();
          }
      }
  }
} // namespace network_butcher::io
