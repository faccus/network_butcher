//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H

#include "Weight_Importer.h"
#include "Weight_Importer_Utils.h"

namespace network_butcher::io
{
  template <typename T>
  class Csv_Weight_Importer : public Weight_Importer
  {
  protected:
    T &graph;

    std::vector<std::string> paths;
    std::vector<std::size_t> devices;
    std::vector<std::string> relevant_entries;
    char const               separator;
    bool                     only_non_negative;

  public:
    Csv_Weight_Importer(T                              &graph,
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


    Csv_Weight_Importer(T                                                      &graph,
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


    Csv_Weight_Importer(T                                                      &graph,
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


    virtual ~Csv_Weight_Importer() override = default;
  };


  template <>
  class Csv_Weight_Importer<graph_type> : public Weight_Importer
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


  template <>
  class Csv_Weight_Importer<block_graph_type> : public Weight_Importer
  {
  protected:
    block_graph_type &graph;

    std::vector<std::string> paths;
    std::vector<std::size_t> devices;
    std::vector<std::string> relevant_entries;
    char const               separator;
    bool                     only_non_negative;

  public:
    Csv_Weight_Importer(block_graph_type               &graph,
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


    Csv_Weight_Importer(block_graph_type                                       &graph,
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


    Csv_Weight_Importer(block_graph_type                                       &graph,
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
    import_weights(std::function<bool(block_graph_type::Node_Type const &)> const &extra_condition);
  };
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H
