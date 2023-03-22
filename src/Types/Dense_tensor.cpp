//
// Created by faccus on 28/08/21.
//

#include "Dense_tensor.h"

namespace network_butcher::types
{
  Dense_tensor::Dense_tensor(type_info_id_type in_type_id, std::vector<shape_type> in_shape, bool given, bool constant)
    : Type_info(given, constant)
    , type_tensor_memory(Utilities::compute_memory_usage_from_enum(in_type_id))
    , shape(std::move(in_shape))
  {}


  Dense_tensor::Dense_tensor(const onnx::ValueInfoProto &info, bool given, bool constant)
    : Type_info(given, constant, info.name())
  {
    const auto &type = info.type();

    if (type.has_tensor_type())
      {
        type_tensor_memory   = Utilities::compute_memory_usage_from_enum(type.tensor_type().elem_type());
        const auto &in_shape = type.tensor_type().shape();

        for (int i = 0; i < in_shape.dim_size(); ++i)
          {
            auto const &tm = in_shape.dim(i);

            if (!tm.has_dim_value())
              shape.push_back(1);
            else
              shape.push_back(tm.dim_value());
          }
      }
  }


  Dense_tensor::Dense_tensor(const onnx::TensorProto &info, bool given, bool constant)
    : Type_info(given, constant, info.name())
  {
    if (info.IsInitialized())
      {
        type_tensor_memory   = Utilities::compute_memory_usage_from_enum(info.data_type());
        const auto &in_shape = info.dims();

        for (int i = 0; i < in_shape.size(); ++i)
          shape.push_back(in_shape[i]);
      }
  }


  memory_type
  Dense_tensor::compute_memory_usage() const
  {
    memory_type num_entries = 1;
    for (auto &e : shape)
      num_entries *= e;

    return num_entries * type_tensor_memory;
  }


  shape_type
  Dense_tensor::compute_shape_volume() const
  {
    if (shape.size() == 0)
      return 0;

    memory_type num_entries = 1;
    for (auto &e : shape)
      num_entries *= e;

    return num_entries;
  }

} // namespace network_butcher::types