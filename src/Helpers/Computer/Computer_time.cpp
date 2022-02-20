//
// Created by faccus on 07/11/21.
//

#include "Computer_time.h"

Computer_time::Computer_time()
{
  setup();
}

void
Computer_time::setup() const
{
  static bool initialized = false;
  if (!initialized)
    {
      auto &factory = get_factory();

      factory.add("relu", [](node_type const &node, bool forward) {
        std::size_t res  = -1;
        auto const &outs = node.content.get_output();

        if (outs.size() == 1)
          {
            auto const &out_shape = outs.begin()->second->get_shape();

            std::size_t const C_out = out_shape[1];
            res                     = forward ? 3 * C_out : 4 * C_out;
          }

        return res;
      });

      factory.add("loss", [](node_type const &node, bool forward) {
        std::size_t res  = 0;
        auto const &outs = node.content.get_output();

        if (outs.size() == 1)
          {
            auto const &out_shape = outs.begin()->second->get_shape();

            std::size_t const C_out = out_shape[1];

            res = forward ? 4 * C_out - 1 : C_out + 1;
          }
        return res;
      });

      factory.add("batchnormalization",
                  [](node_type const &node, bool forward) {
                    std::size_t res = 0;

                    auto const &ins  = node.content.get_input();
                    auto const &outs = node.content.get_output();

                    if (ins.size() == 1 && outs.size() == 1)
                      {
                        auto const &out_shape =
                          outs.begin()->second->get_shape();
                        auto const &in_shape = ins.begin()->second->get_shape();

                        std::size_t const C_in  = in_shape[1];
                        std::size_t const C_out = out_shape[1];
                        res =
                          forward ? 5 * C_out + C_in - 2 : 8 * C_out + C_in - 1;
                      }

                    return res;
                  });

      factory.add("conv", [](node_type const &node, bool forward) {
        std::size_t res     = 0;
        auto const &content = node.content;

        auto const &ins  = content.get_input();
        auto const &outs = content.get_output();
        auto const  kernel_iterator =
          content.get_attributes().find("kernel_shape");

        if (ins.size() == 1 && outs.size() == 1 &&
            kernel_iterator != content.get_attributes().cend())
          {
            auto const &out_shape    = outs.begin()->second->get_shape();
            auto const &in_shape     = ins.begin()->second->get_shape();
            auto const &kernel_shape = kernel_iterator->second;

            std::size_t const H_f_times_W_f =
              kernel_iterator->second[0] * kernel_iterator->second[1];

            std::size_t const C_in  = in_shape[1];
            std::size_t const C_out = out_shape[1];

            res = forward ? H_f_times_W_f * C_in * C_out :
                            (2 * H_f_times_W_f * C_in + 1) * C_out;
          }

        return res;
      });

      factory.add("maxpool", [](node_type const &node, bool forward) {
        std::size_t res     = 0;
        auto const &content = node.content;

        auto const &outs = content.get_output();
        auto const  kernel_iterator =
          content.get_attributes().find("kernel_shape");

        if (outs.size() == 1 &&
            kernel_iterator != content.get_attributes().cend())
          {
            auto const &out_shape    = outs.begin()->second->get_shape();
            auto const &kernel_shape = kernel_iterator->second;

            std::size_t const C_out         = out_shape[1];
            std::size_t const H_f_times_W_f = kernel_shape[0] * kernel_shape[1];
            res = forward ? H_f_times_W_f * C_out : (H_f_times_W_f + 1) * C_out;
          }

        return res;
      });

      factory.add("add", [](node_type const &node, bool forward) {
        std::size_t res = 0;

        auto const &outs = node.content.get_output();

        if (outs.size() == 1)
          {
            auto const &out_shape = outs.begin()->second->get_shape();
            res = out_shape[0] * out_shape[1] * out_shape[2] * out_shape[3];
          }

        return res;
      });

      initialized = true;
    }
}

time_type
Computer_time::compute_operation_time(const node_type               &node,
                                      const Hardware_specifications &hw)
{
  time_type   res          = .0;
  auto const &operation_id = node.content.get_operation_id();

  auto       &factory     = get_factory();
  auto const &time_coeffs = hw.get_regression_coefficients(operation_id);

  if (factory.registered(operation_id) && time_coeffs.first >= 0 &&
      time_coeffs.second >= 0)
    {
      auto       computer   = factory.get(operation_id);
      auto const operations = computer(node, true);
      res = operations * time_coeffs.second + time_coeffs.first;
    }

  return res;
}