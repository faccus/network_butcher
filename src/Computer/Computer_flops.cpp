//
// Created by faccus on 12/20/22.
//
#include "../../include/Computer/Computer_flops.h"
std::unordered_map<std::string,
                   std::function<std::pair<double, std::size_t>(const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &)>>
network_butcher_computer::Computer_flops::generate_maps_flops_func()
{
  const std::size_t ADD_MACS   = 1;
  const std::size_t EXP_MACS  = 16;
  const std::size_t LOG_MACS  = 16;
  const std::size_t SQRT_MACS = 4;
  const std::size_t POW_MACS  = 32;
  const std::size_t MUL_MACS  = 1;
  const std::size_t DIV_MACS  = 2;
  const std::size_t CMP_MACS  = 1;
  const std::size_t SIN_MACS  = 14;
  const std::size_t COS_MACS  = 14;

  const std::size_t RESIZE_LINEAR_MACS = 4;
  const std::size_t RESIZE_CUBIC_MACS = 8;

  std::unordered_map<std::string,
                     std::function<std::pair<double, std::size_t>(
                       const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &)>>
    res;

  res["conv"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    double macs = 0;
    std::size_t params = 0;

    auto const num_outputs = content.get_output().size();
    auto const num_inputs = content.get_input().size();

    if(num_outputs == 1 && num_inputs == 1) {
        auto const &input_type = content.get_input().begin()->second;
        auto const &output_type = content.get_output().begin()->second;

        auto const &attributes = content.get_attributes();

        auto const &kernel = (attributes.find("kernel_shape")->second);
        auto vol_kernel = get_volume(kernel);
        params += vol_kernel * input_type->get_shape()[1] * (output_type->get_shape()[1] + 1);

        if(kernel.size() == 1) {
            params *= kernel[0].get_int();
            vol_kernel *= kernel[0].get_int();
          }

        auto const out_vol = output_type->compute_shape_volume();
        macs               = out_vol * (vol_kernel + ADD_MACS);
      }

    return std::pair{macs, params};
  };

  res["convtranspose"] = res["conv"];

  auto const pool_gen = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &output_type = content.get_output().begin()->second;
    auto const &attributes  = content.get_attributes();
    auto const  out_vol     = output_type->compute_shape_volume();

    auto const &kernel     = (attributes.find("kernel_shape")->second);
    auto        vol_kernel = get_volume(kernel);


    macs = out_vol * CMP_MACS * kernel[0].get_int();
    if (kernel.size() == 2)
      macs *= kernel[1].get_int();

    return std::pair{macs, params};
  };
  auto const op_gen = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content,
                         std::size_t                                                                      op_mac) {
    double      macs   = 0;
    std::size_t params = 0;

    std::size_t ratio;

    if (content.get_input().size() < 2)
      ratio = 1;
    else
      ratio = content.get_input().size() - 1;

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    macs = out_vol * ratio * op_mac;

    return std::pair{macs, params};
  };

  res["add"] = [op_gen](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return op_gen(content, ADD_MACS);
  };

  res["sum"] = res["add"];
  res["abs"] = [op_gen](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return op_gen(content, CMP_MACS);
  };
  res["neg"] = [op_gen](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return op_gen(content, CMP_MACS);
  };
  res["sub"] = [op_gen](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return op_gen(content, ADD_MACS);
  };

  res["resize"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    double macs = 0;
    std::size_t params = 0;

    auto const &output_type = content.get_output().begin()->second;
    auto const out_vol = output_type->compute_shape_volume();

    auto const &attributes = content.get_attributes();
    auto const &mode = attributes.find("mode")->second[0].get_string();

    if (mode == "nearest")
      {
        macs = 0;
      }
    else if (mode == "linear")
      {
        macs = out_vol * RESIZE_LINEAR_MACS;
      }
    else if (mode == "cubic")
      {
        macs = out_vol * RESIZE_CUBIC_MACS;
      }

    return std::pair{macs, params};
  };
  res["scatternd"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    return std::pair<double, std::size_t>(0., 0);
  };

  res["argmax"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    return std::pair<double, std::size_t>(0., 0);
  };
  res["upsample"] = res["argmax"];
  res["expand"] = res["upsample"];
  res["tile"] = res["expand"];

  res["gru"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    double macs = 0;
    std::size_t params = 0;

    // WIP

    return std::pair{macs, params};
  };
  res["lstm"] = res["gru"];

  res["maxpool"] = pool_gen;
  res["averagepool"] = pool_gen;

  res["reducemean"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * ADD_MACS, params};
  };
  res["reduceprod"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * MUL_MACS, params};
  };
  res["reducel2"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * (ADD_MACS + SQRT_MACS), params};
  };
  res["reducesum"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    double macs = 0.;

    // WIP

    return std::pair{macs, params};
  };
  res["reducemin"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * CMP_MACS, params};
  };
  res["reducemax"] = res["reducemin"];

  res["scan"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    double macs = 0.;

    // WIP

    return std::pair{macs, params};
  };
  res["compress"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    double macs = 0.;

    // WIP

    return std::pair{macs, params};
  };
  res["nonzero"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &output_type = content.get_output().begin()->second;

    return std::pair{output_type->compute_shape_volume() * CMP_MACS, params};
  };
  res["less"] = res["nonzero"];

  res["lessorequal"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    double macs = 0.;

    // WIP

    return std::pair{macs, params};
  };
  res["not"] = res["lessorequal"];
  res["and"] = res["not"];


  // https://github.com/ThanatosShinji/onnx-tool/blob/25f46e2efd0243e2dffb958a722cd07f7cdf3438/onnx_tool/node_profilers.py#L751
}

std::size_t
network_butcher_computer::Computer_flops::get_volume(const std::vector<network_butcher_types::DynamicType> &vect)
{
  if(vect.empty())
    return 0;

  std::size_t res = 1;
  if(vect.size() == 1) {
      auto const &value = vect[0];

      if(value.has_int())
        return value.get_int();

      if(value.has_ints()) {
          for(auto const &el : value.get_ints())
            res *= el;

          return res;
        }

      return 0;
    }

  for(auto const &el : vect) {
      if(el.has_int()) {
          res *= el.get_int();
        }
      else {
          return 0;
        }
    }

  return res;
}

std::pair<double, std::size_t>
network_butcher_computer::Computer_flops::compute_macs_flops(
  const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content)
{
  return std::pair<double, std::size_t>();
}
