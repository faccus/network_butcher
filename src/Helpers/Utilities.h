//
// Created by faccus on 28/08/21.
//

#ifndef NETWORK_BUTCHER_UTILITIES_H
#define NETWORK_BUTCHER_UTILITIES_H

#include <sys/stat.h>

#include "../Onnx_model/onnx.pb.h"
#include "Types/Type_info.h"
#include "Types/Dense_tensor.h"

namespace utilities
{
  std::size_t
  compute_memory_usage_from_enum(int);

  void
  parse_onnx_file(onnx::ModelProto &m, std::string model_path);

  onnx::ModelProto
  parse_onnx_file(std::string model_path);

  inline bool
  file_exists(const std::string &name)
  {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
  }


} // namespace utilities

#endif // NETWORK_BUTCHER_UTILITIES_H
