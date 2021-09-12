# Onnx protobuf model download and compilation
set(onnx_model_file_url "https://raw.githubusercontent.com/onnx/onnx/master/onnx/onnx.proto3")

set(onnx_model_directory ${CMAKE_SOURCE_DIR}/src/Onnx_model)
set(onnx_model_schema_name onnx)
set(onnx_model_schema_full_path ${onnx_model_directory}/onnx)

file(DOWNLOAD ${onnx_model_file_url} ${onnx_model_schema_full_path})

FILE(REMOVE ${onnx_model_schema_full_path}.pb.h)
FILE(REMOVE ${onnx_model_schema_full_path}.pb.cc)

FILE(TO_NATIVE_PATH ${onnx_model_directory} onnx_model_directory_native)
FILE(TO_NATIVE_PATH ${onnx_model_schema_full_path} proto_native)

EXECUTE_PROCESS(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${onnx_model_directory_native} --cpp_out=${onnx_model_directory_native} ${proto_native})