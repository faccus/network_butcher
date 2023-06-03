//
// Created by faccus on 28/08/21.
//

#ifndef NETWORK_BUTCHER_UTILITIES_H
#define NETWORK_BUTCHER_UTILITIES_H

#include <algorithm>
#include <numeric>

#include <filesystem>
#include <fstream>
#include <ranges>

#include "basic_traits.h"
#include "onnx.pb.h"

#include "dense_tensor.h"
#include "type_info.h"


namespace network_butcher::Utilities
{

  /// Simple function that will apply std::to_string, if available. If the std::to_string was not provided, print " "
  /// \tparam T The type of the object to convert to string
  /// \param obj The object of type T to be converted to a string
  /// \return The string
  template <typename T>
    requires requires(T const &a) { std::to_string(a); }
  auto
  custom_to_string(T const &obj) -> std::string
  {
    return std::to_string(obj);
  }

  /// Simple function that will apply std::to_string, if available. If the std::to_string was not provided, print " "
  /// \tparam T The type of the object to convert to string
  /// \param obj The object of type T to be converted to a string
  /// \return The string
  template <typename T>
  auto
  custom_to_string(T const &obj) -> std::string
  {
    return "";
  }

  /// Simple function that will apply std::to_string, if available. Specialized for pairs
  /// \tparam T The type of the object to convert to string
  /// \param obj The object of type T to be converted to a string
  /// \return The string
  template <typename T>
  auto
  custom_to_string(std::pair<T, T> const &obj) -> std::string
  {
    return custom_to_string(obj.first) + " " + custom_to_string(obj.second);
  }


  /// From onnx::TensorProto_DataType_*, it will return the size of the respective type in bytes
  /// \param input The onnx_type
  /// \return The memory usage of the type
  auto
  compute_memory_usage_from_enum(int input) -> Memory_Type;


  /// Construct a ModelProto from an onnx file
  /// \param m Reference to the model that will be constructed
  /// \param model_path Path to the .onnx file
  void
  parse_onnx_file(onnx::ModelProto &m, const std::string &model_path);


  /// Construct a ModelProto from an onnx file
  /// \param model_path Path to the .onnx file
  /// \return The constructed model
  auto
  parse_onnx_file(const std::string &model_path) -> onnx::ModelProto;


  /// Outputs an onnx file from the given model
  /// \param m Model to be exported
  /// \param path Path of the exported model
  void
  output_onnx_file(onnx::ModelProto const &m, const std::string &path);

  /// IT will copy the file from the source to the destination
  /// \param from The source path
  /// \param to The destination path
  /// \return True if the copy was successful, false otherwise
  auto
  file_copy(std::string const &from, std::string const &to) -> bool;


  /// Check if a file exists
  /// \param name Path to the file
  /// \return True if it exists, false otherwise
  auto
  file_exists(const std::string &name) -> bool;


  /// Check if a directory exists
  /// \param name Path to the directory
  /// \return True if it exists, false otherwise
  auto
  directory_exists(const std::string &name) -> bool;


  /// Deletes the file at the specified location
  /// \param path The path of the file
  void
  file_delete(std::string const &path);


  /// Deletes the directory at the specified location
  /// \param path The path of the directory
  void
  directory_delete(std::string const &path);


  /// Creates a directory with the given path
  /// \param path The path of the directory
  void
  create_directory(const std::string &path);


  /// Left trim for the input string (modifies the input string)
  /// \param s The input string
  void
  ltrim(std::string &s);


  /// Right trim for the input string (modifies the input string)
  /// \param s The input string
  void
  rtrim(std::string &s);


  /// Left trim for the input string vector
  /// \param vect The input string vector
  void
  ltrim(std::vector<std::string> &vect);


  /// Right trim for the input string vector
  /// \param vect The input string vector
  void
  rtrim(std::vector<std::string> &vect);


  /// Trim for the input string (modifies the input string)
  /// \param s The input string
  void
  trim(std::string &s);


  /// Trim for the input string vector
  /// \param s The input string vector
  void
  trim(std::vector<std::string> &s);


  /// Sets in lowercase the input string
  /// \param s The input string
  void
  to_lowercase(std::string &s);


  /// Sets in lowercase the input string vector
  /// \param vect The input string vector
  void
  to_lowercase(std::vector<std::string> &vect);


  /// Left trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  auto
  ltrim_copy(std::string s) -> std::string;


  /// Right trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  auto
  rtrim_copy(std::string s) -> std::string;


  /// Left trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  auto
  trim_copy(std::string s) -> std::string;


  /// Trim for the input string vector (returns the modified string vector)
  /// \param s The input string vector
  /// \return The modified string vector
  auto
  trim_copy(std::vector<std::string> s) -> std::vector<std::string>;


  /// It returns in lowercase the input string
  /// \param s The input string
  /// \return The modified string
  auto
  to_lowercase_copy(std::string s) -> std::string;


  /// It returns in lowercase the input string vector
  /// \param s The input string vector
  /// \return The modified string vector
  auto
  to_lowercase_copy(std::vector<std::string> s) -> std::vector<std::string>;


  /// It splits the input string in a vector (https://stackoverflow.com/a/46931770)
  /// \param s Input string vector
  /// \param delimiter The delimiter string
  /// \return The "splitted" string vector
  auto
  split(const std::string& s, const std::string& delimiter) -> std::vector<std::string>;


  /// It concatenates the two input paths
  /// \param first first path
  /// \param second second path
  /// \return Concatenated path
  auto
  combine_path(std::string const &first, std::string const &second) -> std::string;

} // namespace network_butcher::Utilities
#endif // NETWORK_BUTCHER_UTILITIES_H
