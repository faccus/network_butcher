#include "utilities.h"

namespace network_butcher::Utilities
{
  auto
  compute_memory_usage_from_enum(int input) -> Memory_Type
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
  parse_onnx_file(onnx::ModelProto &m, const std::string &model_path)
  {
    std::fstream input(model_path, std::ios::in | std::ios::binary);

    // File can be opened
    if (input.good())
      m.ParseFromIstream(&input);

    input.close();
  }


  void
  output_onnx_file(onnx::ModelProto const &m, const std::string &path)
  {
    std::fstream output(path, std::ios::out | std::ios::trunc | std::ios::binary);

    if (output.good())
      m.SerializeToOstream(&output);

    output.close();
  }


  auto
  parse_onnx_file(const std::string &model_path) -> onnx::ModelProto
  {
    if (!file_exists(model_path))
      throw std::runtime_error("The model in the specified path " + model_path + " doesn't exist");

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    onnx::ModelProto model;
    parse_onnx_file(model, model_path);
    return model;
  }


  auto
  split(const std::string &s, const std::string &delimiter) -> std::vector<std::string>
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


  auto
  combine_path(const std::string &first, const std::string &second) -> std::string
  {
    if (first.back() != '/' && second.front() != '/')
      return first + "/" + second;
    else if (first.back() == '/' && second.front() == '/')
      return first + second.substr(1);
    else
      return first + second;
  }

  auto
  file_exists(const std::string &name) -> bool
  {
    const std::filesystem::path p = name;
    return std::filesystem::exists(p);
  }

  auto
  directory_exists(const std::string &name) -> bool
  {
    const std::filesystem::path p = name;
    return std::filesystem::exists(p);
  }

  void
  file_delete(const std::string &path)
  {
    std::filesystem::remove_all(path);
  }

  void
  directory_delete(const std::string &path)
  {
    std::filesystem::remove_all(path);
  }

  void
  create_directory(const std::string &path)
  {
    if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path))
      {
        std::filesystem::create_directory(path);
      }
  }

  void
  ltrim(std::string &s)
  {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  }

  void
  rtrim(std::string &s)
  {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  }

  void
  ltrim(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  }

  void
  rtrim(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  }

  void
  trim(std::string &s)
  {
    ltrim(s);
    rtrim(s);
  }

  void
  trim(std::vector<std::string> &s)
  {
    ltrim(s);
    rtrim(s);
  }

  void
  to_lowercase(std::string &s)
  {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }

  void
  to_lowercase(std::vector<std::string> &vect)
  {
    for (auto &s : vect)
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }

  auto
  ltrim_copy(std::string s) -> std::string
  {
    ltrim(s);
    return s;
  }

  auto
  rtrim_copy(std::string s) -> std::string
  {
    rtrim(s);
    return s;
  }

  auto
  trim_copy(std::string s) -> std::string
  {
    trim(s);
    return s;
  }

  auto
  trim_copy(std::vector<std::string> s) -> std::vector<std::string>
  {
    trim(s);
    return s;
  }

  auto
  to_lowercase_copy(std::string s) -> std::string
  {
    to_lowercase(s);
    return s;
  }

  auto
  to_lowercase_copy(std::vector<std::string> s) -> std::vector<std::string>
  {
    to_lowercase(s);
    return s;
  }

  auto
  file_copy(std::string const &from, std::string const &to) -> bool
  {
    if (Utilities::file_exists(from))
      {
        std::filesystem::copy_file(from, to);
        return true;
      }

    return false;
  }

} // namespace network_butcher::Utilities