//
// Created by faccus on 28/08/21.
//

#include "../../include/Types/Dense_tensor.h"

#include <utility>


network_butcher_types::Dense_tensor::Dense_tensor(
  type_info_id_type       in_type_id,
  std::vector<shape_type> in_shape,
  bool                    given,
  bool                    constant)
  : Type_info()
  , type_id(in_type_id)
  , shape(std::move(in_shape))
{
  t_initialized  = given;
  this->constant = constant;
}


network_butcher_types::Dense_tensor::Dense_tensor(
  const onnx::ValueInfoProto &info,
  bool                        given,
  bool                        constant)
{
  const auto &type = info.type();
  name             = info.name();

  if (type.has_tensor_type())
    {
      type_id              = type.tensor_type().elem_type();
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

  t_initialized  = given;
  this->constant = constant;
}

network_butcher_types::Dense_tensor::Dense_tensor(const onnx::TensorProto &info,
                                                  bool given,
                                                  bool constant)
{
  name = info.name();

  if (info.IsInitialized())
    {
      type_id              = info.data_type();
      const auto &in_shape = info.dims();

      for (int i = 0; i < in_shape.size(); ++i)
        shape.push_back(in_shape[i]);
    }

  t_initialized  = given;
  this->constant = constant;
}

memory_type
network_butcher_types::Dense_tensor::compute_memory_usage() const
{
  memory_type num_entries = 1;
  for (auto &e : shape)
    num_entries *= e;

  return num_entries *
         network_butcher_utilities::compute_memory_usage_from_enum(type_id);
}
