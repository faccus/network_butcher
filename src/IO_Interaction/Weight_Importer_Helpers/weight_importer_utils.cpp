//
// Created by faccus on 13/04/23.
//

#include "weight_importer_utils.h"

namespace network_butcher::io::Weight_importer_helpers
{
  Csv_Result_Type<Time_Type>
  read_csv_numerics(std::string const              &path,
                    char                            separator,
                    std::vector<std::string> const &columns_to_read,
                    std::string const              &column_suffix,
                    bool                            only_non_negative)
  {
    Csv_Result_Type<Time_Type> res;

    auto const data = read_csv(path, separator, columns_to_read, column_suffix);

    std::function<std::vector<Time_Type>(std::vector<std::string> const &)> process_data_entry;

    if (only_non_negative)
      {
        process_data_entry = [](auto const &value) {
          std::vector<Time_Type> tmp_vec;
          tmp_vec.reserve(value.size());
          for (auto const &val : value)
            {
              tmp_vec.push_back(std::max(std::stod(val), 0.));
            }
          return tmp_vec;
        };
      }
    else
      {
        process_data_entry = [](auto const &value) {
          std::vector<Time_Type> tmp_vec;
          tmp_vec.reserve(value.size());
          for (auto const &val : value)
            {
              tmp_vec.push_back(std::stod(val));
            }
          return tmp_vec;
        };
      }

    for (auto const &[key, value] : data)
      {
        res.emplace(key, process_data_entry(value));
      }

    return res;
  }


  Csv_Result_Type<std::string>
  read_csv(std::string const       &path,
           char                     separator,
           std::vector<std::string> columns_to_read,
           std::string const       &column_suffix)
  {
    std::ifstream file(path);

    Csv_Result_Type<std::string> res;


    if (file.is_open())
      {
        Utilities::to_lowercase(columns_to_read);
        Utilities::trim(columns_to_read);

        std::vector<std::size_t>           indices;
        std::string                        tmp_line;
        std::map<std::size_t, std::string> index_map;

        if (!std::getline(file, tmp_line))
          return res;

        {
          std::stringstream reader(tmp_line);
          std::size_t       i = 0;

          if (!columns_to_read.empty())
            {
              while (std::getline(reader, tmp_line, separator))
                {
                  Utilities::to_lowercase(tmp_line);
                  Utilities::trim(tmp_line);

                  if (std::find(columns_to_read.cbegin(), columns_to_read.cend(), tmp_line) != columns_to_read.cend())
                    {
                      indices.push_back(i);
                      index_map[i] = (tmp_line + column_suffix);
                    }

                  ++i;
                }
            }
          else
            {
              while (std::getline(reader, tmp_line, separator))
                {
                  Utilities::to_lowercase(tmp_line);
                  Utilities::trim(tmp_line);

                  indices.push_back(i);
                  index_map[i] = (tmp_line + column_suffix);

                  ++i;
                }
            }
        }


        while (std::getline(file, tmp_line))
          {
            std::stringstream reader(tmp_line);
            std::size_t       i = 0, j = 0;

            while (std::getline(reader, tmp_line, separator))
              {
                if (indices[j] == i)
                  {
                    res[index_map[indices[j]]].emplace_back(tmp_line.c_str());
                    ++j;
                  }

                ++i;
              }
          }
      }

    return res;
  }
} // namespace network_butcher::io::Weight_importer_helpers