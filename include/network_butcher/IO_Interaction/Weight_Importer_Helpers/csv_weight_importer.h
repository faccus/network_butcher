#ifndef NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H

#include <network_butcher/IO_Interaction/Weight_Importer_Helpers/weight_importer.h>
#include <network_butcher/IO_Interaction/Weight_Importer_Helpers/weight_importer_utils.h>

namespace network_butcher::io
{
  /// This class will be used to import weights from a .csv file into a graph
  /// \tparam GraphType The graph type
  template <typename GraphType>
  class Csv_Weight_Importer : public Weight_Importer
  {
  protected:
    /// A reference to the graph
    GraphType &graph;

    /// The path(s) to the .csv file(s)
    std::vector<std::string> paths;

    /// The ids of the devices to be considered
    std::vector<std::size_t> devices;

    /// The relevant entries in the .csv file(s)
    std::vector<std::string> relevant_entries;

    /// The separator used in the .csv file(s)
    char const               separator;

    /// True if negative weights should be defaulted to 0.
    bool                     only_non_negative;


    /// Checks if the provided paths are compatible with the Importer. It will throw if not compatible
    /// \return True if a single path was provided, false otherwise
    [[nodiscard]] auto
    check_paths() const -> bool;

    /// Reads the data from the .csv file(s)
    /// \param single_call True if a single path was provided, false otherwise
    /// \return A map with the data read from the .csv file(s)
    [[nodiscard]] auto
    get_data(bool single_call) const -> Weight_importer_helpers::Csv_Result_Type<Time_Type>;

    /// Checks if the columns in the .csv file(s) coincide with relevant_entries. If not, it will throw an exception
    /// \param data The data read from the .csv file(s)
    /// \param single_call True if a single path was provided, false otherwise
    void
    check_entries(Weight_importer_helpers::Csv_Result_Type<Time_Type> const &data, bool single_call) const;


  public:
    /// It will prepare a Csv_Weight_Importer
    /// \param graph A reference to a graph
    /// \param paths The path(s) to the .csv file(s)
    /// \param relevant_entries The relevant entries in the .csv file(s)
    /// \param devices The ids of the devices to be considered
    /// \param separator The separator used in the .csv file(s)
    /// \param only_non_negative True if negative weights should be defaulted to 0.
    Csv_Weight_Importer(GraphType                      &graph,
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


    /// It will prepare a Csv_Weight_Importer
    /// \param graph A reference to a graph
    /// \param paths The path(s) to the .csv file(s)
    /// \param relevant_entries The relevant entries in the .csv file(s)
    /// \param devices The devices to be considered
    /// \param separator The separator used in the .csv file(s)
    /// \param only_non_negative True if negative weights should be defaulted to 0.
    Csv_Weight_Importer(GraphType                                              &graph,
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


    /// It will prepare a Csv_Weight_Importer
    /// \param graph A reference to a graph
    /// \param devices The devices to be considered
    /// \param separator The separator used in the .csv file(s)
    /// \param only_non_negative True if negative weights should be defaulted to 0.
    Csv_Weight_Importer(GraphType                                              &graph,
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

    /// It will import the weights into the graph
    /// \param extra_condition A function that will be called for each node. If it returns true, the node will be
    /// considered. If it returns false, the node will be ignored
    void
    import_weights(std::function<bool(typename GraphType::Node_Type const &)> const &extra_condition);

    /// It will import the weights into the graph
    virtual void
    import_weights() override
    {
      import_weights(nullptr);
    }

    virtual ~Csv_Weight_Importer() override = default;
  };

  template <typename GraphType>
  void
  Csv_Weight_Importer<GraphType>::check_entries(
    const Weight_importer_helpers::Csv_Result_Type<network_butcher::Time_Type> &data,
    bool                                                                        single_call) const
  {
    // Check if there are missing entries
    for (std::size_t i = 0; i < relevant_entries.size(); ++i)
      {
        auto const &entry = relevant_entries[i];
        if (single_call ? data.find(entry) == data.cend() : data.find(entry + "_" + std::to_string(i)) == data.cend())
          {
            throw std::runtime_error("Csv_Weight_Importer: The entry " + entry + " for the device " +
                                     std::to_string(devices[i]) + " doesn't exist! We cannot import the weights");
          }
      }
  }

  template <typename GraphType>
  auto
  Csv_Weight_Importer<GraphType>::get_data(bool single_call) const
    -> Weight_importer_helpers::Csv_Result_Type<Time_Type>
  {
    Weight_importer_helpers::Csv_Result_Type<Time_Type> data;

    // Import the data (if from multiple files, append to the name of the columns a suffix)
    if (single_call)
      {
        data = Weight_importer_helpers::read_csv_numerics(paths[0], separator, relevant_entries, "", only_non_negative);
      }
    else
      {
        for (std::size_t i = 0; i < paths.size(); ++i)
          {
            auto tmp = Weight_importer_helpers::read_csv_numerics(
              paths[i], separator, {relevant_entries[i]}, "_" + std::to_string(i), only_non_negative);
            data.insert(tmp.begin(), tmp.end());
          }
      }

    return data;
  }

  template <typename GraphType>
  auto
  Csv_Weight_Importer<GraphType>::check_paths() const -> bool
  {
    if (paths.empty())
      throw std::runtime_error("Csv_Weight_Importer: No paths were provided to import the weights");

    if (paths.size() != 1 && paths.size() != relevant_entries.size())
      throw std::runtime_error("Csv_Weight_Importer: The number of paths provided to import the weights is not the "
                               "same as the number of the specified entries");

    if (paths.size() > devices.size())
      throw std::runtime_error("Csv_Weight_Importer: The number of paths provided to import the weights is greater "
                               "than the number of devices");

    if (relevant_entries.size() != devices.size())
      throw std::runtime_error("Csv_Weight_Importer: The number of entries provided to import the weights is not the "
                               "same as the number of devices");

    return paths.size() == 1;
  }
} // namespace network_butcher::io

#endif // NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H
