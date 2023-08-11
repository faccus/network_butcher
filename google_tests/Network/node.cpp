#include <gtest/gtest.h>
#include <memory>

#include <network_butcher/Network/node.h>
#include <network_butcher/Network/node_traits.h>

// Simple tests to verify basic constructors

namespace
{
  using namespace network_butcher;
  using namespace network_butcher::types;

  // Verify that it can be built successfully
  TEST(NodeTests, Conctructor)
  {
    Node node;

    CNode<int> node_2(1);
  }

  // Verify that the node can be moved
  TEST(NodeTests, MoveConstructible)
  {
    ASSERT_TRUE(std::is_move_constructible_v<CNode<std::unique_ptr<int>>>);
    ASSERT_TRUE(std::is_move_constructible_v<Onnx_Converted_Node_Type>);
  }

  // Verify that the node can be moved
  TEST(NodeTests, MoveAssignable)
  {
    ASSERT_TRUE(std::is_move_assignable_v<CNode<std::unique_ptr<int>>>);
    ASSERT_TRUE(std::is_move_assignable_v<Onnx_Converted_Node_Type>);
  }

  // Verify that the node can be copied
  TEST(NodeTests, Copy)
  {
    ASSERT_FALSE(std::is_copy_assignable_v<CNode<std::unique_ptr<int>>>);
    ASSERT_FALSE(std::is_copy_constructible_v<CNode<std::unique_ptr<int>>>);

    ASSERT_TRUE(std::is_copy_constructible_v<Onnx_Converted_Node_Type>);
    ASSERT_TRUE(std::is_copy_assignable_v<Onnx_Converted_Node_Type>);
  }

} // namespace