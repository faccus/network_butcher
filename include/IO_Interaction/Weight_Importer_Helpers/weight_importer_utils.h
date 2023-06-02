//
// Created by faccus on 13/04/23.
//

#ifndef NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H
#define NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H

#include "basic_traits.h"
#include "utilities.h"

namespace network_butcher::io::Weight_importer_helpers
{
  template <typename T>
  using Csv_Result_Type = std::map<std::string, std::vector<T>>;

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
  /// \param column_suffix A suffix to place after the column name in the result object
  /// \param only_non_negative It will convert negative weights to zeros
  /// \return The numeric columns in a map
  Csv_Result_Type<Time_Type>
  read_csv_numerics(std::string const              &path,
                    char                            separator,
                    std::vector<std::string> const &columns_to_read,
                    std::string const              &column_suffix     = "",
                    bool                            only_non_negative = false);

  /// It will read a .csv file
  /// \param path The file path
  /// \param separator The column separator character
  /// \param columns_to_read The columns to read
  /// \param column_suffix A suffix to place after the column name in the result object
  /// \return The columns in a map
  Csv_Result_Type<std::string>
  read_csv(std::string const       &path,
           char                     separator       = ',',
           std::vector<std::string> columns_to_read = {},
           std::string const       &column_suffix   = "");
} // namespace network_butcher::io::Weight_importer_helpers

#endif // NETWORK_BUTCHER_WEIGHT_IMPORTER_UTILS_H
