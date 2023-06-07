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

  /// Pure virtual generic class used to represent an Heap
  /// \tparam T The stored type
  /// \tparam heap_comparison The comparison function used to construct the max heap (std::less -> Max Heap,
  /// std::greater -> Min Heap)
  template <typename T, typename heap_comparison = std::less<T>>
  class Basic_Heap
  {
  public:
    using container_type = std::vector<T>;

  protected:
    container_type  children;
    heap_comparison comp{};

    virtual void
    heapify() = 0;

  public:
    Basic_Heap() = default;

    virtual void
    push(T const &value) = 0;

    virtual void
    push(T &&value) = 0;

    void
    overwrite_children(container_type const &new_children, bool check_heap = true)
    {
      children = new_children;

      if (check_heap)
        {
          heapify();
        }
    }

    void
    overwrite_children(Basic_Heap<T, heap_comparison> const &other_heap)
    {
      overwrite_children(other_heap.children, false);
    }

    void
    reserve(std::size_t val)
    {
      children.reserve(val);
    }


    [[nodiscard]] bool
    empty() const
    {
      return children.empty();
    }

    [[nodiscard]] std::size_t
    size() const
    {
      return children.size();
    }


    [[nodiscard]] T const &
    get_head() const
    {
      return children.front();
    }

    [[nodiscard]] T const &
    get_elem(std::size_t index) const
    {
      return children[index];
    }

    [[nodiscard]] virtual std::set<std::size_t>
    find_children_indices(std::size_t index) const = 0;

    virtual ~Basic_Heap() = default;
  };


  /// Generic class to represent a (binary) Heap. It's implemented through a std::vector
  /// \tparam T The stored type
  /// \tparam comparison The comparison function used to construct the max heap (std::less -> Max Heap, std::greater ->
  /// Min Heap)
  template <typename T, typename heap_comparison = std::less<T>>
  class Heap : public Basic_Heap<T, heap_comparison>
  {
  public:
    using base           = Basic_Heap<T, heap_comparison>;
    using container_type = base::container_type;

  protected:
    using base::children;
    using base::comp;

    void
    heapify() override
    {
      std::make_heap(children.begin(), children.end(), comp);
    }

  public:
    explicit Heap()
      : base()
    {}

    void
    push(T const &value) override
    {
      children.push_back(value);
      std::push_heap(children.begin(), children.end(), comp);
    }

    void
    push(T &&value) override
    {
      children.push_back(std::move(value));
      std::push_heap(children.begin(), children.end(), comp);
    }

    template <typename... Args>
    void
    emplace(Args &&...args)
    {
      children.emplace_back(std::forward<Args>(args)...);
      std::push_heap(children.begin(), children.end(), comp);
    }

    T
    pop_head()
    {
      std::pop_heap(children.begin(), children.end(), comp);

      T result = std::move(children.back());
      children.pop_back();

      return result;
    }


    [[nodiscard]] std::set<std::size_t>
    find_children_indices(std::size_t index) const override
    {
      std::set<std::size_t> result;
      if (index < children.size())
        {
          std::size_t left  = 2 * index + 1;
          std::size_t right = 2 * index + 2;
          if (left < children.size())
            {
              result.insert(left);
            }
          if (right < children.size())
            {
              result.insert(right);
            }
        }

      return result;
    }

    ~Heap() override = default;
  };


  /// Generic class used to represent an H_out, a data structure similar to an heap: it's made by a node followed by
  /// an heap. The nodes are ordered with the same criteria as in the heap.
  /// \tparam T The stored type
  template <class T, typename heap_comparison = std::less<T>, typename element_less = std::less<T>>
  class H_out : public Basic_Heap<T, heap_comparison>
  {
  public:
    using base           = Basic_Heap<T, heap_comparison>;
    using container_type = base::container_type;

  protected:
    using base::children;
    using base::comp;

    void
    heapify() override
    {
      if (children.size() > 1)
        {
          auto it = std::max_element(children.begin(), children.end(), comp);

          if (it != children.begin())
            {
              std::swap(*it, *children.begin());
            }

          std::make_heap(++children.begin(), children.end(), comp);
        }
    }

  public:

    explicit H_out()
      : base() {};

    void
    push(T &&value) override
    {
      children.push_back(std::move(value));
      if (children.size() > 1)
        {
          if (comp(children.front(), children.back()))
            {
              std::swap(children.front(), children.back());
            }

          std::push_heap(++children.begin(), children.end(), comp);
        }
    }

    void
    push(T const &value) override
    {
      children.push_back(value);
      if (children.size() > 1)
        {
          if (comp(children.front(), children.back()))
            {
              std::swap(children.front(), children.back());
            }

          std::push_heap(++children.begin(), children.end(), comp);
        }
    }


    [[nodiscard]] std::set<Node_Id_Type>
    find_children_indices(std::size_t index) const override
    {
      std::set<Node_Id_Type> result;

      if (index < children.size())
        {
          if (index == 0)
            {
              if (children.size() > 1)
                result.insert(1);
            }
          else
            {
              Node_Id_Type left  = 2 * (index - 1) + 1 + 1;
              Node_Id_Type right = 2 * (index - 1) + 2 + 1;
              if (left < children.size())
                {
                  result.insert(left);
                }
              if (right < children.size())
                {
                  result.insert(right);
                }
            }
        }

      return result;
    }

    bool
    operator<(const H_out &rhs) const
    {
      static element_less local_comp{};

      if (children.empty())
        return true;
      if (rhs.children.empty())
        return false;
      return local_comp(*children.begin(), *rhs.children.begin());
    }

    ~H_out() override = default;
  };
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
