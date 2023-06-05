//
// Created by faccus on 30/10/21.
//

#ifndef NETWORK_BUTCHER_HEAP_EPPSTEIN_H
#define NETWORK_BUTCHER_HEAP_EPPSTEIN_H

#include <limits>
#include <list>
#include <memory>
#include <utility>

#include "basic_traits.h"
#include "kfinder_base_traits.h"

#include "crtp_grater.h"


namespace network_butcher::kfinder
{
  /// Simple struct used to store some edge information
  /// \tparam Weight_Type The weight type
  template <typename Weight_Type = Time_Type>
  struct Templated_Edge_Info : kfinder::Crtp_Greater<Templated_Edge_Info<Weight_Type>>
  {
    Edge_Type   edge;
    Weight_Type delta_weight;

    Templated_Edge_Info(Edge_Type const &in_edge, Weight_Type const &in_delta_weight)
      : edge(in_edge)
      , delta_weight(in_delta_weight)
    {}

    bool
    operator<(const Templated_Edge_Info &rhs) const
    {
      return delta_weight < rhs.delta_weight || (delta_weight == rhs.delta_weight && edge < rhs.edge);
    }
  };


  template <typename T, typename Comparison, std::size_t Max_Children = 2>
  class Heap_Node
  {
  public:
    using Content_Type = T;

    static inline Comparison comp{};

    Heap_Node() = default;

    explicit Heap_Node(T const &initial_content)
      : content(initial_content)
    {
      children.reserve(Max_Children);
      edit_children.reserve(Max_Children);
    };

    explicit Heap_Node(T &&initial_content)
      : content(std::move(initial_content))
    {
      children.reserve(Max_Children);
      edit_children.reserve(Max_Children);
    };

    auto
    operator=(Heap_Node const &) -> Heap_Node & = default;
    Heap_Node(Heap_Node const &)                = default;

    auto
    operator=(Heap_Node &&) -> Heap_Node & = delete;
    Heap_Node(Heap_Node &&)                = delete;


    [[nodiscard]] auto
    get_children() const -> std::vector<Heap_Node const *> const &
    {
      return children;
    }

    void
    clear_children()
    {
      children.clear();
      edit_children.clear();
    }

    void
    add_child(Heap_Node *child)
    {
      if (children.size() >= Max_Children)
        {
          throw std::runtime_error("Heap_Node::copy_child: children.size() >= Max_Children");
        }

      depth[edit_children.size()] = 0;

      edit_children.push_back(child);
      children.push_back(child);
    }

    void
    copy_children(Heap_Node const *const &node)
    {
      edit_children = node->edit_children;
      children      = node->children;
      depth         = node->depth;
    }

    void
    copy_child(std::size_t id, Heap_Node const *const &node)
    {
      if (children.size() >= Max_Children)
        {
          throw std::runtime_error("Heap_Node::copy_child: children.size() >= Max_Children");
        }

      depth[edit_children.size()] = 0;

      edit_children.push_back(node->edit_children[id]);
      children.push_back(node->children[id]);
    }


    [[nodiscard]] auto
    get_content() const -> T const &
    {
      return content;
    };

    [[nodiscard]] auto
    get_content_edit() -> T &
    {
      return content;
    };


    void
    push(Heap_Node *new_heap)
    {
      internal_push(new_heap);
    }


    [[nodiscard]] auto
    get_depth() const -> std::array<std::size_t, Max_Children> const &
    {
      return depth;
    }

    [[nodiscard]] auto
    get_depth_edit() -> std::array<std::size_t, Max_Children> &
    {
      return depth;
    }


    [[nodiscard]] auto
    operator<(const Heap_Node &rhs) const -> bool
    {
      return comp(content, rhs.content);
    }


    virtual ~Heap_Node() = default;

  protected:
    std::array<std::size_t, Max_Children> depth;
    std::vector<Heap_Node const *>        children;
    std::vector<Heap_Node *>              edit_children;

