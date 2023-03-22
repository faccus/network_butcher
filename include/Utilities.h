//
// Created by faccus on 28/08/21.
//

#ifndef NETWORK_BUTCHER_UTILITIES_H
#define NETWORK_BUTCHER_UTILITIES_H

#include <filesystem>
#include <fstream>

#include "Basic_traits.h"
#include "Type_info_traits.h"
#include "onnx.pb.h"

#include "Dense_tensor.h"
#include "Type_info.h"


class Utilities
{
public:
  /// From onnx::TensorProto_DataType_*, it will return the size of the respective type in bytes
  /// \return Size of the type in bytes

  /// @brief From onnx::TensorProto_DataType_*, it will return the size of the respective type in bytes
  /// @param  input The onnx_type
  /// @return The memory usage of the type
  static memory_type
  compute_memory_usage_from_enum(type_info_id_type input);


  /// @brief Construct a ModelProto from an onnx file
  /// @param m Reference to the model that will be constructed
  /// @param model_path Path to the .onnx file
  static void
  parse_onnx_file(onnx::ModelProto &m, const std::string &model_path);


  /// @brief Construct a ModelProto from an onnx file
  /// @param model_path Path to the .onnx file
  /// @return The constructed model
  static onnx::ModelProto
  parse_onnx_file(const std::string &model_path);


  /// @brief Outputs an onnx file from the given model
  /// @param m Model to be exported
  /// @param path Path of the exported model
  static void
  output_onnx_file(onnx::ModelProto const &m, const std::string &path);


  /// @brief Check if a file exists
  /// @param name Path to the file
  /// @return True if it exists, false otherwise
  static bool
  file_exists(const std::string &name)
  {
    const std::filesystem::path p = name;
    return std::filesystem::exists(p);
  }


  /// @brief Check if a directory exists
  /// @param name Path to the directory
  /// @return True if it exists, false otherwise
  static bool
  directory_exists(const std::string &name)
  {
    const std::filesystem::path p = name;
    return std::filesystem::exists(p);
  }


  /// @brief Deletes the file at the specified location
  /// @param path The path of the file
  static void
  file_delete(std::string const &path)
  {
    std::filesystem::remove_all(path);
  }


  /// @brief Deletes the directory at the specified location
  /// @param path The path of the directory
  static void
  directory_delete(std::string const &path)
  {
    std::filesystem::remove_all(path);
  }


  /// @brief Creates a directory with the given path
  /// @param path The path of the directory
  static void
  create_directory(const std::string &path)
  {
    if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path))
      {
        std::filesystem::create_directory(path);
      }
  }


  /// @brief Left trim for the input string (modifies the input string)
  /// @param s The input string
  static void
  ltrim(std::string &s)
  {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  }


  /// @brief Right trim for the input string (modifies the input string)
  /// @param s The input string
  static void
  rtrim(std::string &s)
  {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  }


  /// @brief Left trim for the input string vector
  /// @param s The input string vector
  static void
  ltrim(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  }


  /// @brief Right trim for the input string vector
  /// @param s The input string vector
  static void
  rtrim(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  }


  /// @brief Trim for the input string (modifies the input string)
  /// @param s The input string
  static void
  trim(std::string &s)
  {
    ltrim(s);
    rtrim(s);
  }


  /// @brief Trim for the input string vector
  /// @param s The input string vector
  static void
  trim(std::vector<std::string> &s)
  {
    ltrim(s);
    rtrim(s);
  }


  /// @brief Sets in lowercase the input string
  /// @param s The input string
  static void
  to_lowercase(std::string &s)
  {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }


  /// @brief Sets in lowercase the input string vector
  /// @param vect The input string vector
  static void
  to_lowercase(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }


  /// @brief Left trim for the input string (returns the modified string)
  /// @param s The input string
  /// @return The modified string
  static std::string
  ltrim_copy(std::string s)
  {
    ltrim(s);
    return s;
  }


  /// @brief Right trim for the input string (returns the modified string)
  /// @param s The input string
  /// @return The modified string
  static std::string
  rtrim_copy(std::string s)
  {
    rtrim(s);
    return s;
  }


  /// @brief Left trim for the input string (returns the modified string)
  /// @param s The input string
  /// @return The modified string
  static std::string
  trim_copy(std::string s)
  {
    trim(s);
    return s;
  }


  /// @brief Trim for the input string vector (returns the modified string vector)
  /// @param s The input string vector
  /// @return The modified string vector
  static std::vector<std::string>
  trim_copy(std::vector<std::string> s)
  {
    trim(s);
    return s;
  }


  /// @brief It returns in lowercase the input string
  /// @param s The input string
  /// @return The modified string
  static std::string
  to_lowercase_copy(std::string s)
  {
    to_lowercase(s);
    return s;
  }


  /// @brief It returns in lowercase the input string vector
  /// @param s The input string vector
  /// @return The modified string vector
  static std::vector<std::string>
  to_lowercase_copy(std::vector<std::string> s)
  {
    to_lowercase(s);
    return s;
  }


  /// @brief It splits the input string in a vector (https://stackoverflow.com/a/46931770)
  /// @param s Input string vector
  /// @param delimiter The delimiter string
  /// @return The "splitted" string vector
  static std::vector<std::string>
  split(std::string s, std::string delimiter);


  /// @brief It concatenates the two input paths
  /// @param first first path
  /// @param second second path
  /// @return Concatenated path
  static std::string
  combine_path(std::string const &first, std::string const &second);
};

#endif // NETWORK_BUTCHER_UTILITIES_H
