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

  auto const pwnbase = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content,
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
  auto const zeros = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    return std::pair<double, std::size_t>(0., 0);
  };

  res["add"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, ADD_MACS);
  };
  res["sum"] = res["add"];
  res["sub"] = res["add"];

  res["abs"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, CMP_MACS);
  };
  res["neg"] = res["abs"];

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
  res["scatternd"] = zeros;

  /* WIP
  res["argmax"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    return std::pair<double, std::size_t>(0., 0);
  };
  */
  res["upsample"] = zeros;
  res["expand"] = zeros;
  res["tile"] = zeros;

  /* WIP
  res["gru"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    double macs = 0;
    std::size_t params = 0;

    // WIP

    return std::pair{macs, params};
  };
  res["lstm"] = res["gru"];
  */

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
  res["maxpool"] = pool_gen;
  res["averagepool"] = pool_gen;

  res["reducemean"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * ADD_MACS, params};
  };
  res["reduceprod"] = res["reducemean"];
  res["reducesum"] = res["reducemean"];

  res["reducel2"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * (ADD_MACS + SQRT_MACS), params};
  };

  res["reducemin"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * CMP_MACS, params};
  };
  res["reducemax"] = res["reducemin"];

  res["scan"] = zeros;
  res["compress"] = zeros;
  res["nonzero"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    std::size_t params = 0;
    auto const &output_type = content.get_output().begin()->second;

    return std::pair{output_type->compute_shape_volume() * CMP_MACS, params};
  };
  res["less"] = res["nonzero"];
  res["lessorequal"] = res["less"];

  res["not"] = zeros;
  res["and"] = zeros;
  res["where"] = zeros;

  res["min"] = res["add"];
  res["max"] = res["min"];
  res["equal"] = res["min"];
  res["greater"] = res["min"];

  res["roialign"] = zeros;
  res["scatterelements"] = zeros;

  res["hardmax"] = res["abs"];

  res["globalaveragepool"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    auto const &input_type = content.get_input().begin()->second;
    auto const  in_vol     = input_type->compute_shape_volume();

    macs = in_vol * ADD_MACS + out_vol * DIV_MACS;

    return std::pair{macs, params};
  };

  res["categorymapper"] = res["add"];
  res["topk"] = zeros;

  res["relu"] = res["abs"];
  res["relu"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, CMP_MACS + MUL_MACS);
  };
  res["clip"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, CMP_MACS * 2);
  };
  res["relu6"] = res["clip"];

  res["lnr"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &size = content.get_attributes().find("size")->second.front().get_int();

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    macs = (DIV_MACS + EXP_MACS + ADD_MACS + size * MUL_MACS) * out_vol;

    return std::pair{macs, params};
  };

  res["gemm"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &input_type = content.get_input().begin()->second;
    auto const  in_vol     = input_type->compute_shape_volume();

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    auto const compute = [&params, &macs, in_vol, out_vol](std::shared_ptr<network_butcher_types::Type_info> const &weights,
                                                           std::shared_ptr<network_butcher_types::Type_info> const &bias,
                                                           bool params_non_zero = true) {
      auto const weight_vol = weights->compute_shape_volume();
      auto const bias_vol = bias->compute_shape_volume();

      if(params_non_zero)
        params = weight_vol + bias_vol;

      macs = in_vol * weights->get_shape()[0] + out_vol * ADD_MACS;
    };

    if(content.get_input().size() == 1) {
        auto const &first_parameters  =    content.get_parameters().cbegin();
        auto const &second_parameters = (++content.get_parameters().cbegin());

        if(second_parameters->second->get_shape().size() > first_parameters->second->get_shape().size()) {
            compute(second_parameters->second, first_parameters->second);
          } else {
            compute(first_parameters->second, second_parameters->second);
          }
      } else if(content.get_input().size() == 2) {
        auto const &first_parameters  = content.get_input().cbegin();
        auto const &second_parameters = content.get_parameters().cbegin();

        if(second_parameters->second->get_shape().size() > first_parameters->second->get_shape().size()) {
            compute(second_parameters->second, first_parameters->second);
          } else {
            compute(first_parameters->second, second_parameters->second);
          }
      } else {
        auto const &first_parameters  =   content.get_input().cbegin();
        auto const &second_parameters = ++content.get_input().cbegin();

        if(second_parameters->second->get_shape().size() > first_parameters->second->get_shape().size()) {
            compute(second_parameters->second, first_parameters->second);
          } else {
            compute(first_parameters->second, second_parameters->second);
          }
      }

    return std::pair{macs, params};
  };

  res["matmult"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &input_type = content.get_input().begin()->second;
    auto const  in_vol     = input_type->compute_shape_volume();

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    auto const compute =
      [&params, &macs, in_vol, out_vol](std::shared_ptr<network_butcher_types::Type_info> const &weights,
                                        std::shared_ptr<network_butcher_types::Type_info> const &bias,
                                        bool params_non_zero = true) {
        auto const weight_vol = weights->compute_shape_volume();
        auto const bias_vol   = bias->compute_shape_volume();

        if (params_non_zero)
          params = weight_vol + bias_vol;

        macs = in_vol * weights->get_shape().back() + out_vol * ADD_MACS;
      };

    if (content.get_input().size() == 1)
      {
        auto const &first_parameters  = content.get_parameters().cbegin();
        auto const &second_parameters = (++content.get_parameters().cbegin());

        if (second_parameters->second->get_shape().size() > first_parameters->second->get_shape().size())
          {
            compute(second_parameters->second, first_parameters->second);
          }
        else
          {
            compute(first_parameters->second, second_parameters->second);
          }
      }
    else if (content.get_input().size() == 2)
      {
        auto const &first_parameters  = content.get_input().cbegin();
        auto const &second_parameters = content.get_parameters().cbegin();

        if (second_parameters->second->get_shape().size() > first_parameters->second->get_shape().size())
          {
            compute(second_parameters->second, first_parameters->second);
          }
        else
          {
            compute(first_parameters->second, second_parameters->second);
          }
      }
    else
      {
        auto const &first_parameters  = content.get_input().cbegin();
        auto const &second_parameters = ++content.get_input().cbegin();

        if (second_parameters->second->get_shape().size() > first_parameters->second->get_shape().size())
          {
            compute(second_parameters->second, first_parameters->second);
          }
        else
          {
            compute(first_parameters->second, second_parameters->second);
          }
      }

    return std::pair{macs, params};
  };
  res["matmulinteger"] = res["matmult"];

  res["shape"] = zeros;
  res["gather"] = zeros;
  res["constant"] = zeros;
  res["unsqueeze"] = zeros;
  res["squeeze"] = zeros;
  res["concat"] = zeros;
  res["reshape"] = zeros;
  res["onehot"] = zeros;
  res["nonmaxsuppression"] = zeros;
  res["identity"] = zeros;
  res["erf"] = zeros;
  res["dropout"] = zeros;
  res["pad"] = zeros;
  res["split"] = zeros;
  res["transpose"] = zeros;
  res["constantofshape"] = zeros;
  res["batchnormalization"] = zeros;
  res["slice"] = zeros;
  res["cast"] = zeros;
  res["flatten"] = zeros;

  res["exp"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, EXP_MACS);
  };
  res["log"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, LOG_MACS);
  };
  res["softmax"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, EXP_MACS + DIV_MACS);
  };
  res["tanh"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    auto pair = pwnbase(content, MUL_MACS);

    return std::pair{2 * pair.first, pair.second};
  };
  res["mul"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, MUL_MACS);
  };
  res["imagescaler"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, ADD_MACS + MUL_MACS);
  };
  res["instancenormalization"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, ADD_MACS + MUL_MACS + ADD_MACS + DIV_MACS);
  };
  res["sqrt"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, SQRT_MACS);
  };
  res["pow"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, POW_MACS);
  };
  res["sin"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, SIN_MACS);
  };
  res["cos"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, COS_MACS);
  };
  res["div"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, DIV_MACS);
  };
  res["floor"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, CMP_MACS);
  };
  res["ceil"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, CMP_MACS);
  };
  res["reciprocal"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, DIV_MACS);
  };
  res["hardsigmoid"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, MUL_MACS + ADD_MACS + CMP_MACS * 2);
  };
  res["leakyrelu"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, MUL_MACS + CMP_MACS);
  };
  res["dequantizelinear"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, MUL_MACS + ADD_MACS);
  };
  res["quantizelinear"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, MUL_MACS + ADD_MACS);
  };


  /* WIP
  res["einsum"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, MUL_MACS + CMP_MACS);
  };
   res["QLinearMatMul"] QLinearConv,
  */

  res["range"] = zeros;
  res["arrayfeatureextractor"] = zeros;
  res["zipmap"] = zeros;

  res["sigmoid"] = res["exp"];
  res["cumsum"]  = res["sum"];
  return res;

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