    T content;

    void
    safe_swap(T &other_content)
    {
      if constexpr (std::is_pointer_v<T> || std::is_reference_v<T>)
        {
          auto tmp      = other_content;
          other_content = content;
          content       = tmp;
        }
      else
        {
          std::swap(other_content, content);
        }
    }

    void
    internal_push(Heap_Node *new_heap)
    {
      if (edit_children.size() < Max_Children)
        {
          depth[edit_children.size()] = 0;
          edit_children.push_back(new_heap);

          if (comp(edit_children.back()->content, content))
            {
              safe_swap(edit_children.back()->content);
            }

          children.push_back(new_heap);
        }
      else
        {
          std::size_t index = std::max_element(depth.cbegin(),
                                               depth.cend(),
                                               [](std::size_t const &lhs, std::size_t const &rhs) {
                                                 return (lhs % Max_Children) < (rhs % Max_Children);
                                               }) -
                              depth.cbegin();

          if (index == 0 && depth.front() % Max_Children == 0)
            {
              index = std::min_element(depth.cbegin(), depth.cend()) - depth.cbegin();
            }

          edit_children[index]->internal_push(new_heap);
          ++depth[index];

          if (comp(edit_children[index]->content, content))
            {
              safe_swap(edit_children[index]->content);
            }
        }
    }
  };


  template <typename T, typename Comparison>
  class H_out_test
  {
  public:
    using Node_Type = Heap_Node<T, Comparison, 2>;

    H_out_test() = default;

    explicit H_out_test(std::vector<T> &&initial_collection)
      : internal_children()
    {
      if (!initial_collection.empty())
        {
          std::make_heap(initial_collection.begin(), initial_collection.end(), Node_Type::comp); // O(N)

          std::vector<typename std::list<Node_Type>::iterator> iterators;
          iterators.reserve(initial_collection.size());                           // O(N)

          initial_collection.emplace_back(std::move(initial_collection.front())); // O(1)

          iterators.push_back(internal_children.begin());                         // O(1)

          for (std::size_t i = 1; i < initial_collection.size(); ++i)             // O(N)
            {
              internal_children.emplace_back(std::move(initial_collection[i]));
              iterators.push_back((++internal_children.rbegin()).base());

              iterators[(i - 1) / 2]->add_child(&internal_children.back());
            }
        }
    };

    auto
    operator=(H_out_test const &) -> H_out_test & = default;
    H_out_test(H_out_test const &)                = default;


    auto
    operator=(H_out_test &&) -> H_out_test & = delete;
    H_out_test(H_out_test &&)                = delete;

    void
    add_elem(T const &elem)
    {
      if (internal_children.empty())
        {
          internal_children.emplace_back(elem);
        }
      else if (internal_children.size() == 1)
        {
          if (Heap_Node<T, Comparison, 2>::comp(elem, internal_children.front().get_content()))
            {
              internal_children.emplace_front(elem);
            }
          else
            {
              internal_children.emplace_back(elem);
            }

          internal_children.front().add_child(&internal_children.back());
        }
      else
        {
          if (Heap_Node<T, Comparison, 2>::comp(elem, internal_children.front().get_content()))
            {
              auto val = internal_children.front();
              internal_children.pop_front();
              val.clear_children();

              internal_children.emplace_front(elem);
              internal_children.front().add_child(&(*(++internal_children.begin())));

              internal_children.emplace_back(val);
            }
          else
            {
              internal_children.emplace_back(elem);
            }

          (++internal_children.begin())->push(&internal_children.back());
        }
    }

    [[nodiscard]] auto
    get_head_node() const -> Node_Type const *
    {
      if (internal_children.empty())
        {
          throw std::runtime_error("H_out_test: Empty heap");
        }

      return &internal_children.front();
    }

