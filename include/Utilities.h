//
// Created by faccus on 28/08/21.
//

#ifndef NETWORK_BUTCHER_UTILITIES_H
#define NETWORK_BUTCHER_UTILITIES_H

#include <filesystem>
#include <fstream>

#include "onnx.pb.h"
#include "Basic_traits.h"
#include "Type_info_traits.h"

#include "Dense_tensor.h"
#include "Type_info.h"


class Utilities
{
public:
  /// From onnx::TensorProto_DataType_*, it will return the size of the respective type in bytes
  /// \return Size of the type in bytes
  static memory_type compute_memory_usage_from_enum(type_info_id_type);

  /// Construct a ModelProto from an onnx file
  /// \param m Reference to the model that will be constructed
  /// \param model_path Path to the .onnx file
  static void
  parse_onnx_file(onnx::ModelProto &m, const std::string &model_path);

  /// Construct a ModelProto from an onnx file
  /// \param model_path Path to the .onnx file
  /// \return The constructed model
  static onnx::ModelProto
  parse_onnx_file(const std::string &model_path);

  /// Outputs an onnx file from the given model
  /// \param m Model to be exported
  /// \param path Path of the exported model
  static void
  output_onnx_file(onnx::ModelProto const &m, const std::string &path);

  /// Check if a file exists
  /// \param name Path to the file
  /// \return True if it exists, false otherwise
  static bool
  file_exists(const std::string &name)
  {
    const std::filesystem::path p = name;
    return std::filesystem::exists(p);
  }

  /// Check if a directory exists
  /// \param name Path to the directory
  /// \return True if it exists, false otherwise
  static bool
  directory_exists(const std::string &name)
  {
    const std::filesystem::path p = name;
    return std::filesystem::exists(p);
  }

  /// Deletes the file at the specified location
  /// \param path The path of the file
  static void
  file_delete(std::string const &path)
  {
    std::filesystem::remove_all(path);
  }

  /// Deletes the directory at the specified location
  /// \param path The path of the directory
  static void
  directory_delete(std::string const &path)
  {
    std::filesystem::remove_all(path);
  }

  /// Creates a directory with the given path
  /// \param path The input string
  static void
  create_directory(const std::string &path)
  {
    if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path))
      {
        std::filesystem::create_directory(path);
      }
  }

  /// Left trim for the input string (modifies the input string)
  /// \param s The input string
  static void
  ltrim(std::string &s)
  {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  }

  /// Right trim for the input string (modifies the input string)
  /// \param s The input string
  static void
  rtrim(std::string &s)
  {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  }

  /// Left trim for the input string (modifies the input string)
  /// \param s The input string
  static void
  ltrim(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  }

  /// Right trim for the input string (modifies the input string)
  /// \param s The input string
  static void
  rtrim(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  }

  /// Trim for the input string (modifies the input string)
  /// \param s The input string
  static void
  trim(std::string &s)
  {
    ltrim(s);
    rtrim(s);
  }

  /// Trim for the input string (modifies the input string)
  /// \param s The input string
  static void
  trim(std::vector<std::string> &s)
  {
    ltrim(s);
    rtrim(s);
  }

  /// Sets in lowercase the input string
  /// \param s The input string
  static void
  to_lowercase(std::string &s)
  {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }

  /// Sets in lowercase the input string
  /// \param s The input string
  static void
  to_lowercase(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }

  /// Left trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  static std::string
  ltrim_copy(std::string s)
  {
    ltrim(s);
    return s;
  }

  /// Right trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  static std::string
  rtrim_copy(std::string s)
  {
    rtrim(s);
    return s;
  }

  /// Trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  static std::string
  trim_copy(std::string s)
  {
    trim(s);
    return s;
  }

  /// Trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  static std::vector<std::string>
  trim_copy(std::vector<std::string> s)
  {
    trim(s);
    return s;
  }

  /// Returns in lowercase the input string
  /// \param s The input string
  /// \return The modified string
  static std::string
  to_lowercase_copy(std::string s)
  {
    to_lowercase(s);
    return s;
  }

  /// Returns in lowercase the input string
  /// \param s The input string
  /// \return The modified string
  static std::vector<std::string>
  to_lowercase_copy(std::vector<std::string> s)
  {
    to_lowercase(s);
    return s;
  }

  /// It splits the input string in a vector (https://stackoverflow.com/a/46931770)
  /// \param s Input string
  /// \param delimiter Delimiter string
  /// \return The "splitted" string
  static std::vector<std::string>
  split(std::string s, std::string delimiter);

  /// It combines the two input paths
  /// \param first first path
  /// \param second second path
  /// \return path concatenation
  static std::string
  combine_path(std::string const &first, std::string const &second);
};

#endif // NETWORK_BUTCHER_UTILITIES_H
