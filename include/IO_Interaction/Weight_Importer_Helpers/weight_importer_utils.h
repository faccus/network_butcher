#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H

#include "basic_traits.h"
#include "utilities.h"

namespace network_butcher::io::Weight_importer_helpers
{
  /// It's the result of a CSV file
  template <typename T>
  using Csv_Result_Type = std::map<std::string, std::vector<T>>;

  /// It summarizes an entry produced by onnx_tool
  struct Onnx_Tool_Output_Type
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
  /// \param column_suffix A suffix to place after the column name in the result object
  /// \param only_non_negative It will convert negative weights to zeros
  /// \return The numeric columns in a map
  auto
  read_csv_numerics(std::string const              &path,
                    char                            separator,
                    std::vector<std::string> const &columns_to_read,
                    std::string const              &column_suffix     = "",
                    bool                            only_non_negative = false) -> Csv_Result_Type<Time_Type>;

  /// It will read a .csv file
  /// \param path The file path
  /// \param separator The column separator character
  /// \param columns_to_read The columns to read
  /// \param column_suffix A suffix to place after the column name in the result object
  /// \return The columns in a map
  auto
  read_csv(std::string const       &path,
           char                     separator       = ',',
           std::vector<std::string> columns_to_read = {},
           std::string const       &column_suffix   = "") -> Csv_Result_Type<std::string>;
} // namespace network_butcher::io::Weight_importer_helpers

#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H
