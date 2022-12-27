//
// Created by faccus on 12/20/22.
//
#include "../../include/Computer/Computer_flops.h"

bool network_butcher_computer::Computer_flops::factory_initialized = false;

void
network_butcher_computer::Computer_flops::generate_maps_flops_func()
{
  // https://github.com/ThanatosShinji/onnx-tool/blob/25f46e2efd0243e2dffb958a722cd07f7cdf3438/onnx_tool/node_profilers.py

  const std::size_t ADD_MACS  = 1;
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
  const std::size_t RESIZE_CUBIC_MACS  = 8;

  auto &factory = FactoryType::Instance();


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
  auto const zeros = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return std::pair<double, std::size_t>(0., 0);
  };
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


  factory.add("conv", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const num_outputs = content.get_output().size();
    auto const num_inputs  = content.get_input().size();

    if (num_outputs == 1 && num_inputs == 1)
      {
        auto const &input_type  = content.get_input().begin()->second;
        auto const &output_type = content.get_output().begin()->second;

        auto const &attributes = content.get_attributes();

        auto const &kernel     = (attributes.find("kernel_shape")->second);
        auto        vol_kernel = get_volume(kernel);
        params += vol_kernel * input_type->get_shape()[1] * (output_type->get_shape()[1] + 1);

        if (kernel.size() == 1)
          {
            params *= kernel[0].get_int();
            vol_kernel *= kernel[0].get_int();
          }

        auto const out_vol = output_type->compute_shape_volume();
        macs               = out_vol * (vol_kernel + ADD_MACS);
      }

    return std::pair{macs, params};
  });
  factory.add("convtranspose", factory.get("add"));
  factory.add("add",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, ADD_MACS);
              });

  factory.add("sum", factory.get("add"));
  factory.add("cumsum", factory.get("sum"));
  factory.add("sub", factory.get("add"));
  factory.add("min", factory.get("add"));
  factory.add("categorymapper", factory.get("add"));
  factory.add("max", factory.get("min"));
  factory.add("equal", factory.get("min"));
  factory.add("greater", factory.get("min"));

  factory.add("abs",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, CMP_MACS);
              });
  factory.add("neg", factory.get("abs"));
  factory.add("hardmax", factory.get("abs"));
  factory.add("relu", factory.get("abs"));

  /*
  factory.add("scatternd", zeros);
  factory.add("upsample", zeros);
  factory.add("expand", zeros);
  factory.add("tile", zeros);
  factory.add("scan", zeros);
  factory.add("compress", zeros);
  factory.add("not", zeros);
  factory.add("and", zeros);
  factory.add("where", zeros);
  factory.add("roialign", zeros);
  factory.add("scatterelements", zeros);
  factory.add("topk", zeros);
  factory.add("shape", zeros);
  factory.add("gather", zeros);
  factory.add("constant", zeros);
  factory.add("unsqueeze", zeros);
  factory.add("squeeze", zeros);
  factory.add("concat", zeros);
  factory.add("reshape", zeros);
  factory.add("onehot", zeros);
  factory.add("nonmaxsuppression", zeros);
  factory.add("identity", zeros);
  factory.add("erf", zeros);
  factory.add("dropout", zeros);
  factory.add("pad", zeros);
  factory.add("split", zeros);
  factory.add("transpose", zeros);
  factory.add("constantofshape", zeros);
  factory.add("batchnormalization", zeros);
  factory.add("slice", zeros);
  factory.add("cast", zeros);
  factory.add("flatten", zeros);
  factory.add("range", zeros);
  factory.add("arrayfeatureextractor", zeros);
  factory.add("zipmap", zeros);
*/

  factory.add("maxpool", pool_gen);
  factory.add("averagepool", pool_gen);

  factory.add("resize", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    auto const &attributes = content.get_attributes();
    auto const &mode       = attributes.find("mode")->second[0].get_string();

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
  });

  factory.add("reducemean",
              [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                std::size_t params     = 0;
                auto const &input_type = content.get_input().begin()->second;

                return std::pair{input_type->compute_shape_volume() * ADD_MACS, params};
              });
  factory.add("reduceprod", factory.get("reducemean"));
  factory.add("reducesum", factory.get("reducemean"));

  factory.add("reducel2", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    std::size_t params     = 0;
    auto const &input_type = content.get_input().begin()->second;

    return std::pair{input_type->compute_shape_volume() * (ADD_MACS + SQRT_MACS), params};
  });
  factory.add("reducemin",
              [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                std::size_t params     = 0;
                auto const &input_type = content.get_input().begin()->second;

                return std::pair{input_type->compute_shape_volume() * CMP_MACS, params};
              });
  factory.add("reducemax", factory.get("reducemin"));

  factory.add("nonzero", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    std::size_t params      = 0;
    auto const &output_type = content.get_output().begin()->second;

    return std::pair{output_type->compute_shape_volume() * CMP_MACS, params};
  });
  factory.add("less", factory.get("nonzero"));
  factory.add("lessorequal", factory.get("less"));

  factory.add("resize", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    auto const &input_type = content.get_input().begin()->second;
    auto const  in_vol     = input_type->compute_shape_volume();

    macs = in_vol * ADD_MACS + out_vol * DIV_MACS;

    return std::pair{macs, params};
  });

  factory.add("prelu",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, CMP_MACS + MUL_MACS);
              });

  factory.add("clip",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, CMP_MACS * 2);
              });
  factory.add("relu6", factory.get("clip"));

  factory.add("lnr", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    double      macs   = 0;
    std::size_t params = 0;

    auto const &output_type = content.get_output().begin()->second;
    auto const  out_vol     = output_type->compute_shape_volume();

    auto const &input_type = content.get_input().begin()->second;
    auto const  in_vol     = input_type->compute_shape_volume();

    macs = in_vol * ADD_MACS + out_vol * DIV_MACS;

    return std::pair{macs, params};
  });

  factory.add("gemm", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
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

        macs = in_vol * weights->get_shape()[0] + out_vol * ADD_MACS;
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
  });

  factory.add("matmult", [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
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
  });
  factory.add("matmulinteger", factory.get("matmult"));


  factory.add("exp",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, EXP_MACS);
              });
  factory.add("sigmoid", factory.get("exp"));

  factory.add("log",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, LOG_MACS);
              });

  factory.add("softmax",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, EXP_MACS + DIV_MACS);
              });

  factory.add("tanh",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                auto pair = pwnbase(content, MUL_MACS);

                return std::pair{2 * pair.first, pair.second};
              });

  factory.add("mul",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, MUL_MACS);
              });

  factory.add("imagescaler",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, ADD_MACS + MUL_MACS);
              });

  factory.add("instancenormalization",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, ADD_MACS + MUL_MACS + ADD_MACS + DIV_MACS);
              });

  factory.add("sqrt",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, SQRT_MACS);
              });

  factory.add("pow",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, POW_MACS);
              });

  factory.add("sin",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, SIN_MACS);
              });

  factory.add("cos",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, COS_MACS);
              });

  factory.add("div",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, DIV_MACS);
              });

  factory.add("floor",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, CMP_MACS);
              });

  factory.add("ceil",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, CMP_MACS);
              });

  factory.add("reciprocal",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, DIV_MACS);
              });

  factory.add("hardsigmoid",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, MUL_MACS + ADD_MACS + CMP_MACS * 2);
              });

  factory.add("leakyrelu",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, MUL_MACS + CMP_MACS);
              });

  factory.add("dequantizelinear",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, MUL_MACS + ADD_MACS);
              });

  factory.add("quantizelinear",
              [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
                return pwnbase(content, MUL_MACS + ADD_MACS);
              });


  /* WIP
  res["argmax"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    return std::pair<double, std::size_t>(0., 0);
  };
  */

  /* WIP
  res["gru"] = [](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> & content) {
    double macs = 0;
    std::size_t params = 0;

    // WIP

    return std::pair{macs, params};
  };
  res["lstm"] = res["gru"];
  */


  /* WIP
  res["einsum"] = [pwnbase](const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content) {
    return pwnbase(content, MUL_MACS + CMP_MACS);
  };
   res["QLinearMatMul"] QLinearConv,
  */
}

