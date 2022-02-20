//
// Created by faccus on 28/08/21.
//

#include "Dense_tensor.h"

#include <utility>

Dense_tensor::Dense_tensor(type_info_id_type       in_type_id,
                           std::vector<shape_type> in_shape,
                           bool                    given)
  : Type_info()
  , type_id(in_type_id)
  , shape(std::move(in_shape))
{
  t_initialized = given;
}


Dense_tensor::Dense_tensor(const onnx::ValueInfoProto &info, bool given)
{
  const auto &type = info.type();
  name             = info.name();
  if (type.has_tensor_type())
    {
      type_id              = type.tensor_type().elem_type();
      const auto &in_shape = type.tensor_type().shape();

      for (int i = 0; i < in_shape.dim_size(); ++i)
        shape.push_back(in_shape.dim(i).dim_value());
    }

  t_initialized = given;
}

memory_type
Dense_tensor::compute_memory_usage() const
{
  memory_type num_entries = 1;
  for (auto &e : shape)
    num_entries *= e;

  return num_entries * utilities::compute_memory_usage_from_enum(type_id);
}