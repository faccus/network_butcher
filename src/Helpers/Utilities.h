//
// Created by faccus on 28/08/21.
//

#ifndef NETWORK_BUTCHER_UTILITIES_H
#define NETWORK_BUTCHER_UTILITIES_H

#include <filesystem>
#include <fstream>

#include "../Onnx_model/onnx.pb.h"
#include "Traits/Basic_traits.h"
#include "Traits/Type_info_traits.h"

#include "Types/Type_info.h"
#include "Types/Dense_tensor.h"



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
  parse_onnx_file(onnx::ModelProto &m, const std::string& model_path);

  /// Construct a ModelProto from an onnx file
  /// \param model_path Path to the .onnx file
  /// \return The constructed model
  onnx::ModelProto
  parse_onnx_file(const std::string& model_path);

  /// Check if a file exists
  /// \param name Path to the file
  /// \return True if it exists, false otherwise
  inline bool
  file_exists(const std::string &name)
  {
    const std::filesystem::path p = name ;
    return std::filesystem::exists(p);
  }


} // namespace utilities

#endif // NETWORK_BUTCHER_UTILITIES_H