std::size_t
network_butcher_computer::Computer_flops::get_volume(const std::vector<network_butcher_types::DynamicType> &vect)
{
  if (vect.empty())
    return 0;

  std::size_t res = 1;
  if (vect.size() == 1)
    {
      auto const &value = vect[0];

      if (value.has_int())
        return value.get_int();

      if (value.has_ints())
        {
          for (auto const &el : value.get_ints())
            res *= el;

          return res;
        }

      return 0;
    }

  for (auto const &el : vect)
    {
      if (el.has_int())
        {
          res *= el.get_int();
        }
      else
        {
          return 0;
        }
    }

  return res;
}

std::pair<double, std::size_t>
network_butcher_computer::Computer_flops::compute_macs_flops(
  const network_butcher_computer::Computer_flops::Content_Type<type_info_pointer> &content)
{
  auto       &factory = get_factory();
  auto const &op_id   = content.get_operation_id();

  if (factory.registered(op_id))
    return factory.get(op_id)(content);

  return std::pair<double, std::size_t>{0., 0};
}

network_butcher_computer::Computer_flops::FactoryType &
network_butcher_computer::Computer_flops::get_factory()
{
  if (!factory_initialized)
    {
      generate_maps_flops_func();
      factory_initialized = true;
    }

  return FactoryType::Instance();
}
