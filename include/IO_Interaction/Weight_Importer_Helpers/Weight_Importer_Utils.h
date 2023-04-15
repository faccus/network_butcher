//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H

#include "Basic_traits.h"
#include "Utilities.h"

namespace network_butcher::io::Weight_importer_helpers
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
  csv_result_type<double>
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
} // namespace network_butcher::io::Weight_importer_helpers

#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H