//
// Created by faccus on 28/08/21.
//

#ifndef NETWORK_BUTCHER_DENSE_TENSOR_H
#define NETWORK_BUTCHER_DENSE_TENSOR_H

#include "Type_info.h"
#include "../Utilities.h"
#include <vector>

/// Dense tensor type of an onnx model
class Dense_tensor : public Type_info
{
private:
  int type_id = -1; // Maybe, it can be changed to the dimension of the type
  std::vector<int> shape;

public:
  Dense_tensor(int, const std::vector<int> &);
  Dense_tensor(const onnx::ValueInfoProto &);

  std::size_t
  compute_memory_usage() const override;
};

#endif // NETWORK_BUTCHER_DENSE_TENSOR_H
/*
 *
 * */