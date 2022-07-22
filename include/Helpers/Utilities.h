//
// Created by faccus on 28/08/21.
//

#ifndef NETWORK_BUTCHER_UTILITIES_H
#define NETWORK_BUTCHER_UTILITIES_H

#include <filesystem>
#include <fstream>

#include "Traits/Basic_traits.h"
#include "Traits/Type_info_traits.h"
#include "../../src/Onnx_model/onnx.pb.h"

#include "Types/Dense_tensor.h"
#include "Types/Type_info.h"


namespace utilities
{
  /// From onnx::TensorProto_DataType_*, it will return the size of the respective type in bytes
  /// \return Size of the type in bytes
  memory_type
  compute_memory_usage_from_enum(type_info_id_type);

  /// Construct a ModelProto from an onnx file
  /// \param m Reference to the model that will be constructed
  /// \param model_path Path to the .onnx file
  void
  parse_onnx_file(onnx::ModelProto &m, const std::string &model_path);

  /// Construct a ModelProto from an onnx file
  /// \param model_path Path to the .onnx file
  /// \return The constructed model
  onnx::ModelProto
  parse_onnx_file(const std::string &model_path);

  /// Outputs an onnx file from the given model
  /// \param m Model to be exported
  /// \param path Path of the exported model
  void
  output_onnx_file(onnx::ModelProto const &m, const std::string &path);

  /// Check if a file exists
  /// \param name Path to the file
  /// \return True if it exists, false otherwise
  inline bool
  file_exists(const std::string &name)
  {
    const std::filesystem::path p = name;
    return std::filesystem::exists(p);
  }

  /// Creates a directory with the given path
  /// \param path The input string
  inline void
  create_directory(const std::string &path) {
    if(!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
      }
  }

  /// Left trim for the input string (modifies the input string)
  /// \param s The input string
  static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
              return !std::isspace(ch);
            }));
  }

  /// Right trim for the input string (modifies the input string)
  /// \param s The input string
  static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
              return !std::isspace(ch);
            }).base(), s.end());
  }

  /// Trim for the input string (modifies the input string)
  /// \param s The input string
  static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
  }

  /// Sets in lowercase the input string
  /// \param s The input string
  static inline void to_lowercase(std::string &s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }

  /// Left trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
  }

  /// Right trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
  }

  /// Trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
  }

  /// Returns in lowercase the input string
  /// \param s The input string
  /// \return The modified string
  static inline std::string to_lowercase_copy(std::string s) {
    to_lowercase(s);
    return s;
  }

} // namespace utilities

#endif // NETWORK_BUTCHER_UTILITIES_H
