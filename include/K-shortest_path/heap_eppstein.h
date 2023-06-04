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
  protected:
    static inline Comparison              comp{};
    std::array<std::size_t, Max_Children> depth;
    std::vector<Heap_Node *>              children;

    T content;

    void
    internal_push(Heap_Node *new_heap)
    {
      if (children.size() < Max_Children)
        {
          depth[children.size()] = 0;
          children.push_back(new_heap);
        }


    }

  public:
    Heap_Node(T const &initial_content)
      : content(initial_content)
    {
      children.reserve(Max_Children);
    };

    virtual auto
    get_children() const -> std::vector<Heap_Node *> const &
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
    get_head() const -> T const &
    {
      return content;
    };

    void
    push(Heap_Node *new_heap)
    {
      internal_push(new_heap);
    }


    virtual ~Heap_Node() = default;
  };


  template <typename T, typename Comparison>
  class H_out
  {
  private:
    std::list<Heap_Node<T, Comparison>> internal_children;

  public:
    H_out() = default;
  };

} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
