//
// Created by faccus on 28/08/21.
//
#include "Utilities.h"

namespace network_butcher
{
  memory_type
  Utilities::compute_memory_usage_from_enum(type_info_id_type input)
  {
    if (onnx::TensorProto_DataType_IsValid(input))
      {
        switch (input)
          {
            case onnx::TensorProto_DataType_FLOAT:
              return sizeof(float);
            case onnx::TensorProto_DataType_UINT8:
              return sizeof(uint8_t);
            case onnx::TensorProto_DataType_INT8:
              return sizeof(int8_t);
            case onnx::TensorProto_DataType_UINT16:
              return sizeof(uint16_t);
            case onnx::TensorProto_DataType_INT16:
              return sizeof(int16_t);
            case onnx::TensorProto_DataType_INT32:
              return sizeof(int32_t);
            case onnx::TensorProto_DataType_INT64:
              return sizeof(int64_t);
            case onnx::TensorProto_DataType_STRING:
              return sizeof(std::string);
            case onnx::TensorProto_DataType_BOOL:
              return sizeof(bool);
            case onnx::TensorProto_DataType_FLOAT16:
              return 2; // https://www.ibm.com/docs/en/zos/2.4.0?topic=definitions-cc-data-type
            case onnx::TensorProto_DataType_DOUBLE:
              return sizeof(double);
            case onnx::TensorProto_DataType_UINT32:
              return sizeof(uint32_t);
            case onnx::TensorProto_DataType_UINT64:
              return sizeof(uint64_t);
            case onnx::TensorProto_DataType_COMPLEX64:
              return sizeof(int32_t) * 2;
            case onnx::TensorProto_DataType_COMPLEX128:
              return sizeof(int64_t) * 2;
            case onnx::TensorProto_DataType_BFLOAT16:
              return 2; // https://www.ibm.com/docs/en/zos/2.4.0?topic=definitions-cc-data-type
            default:
              return 0;
          }
      }
    else
      return 0;
  }


  void
  Utilities::parse_onnx_file(onnx::ModelProto &m, const std::string &model_path)
  {
    std::fstream input(model_path, std::ios::in | std::ios::binary);

    // File can be opened
    if (input.good())
      m.ParseFromIstream(&input);
  }


  void
  Utilities::output_onnx_file(onnx::ModelProto const &m, const std::string &path)
  {
    std::fstream output(path, std::ios::out | std::ios::trunc | std::ios::binary);

    if (output.good())
      m.SerializeToOstream(&output);
  }


  onnx::ModelProto
  Utilities::parse_onnx_file(const std::string &model_path)
  {
    if (!network_butcher::Utilities::file_exists(model_path))
      throw std::runtime_error("The model in the specified path " + model_path + " doesn't exist");

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    onnx::ModelProto model;
    parse_onnx_file(model, model_path);
    return model;
  }


  std::vector<std::string>
  Utilities::split(std::string s, std::string delimiter)
  {
    size_t                   pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string              token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
      {
        token     = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
      }

    auto end = s.substr(pos_start);

    if (!end.empty())
      res.push_back(std::move(end));
    return res;
  }


  std::string
  Utilities::combine_path(const std::string &first, const std::string &second)
  {
    if (first.back() != '/' && second.front() != '/')
      return first + "/" + second;
    else if (first.back() == '/' && second.front() == '/')
      return first + second.substr(1);
    else
      return first + second;
  }

} // namespace network_butcher