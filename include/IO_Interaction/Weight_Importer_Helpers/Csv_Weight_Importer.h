//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H
#define NETWORK_BUTCHER_CSV_WEIGHT_IMPORTER_H

#include "Weight_Importer.h"
#include "Weight_Importer_Utils.h"

namespace network_butcher::io
{
  /// This class will be used to import weights from a .csv file into a graph
  /// \tparam T The graph type
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


    /// Checks if the provided paths are compatible with the Importer. It will throw if not compatible
    /// \return True if a single path was provided, false otherwise
    [[nodiscard]] bool
    check_paths() const;

    /// Reads the data from the .csv file(s)
    /// \param single_call True if a single path was provided, false otherwise
    /// \return A map with the data read from the .csv file(s)
    [[nodiscard]] Weight_importer_helpers::csv_result_type<weight_type>
    get_data(bool single_call) const;

    /// Checks if the columns in the .csv file(s) coincide with relevant_entries. If not, it will throw an exception
    /// \param data The data read from the .csv file(s)
    /// \param single_call True if a single path was provided, false otherwise
    void
    check_entries(Weight_importer_helpers::csv_result_type<weight_type> const &data, bool single_call) const;


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

    void
    import_weights(std::function<bool(typename T::Node_Type const &)> const &extra_condition);

    virtual void
    import_weights() override
    {
      import_weights(nullptr);
    }

    virtual ~Csv_Weight_Importer() override = default;
  };

  template <typename T>
  void
  Csv_Weight_Importer<T>::check_entries(
    const Weight_importer_helpers::csv_result_type<network_butcher::weight_type> &data,
    bool                                                                          single_call) const
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

  template <typename T>
  Weight_importer_helpers::csv_result_type<weight_type>
  Csv_Weight_Importer<T>::get_data(bool single_call) const
  {
    Weight_importer_helpers::csv_result_type<weight_type> data;

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

  template <typename T>
  bool
  Csv_Weight_Importer<T>::check_paths() const
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
