#include "dense_tensor.h"

namespace network_butcher::types
{
  Dense_tensor::Dense_tensor(int in_type_id, std::vector<Onnx_Element_Shape_Type> in_shape, bool given, bool constant)
    : Type_Info(given, constant)
    , type_tensor_memory(Utilities::compute_memory_usage_from_enum(in_type_id))
    , shape(std::move(in_shape))
  {}


  Dense_tensor::Dense_tensor(const onnx::ValueInfoProto &info, bool given, bool constant)
    : Type_Info(given, constant, info.name())
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
    : Type_Info(given, constant, info.name())
  {
    if (info.IsInitialized())
      {
        type_tensor_memory   = Utilities::compute_memory_usage_from_enum(info.data_type());
        const auto &in_shape = info.dims();

        for (long i : in_shape)
          shape.push_back(i);
      }
  }


  auto
  Dense_tensor::compute_memory_usage() const -> Memory_Type
  {
    Memory_Type num_entries = 1;
    for (auto const &e : shape)
      num_entries *= e;

    return num_entries * type_tensor_memory;
  }


  auto
  Dense_tensor::compute_shape_volume() const -> Onnx_Element_Shape_Type
  {
    if (shape.empty())
      return 0;

    Memory_Type num_entries = 1;
    for (auto const &e : shape)
      num_entries *= e;

    return num_entries;
  }

} // namespace network_butcher::types