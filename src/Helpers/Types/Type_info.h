//
// Created by faccus on 25/08/21.
//

#ifndef NETWORK_BUTCHER_TYPE_INFO_H
#define NETWORK_BUTCHER_TYPE_INFO_H

#include <string>

/// Generic type contained in a onnx model
class Type_info
{
protected:
  std::string name;
public:

  const std::string get_name() {
    return name;
  };

  Type_info() = default;

  virtual std::size_t
  compute_memory_usage() const {
    return -1;
  };


  virtual ~Type_info() = default;
};


#endif // NETWORK_BUTCHER_TYPE_INFO_H