    [[nodiscard]] auto
    get_head_content() const -> T const &
    {
      if (internal_children.empty())
        {
          throw std::runtime_error("H_out_test: Empty heap");
        }

      return get_head_node()->get_content();
    }

    auto
    operator<(H_out_test const &rhs) -> bool
    {
      return Heap_Node<T, Comparison, 2>::comp(get_head_node(), rhs.get_head_node());
    }

    virtual ~H_out_test() = default;

  private:
    std::list<Node_Type> internal_children;
  };


  template <typename T, typename Comparison>
  class H_g_test
  {
  public:
    using Elem_Type = H_out_test<T, Comparison>;


    struct Pointer_Less
    {
      using Ptr_Type = Elem_Type const *;

      auto
      operator()(Ptr_Type const &lhs, Ptr_Type const &rhs) const -> bool
      {
        return Elem_Type::Node_Type::comp(lhs->get_head_node()->get_content(), rhs->get_head_node()->get_content());
      }
    };


    using Node_Type = Heap_Node<Elem_Type const *, Pointer_Less, 2>;

    explicit H_g_test(H_out_test<T, Comparison> const *const &starting_content)
      : internal_children()
    {
      internal_children.emplace_back(starting_content);
    };

    H_g_test(H_out_test<T, Comparison> const *const &starting_content, H_g_test const &to_copy)
      : internal_children()
    {
      if (to_copy.internal_children.empty())
        {
          internal_children.emplace_back(starting_content);
          return;
        }

      construction_from_other_h_g(starting_content, to_copy.get_head_node(), nullptr);
    }

    [[nodiscard]] auto
    get_head_node() const -> Node_Type const *
    {
      return &internal_children.front();
    }

    [[nodiscard]] auto
    size() const -> std::size_t
    {
      return internal_children.size();
    }

    [[nodiscard]] auto
    empty() const -> bool
    {
      return internal_children.empty();
    }


  private:
    std::list<Node_Type> internal_children;

    auto
    recursion_copy(Node_Type const *const &to_copy) -> Node_Type *
    {
      // Insert myself
      internal_children.emplace_back(to_copy->get_content());

      // Get my position
      Node_Type *elem = &internal_children.back();

      // Add the copy of my children position
      for (auto const &child : to_copy->get_children())
        {
          elem->add_child(recursion_copy(child));
        }

      // Return my position
      return elem;
    }

    void
    construction_from_other_h_g(Node_Type::Content_Type const &starting_content,
                                H_g_test::Node_Type const     *other_h_g,
                                H_g_test::Node_Type           *parent)
    {
      static auto const &comp = Node_Type::comp;

      if (comp(starting_content, other_h_g->get_content()))
        {
          Node_Type *father = recursion_copy(other_h_g);

          if (parent)
            {
              parent->add_child(father);
            }

          internal_children.emplace_back(starting_content);
          father->push(&internal_children.back());
        }
      else
        {
          internal_children.emplace_back(other_h_g->get_content());
          auto &inserted_element = internal_children.back();

          if (parent)
            {
              parent->add_child(&inserted_element);
            }

          if (other_h_g->get_children().size() < 2)
            {
              if (other_h_g->get_children().empty())
                {
                  inserted_element.get_depth_edit()[0] = 0;
                }
              else
                {
                  inserted_element.copy_child(0, other_h_g);
                  inserted_element.get_depth_edit() = {0, 0};
                }

              internal_children.emplace_back(starting_content);
              inserted_element.add_child(&internal_children.back());
            }
          else
            {
              auto &depth = inserted_element.get_depth_edit();
              depth       = other_h_g->get_depth();

              std::size_t index = std::min(depth.cbegin(), depth.cend()) - depth.cbegin();

              inserted_element.copy_child(1 - index, other_h_g);
              ++depth[index];

              construction_from_other_h_g(starting_content, other_h_g->get_children()[index], &inserted_element);
            }
        }
    }
  };

} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
