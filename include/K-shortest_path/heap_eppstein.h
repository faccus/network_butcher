//
// Created by faccus on 30/10/21.
//

#ifndef NETWORK_BUTCHER_HEAP_EPPSTEIN_H
#define NETWORK_BUTCHER_HEAP_EPPSTEIN_H

#include <limits>
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
  struct Templated_Edge_Info : Crtp_Greater<Templated_Edge_Info<Weight_Type>>
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

    explicit Heap_Node(T const &initial_content)
      : content(initial_content)
    {
      children.reserve(2);
    };

    virtual auto
    get_children() const -> std::vector<Heap_Node const *> const &
    {
      return children;
    }

    template <typename CollectionType>
    void
    set_children(CollectionType const &collection)
    {
      children.insert(collection.begin(), collection.end());
    }

    void
    add_child(Heap_Node *child)
    {
      children.insert(child);
    }

    virtual auto
    get_content() const -> T const &
    {
      return content;
    };

    void
    push(Heap_Node const *new_heap)
    {
      internal_push(new_heap);
    }

    auto
    get_depth() const -> std::array<std::size_t, Max_Children> const &
    {
      return depth;
    }


    virtual ~Heap_Node() = default;

  protected:
    std::array<std::size_t, Max_Children> depth;
    std::vector<Heap_Node const *>        children;

    T content;

    void
    internal_push(Heap_Node const *const &new_heap)
    {
      if (children.size() < Max_Children)
        {
          depth[children.size()] = 0;
          children.push_back(new_heap);

          if (comp(children.back()->content, content))
            {
              std::swap(children.back()->content, content);
            }
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

          children[index]->internal_push(new_heap);
          ++depth[index];

          if (comp(children[index]->content, content))
            {
              std::swap(children[index]->content, content);
            }
        }
    }
  };


  template <typename T, typename Comparison>
  class H_out
  {
  public:
    using Node_Type = Heap_Node<T, Comparison, 2>;

    H_out()              = default;
    H_out(H_out const &) = delete;
    auto
    operator=(H_out const &) -> H_out & = delete;

    void
    add_elem(T const &elem)
    {
      if (internal_children.empty())
        {
          internal_children.emplace_back(elem);
        }
      else if (internal_children.size() == 1)
        {
          internal_children.emplace_back(elem);
          if (Heap_Node<T, Comparison, 2>::comp(internal_children.back().get_content(),
                                                internal_children.front().get_content()))
            {
              std::swap(internal_children.front(), internal_children.back());
            }

          internal_children.front().add_child(&internal_children.back());
        }
      else
        {
          internal_children.emplace_back(elem);
          internal_children.back().push(&internal_children.back());
        }
    }

    auto
    get_head_node() const -> Node_Type const *
    {
      return &internal_children.front();
    }

    auto
    get_head_content() -> T const &
    {
      if (internal_children.empty())
        {
          throw std::runtime_error("H_out: Empty heap");
        }

      return get_head_node()->get_content();
    }

    auto
    operator<(H_out const &rhs) -> bool
    {
      return Heap_Node<T, Comparison, 2>::comp(get_head_node(), rhs.get_head_node());
    }

    virtual ~H_out() = default;

  private:
    std::list<Node_Type> internal_children;
  };


  template <typename T, typename Comparison>
  struct Pointer_Less
  {
    auto
    operator()(H_out<T, Comparison> *const &lhs, H_out<T, Comparison> *const &rhs)
    {
      return H_out<T, Comparison>::Node_Type::comp(lhs->get_head_node()->get_content(),
                                                   rhs->get_head_node()->get_content());
    }
  };

  template <typename T, typename Comparison>
  class H_g
  {
  public:
    using Node_Type = Heap_Node<H_out<T, Comparison> *, Pointer_Less<T, Comparison>, 2>;
    using Elem_Type = H_out<T, Comparison>;

    explicit H_g(H_out<T, Comparison> *const &starting_content)
      : internal_children(Node_Type(starting_content)){};

    H_g(H_out<T, Comparison> *const &starting_content, H_g const &to_copy)
      : internal_children()
    {
      if (to_copy.internal_children.empty())
        {
          internal_children.emplace_back(starting_content);
          return;
        }

      construct_from_copy(starting_content, to_copy.get_head_node(), nullptr);
    }

    auto
    operator=(H_g const &) -> H_g & = delete;
    H_g(H_g const &)                = delete;

    auto
    get_head_node() const -> Node_Type const *
    {
      return &internal_children.front();
    }

    virtual ~H_g() = default;

  private:
    std::list<Node_Type> internal_children;

    Node_Type *
    recursion_copy(Node_Type const *const &to_copy)
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
    construct_from_copy(H_out<T, Comparison> *const &starting_content,
                        Node_Type const *const      &to_copy,
                        Node_Type                   *parent)
    {
      // If the content is smaller than the head node of to_copy heap, we have to copy ALL to_copy and perform the usual
      // insert
      if (Elem_Type::Node_Type::comp(starting_content->get_head_content(), to_copy->get_content()->get_head_content()))
        {
          Node_Type *father = recursion_copy(to_copy);

          if (parent)
            parent->add_child(father);

          internal_children.emplace_back(starting_content);

          father->push(&internal_children.back());
        }
      else
        {
          internal_children.emplace_back(to_copy->get_content());

          if (parent)
            parent->add_child(&internal_children.back());

          // If you have less than two children, we can safely add the new child (no rearranging required)
          if (to_copy->get_children().size() < 2)
            {
              internal_children.back().set_children(to_copy->get_children());
              auto &back = internal_children.back();
              internal_children.emplace_back(starting_content);
              back.add_child(&internal_children.back());

              return;
            }


          // Get the index of the children that should contain the new content
          auto       &depth = internal_children.back().get_depth();
          std::size_t index =
            std::max_element(depth.cbegin(),
                             depth.cend(),
                             [](std::size_t const &lhs, std::size_t const &rhs) { return (lhs % 2) < (rhs % 2); }) -
            depth.cbegin();

          if (index == 0 && depth.front() % 2 == 0)
            {
              index = std::min_element(depth.cbegin(), depth.cend()) - depth.cbegin();
            }

          // Add the non-selected child between its children and...
          internal_children.back().add_child(to_copy->get_children()[index]);

          // ... try to add the new child in the collection
          construct_from_copy(starting_content, to_copy->get_children()[1 - index], &internal_children.back());
        }
    }
  };


} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
