# Onnx protobuf model download and compilation
set(onnx_model_file_url "https://raw.githubusercontent.com/onnx/onnx/master/onnx/onnx.proto3")

if( EXISTS onnx)
    FILE(REMOVE onnx)
endif()

file(DOWNLOAD ${onnx_model_file_url} onnx)

FILE(REMOVE onnx_old.h)
FILE(REMOVE onnx_old.cc)

if( EXISTS onnx.pb.h)
    FILE(RENAME onnx.pb.h onnx_old.h)
endif()

if(EXISTS onnx.pb.cc)
    FILE(RENAME onnx.pb.cc onnx_old.cc)
endif()