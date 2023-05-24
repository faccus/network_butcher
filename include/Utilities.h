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


namespace network_butcher::Utilities
{
  /// From onnx::TensorProto_DataType_*, it will return the size of the respective type in bytes
  /// \param input The onnx_type
  /// \return The memory usage of the type
  memory_type
  compute_memory_usage_from_enum(type_info_id_type input);


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
  bool
  file_exists(const std::string &name);


  /// Check if a directory exists
  /// \param name Path to the directory
  /// \return True if it exists, false otherwise
  bool
  directory_exists(const std::string &name);


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
  std::string
  ltrim_copy(std::string s);


  /// Right trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  std::string
  rtrim_copy(std::string s);


  /// Left trim for the input string (returns the modified string)
  /// \param s The input string
  /// \return The modified string
  std::string
  trim_copy(std::string s);


  /// Trim for the input string vector (returns the modified string vector)
  /// \param s The input string vector
  /// \return The modified string vector
  std::vector<std::string>
  trim_copy(std::vector<std::string> s);


  /// It returns in lowercase the input string
  /// \param s The input string
  /// \return The modified string
  std::string
  to_lowercase_copy(std::string s);


  /// It returns in lowercase the input string vector
  /// \param s The input string vector
  /// \return The modified string vector
  std::vector<std::string>
  to_lowercase_copy(std::vector<std::string> s);


  /// It splits the input string in a vector (https://stackoverflow.com/a/46931770)
  /// \param s Input string vector
  /// \param delimiter The delimiter string
  /// \return The "splitted" string vector
  std::vector<std::string>
  split(std::string s, std::string delimiter);


  /// It concatenates the two input paths
  /// \param first first path
  /// \param second second path
  /// \return Concatenated path
  std::string
  combine_path(std::string const &first, std::string const &second);

  /// Based on the compiler pre-processor PARALLEL (associated to the same setting in the CMakeList file), it will
  /// apply the std::transform function to the given arguments with either a parallel unsequenced policy or with
  /// sequential policy
  template <typename... Args>
  void
  potentially_par_unseq_transform(Args &&...args)
  {
#if PARALLEL

    std::transform(std::execution::par_unseq, std::forward<Args>(args)...);

#else
    std::transform(args...);
#endif
  };

  /// Based on the compiler pre-processor PARALLEL (associated to the same setting in the CMakeList file), it will
  /// apply the std::reduce function to the given arguments with either a parallel unsequenced policy or with
  /// sequential policy
  template <typename... Args>
  auto
  potentially_par_unseq_reduce(Args &&...args)
  {
#if PARALLEL
    return std::reduce(std::execution::par_unseq, std::forward<Args>(args)...);
#else
    return std::reduce(args...);
#endif
  };


} // namespace network_butcher::Utilities
#endif // NETWORK_BUTCHER_UTILITIES_H
