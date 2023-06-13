#include <network_butcher/general_manager.h>

namespace network_butcher::io
{
  auto
  General_Manager::Helper_Functions::generate_bandwidth_transmission_function(
    const network_butcher::parameters::Parameters::Weights &weights_params,
    const Converted_Onnx_Graph_Type &graph) -> std::function<Time_Type(const Edge_Type &, size_t, size_t)>
  {
    return [&weights_params, &graph](Edge_Type const &edge, std::size_t first_device, std::size_t second_device) {
      auto const device_pair = std::make_pair(first_device, second_device);

      auto const &bandwidth = weights_params.bandwidth;

      Bandwidth_Value_Type    bdw;
      Access_Delay_Value_Type acc;

      auto const &[tail, head] = edge;

      // Is the node the first node in the graph?
      if (tail == graph.get_nodes().front().get_id() &&
          weights_params.in_bandwidth.find(device_pair) != weights_params.in_bandwidth.cend())
        {
          auto [tmp_bdw, tmp_acc] = weights_params.in_bandwidth.find(device_pair)->second;
          bdw                     = tmp_bdw;
          acc                     = tmp_acc;
        }
      // Is the node the last node in the graph?
      else if (head == graph.get_nodes().back().get_id() &&
               weights_params.out_bandwidth.find(device_pair) != weights_params.out_bandwidth.cend())
        {
          auto [tmp_bdw, tmp_acc] = weights_params.out_bandwidth.find(device_pair)->second;
          bdw                     = tmp_bdw;
          acc                     = tmp_acc;
        }
      // Is the connection between the specified devices allowed?
      else if (bandwidth->check_weight(device_pair))
        {
          auto [tmp_bdw, tmp_acc] = bandwidth->get_weight(device_pair);
          bdw                     = tmp_bdw;
          acc                     = tmp_acc;
        }
      else
        {
          throw std::runtime_error("Transmission weights: requested a non-set bandwidth from device " +
                                   std::to_string(first_device) + " to device " + std::to_string(second_device) +
                                   ". Please check the configuration file!");
        }

      // The memory dimension of the output tensor for the given node in bytes
      auto const mem = network_butcher::computer::Computer_memory::compute_memory_usage_output(graph[tail]);

      if (mem > 0)
        {
          // Conversion from MBit to Bytes
          constexpr auto MBit_to_Bytes = static_cast<Time_Type>(1000000.) / 8;

          return (mem / MBit_to_Bytes) / bdw + acc;
        }
      else
        {
          return static_cast<Time_Type>(acc);
        }
    };
  }


  void
  General_Manager::Helper_Functions::print_help()
  {
    std::cout << std::endl << "Command usage: " << std::endl;
    std::cout << "./main config_file=config.conf" << std::endl;
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
      IO_Manager::import_from_onnx(params.model_params.model_path, true, true, params.devices.size());
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
      Helper_Functions::generate_bandwidth_transmission_function(params.weights_params, butcher.get_graph()), params);
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

        out_file.open(Utilities::combine_path(params.model_params.export_directory, "report.txt"));

        out_file << "Network import: " << import_time / 1000. << " ms" << std::endl;
        out_file << "Weight import: " << import_weights_time / 1000. << " ms" << std::endl;
        out_file << "Butchering: " << butcher_time / 1000. << " ms" << std::endl;
        out_file << "Export: " << export_time / 1000. << " ms" << std::endl;

        out_file << std::endl;

        out_file << "Number of found paths: " << paths.size() << std::endl << std::endl;

        if (params.block_graph_generation_params.use_bandwidth_to_manage_connections)
          {
            out_file << "Imported Bandwidth: " << std::endl;
            out_file << params.weights_params.bandwidth->print_graph() << std::endl << std::endl;

            out_file << "Input Bandwidth: ";
            if (params.weights_params.in_bandwidth.empty())
              {
                out_file << "Not specified" << std::endl;
              }
            else
              {
                out_file << std::endl;

                out_file << "In Out Bandwidth Access_Time" << std::endl;

                for (auto const &[edge, bw] : params.weights_params.in_bandwidth)
                  {
                    out_file << Utilities::custom_to_string(edge) << " " << Utilities::custom_to_string(bw)
                             << std::endl;
                  }
              }

            out_file << "Output Bandwidth: ";
            if (params.weights_params.out_bandwidth.empty())
              {
                out_file << "Not specified" << std::endl << std::endl;
              }
            else
              {
                out_file << std::endl;

                out_file << "In Out Bandwidth Access_Time" << std::endl;

                for (auto const &[edge, bw] : params.weights_params.out_bandwidth)
                  {
                    out_file << Utilities::custom_to_string(edge) << " " << Utilities::custom_to_string(bw)
                             << std::endl;
                  }

                out_file << std::endl;
              }
          }

        out_file.close();

        auto path = params.model_params.config_path;
        if (path.back() == '/')
          path = path.substr(0, path.size() - 1);
        auto name = path.substr(path.find_last_of('/') + 1);

        Utilities::file_copy(path, Utilities::combine_path(params.model_params.export_directory, name));
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
        else
          {
            Helper_Functions::print_help();
          }
      }
  }
} // namespace network_butcher::io
