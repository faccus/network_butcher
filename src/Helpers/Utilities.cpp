//
// Created by faccus on 28/08/21.
//
#include "Utilities.h"
#include <fstream>

std::size_t
utilities::compute_memory_usage_from_enum(int input) {
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
            return -1;
        }
    }
  else
    return -1;
}

void utilities::parse_onnx_file(onnx::ModelProto & m, std::string model_path)
{
  std::ifstream in(model_path, std::ios::binary);

  if(in.good())
    {
      in.seekg(0, std::ios_base::end);
      std::size_t len = in.tellg();

      char *ret = new char[len];

      in.seekg(0, std::ios_base::beg);
      in.read(ret, len);
      in.close();

      m.ParseFromArray(ret, len);
    }
}

onnx::ModelProto
utilities::parse_onnx_file(std::string model_path)
{
  onnx::ModelProto model;
  parse_onnx_file(model, model_path);
  return model;
}
