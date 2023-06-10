#ifndef NETWORK_BUTCHER_HEAP_EPPSTEIN_H
#define NETWORK_BUTCHER_HEAP_EPPSTEIN_H

#include <limits>
#include <list>
#include <memory>
#include <utility>

#include "crtp_grater.h"
#include "traits.h"

namespace network_butcher::kfinder
{
  /// Simple struct used to store some edge information (unfortunately, we lose the aggregate status)
  /// \tparam Weight_Type The weight type
  template <typename Weight_Type = Time_Type>
  struct Templated_Edge_Info : Crtp_Greater<Templated_Edge_Info<Weight_Type>>
  {
    Edge_Type   edge;
    Weight_Type delta_weight;

    template <typename A, typename B>
    Templated_Edge_Info(A &&in_edge, B &&in_delta_weight)
      : edge(std::forward<A>(in_edge))
      , delta_weight(std::forward<B>(in_delta_weight))
    {}

    bool
    operator<(const Templated_Edge_Info &rhs) const
    {
      return delta_weight < rhs.delta_weight || (delta_weight == rhs.delta_weight && edge < rhs.edge);
    }
  };


  /// Simple node class, used to construct a min-Heap (though pointers...)
  /// \tparam T The node content
  /// \tparam Comparison Content comparison struct
  /// \tparam Max_Children The number of children per node
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
    };

    explicit Heap_Node(T &&initial_content)
      : content(std::move(initial_content))
    {
      children.reserve(Max_Children);
    };

    auto
    operator=(Heap_Node const &) -> Heap_Node & = default;
    Heap_Node(Heap_Node const &)                = default;

    auto
    operator=(Heap_Node &&) -> Heap_Node & = delete;
    Heap_Node(Heap_Node &&)                = delete;


    /// Get the children of the current node
    /// \return The children nodes
    [[nodiscard]] auto
    get_children() const -> std::vector<Heap_Node *> const &
    {
      return children;
    }

    /// Clear the children of the given nodes
    void
    clear_children()
    {
      children.clear();
    }

    /// Add a child to the current node. It will throw if the number of children is already at the maximum
    /// \param child The child to add
    void
    add_child(Heap_Node *child)
    {
      if (children.size() >= Max_Children)
        {
          throw std::runtime_error("Heap_Node::add_child: children.size() >= Max_Children");
        }

      depth[children.size()] = 0;

      children.push_back(child);
    }

    /// Copy a child from the specified node in the current node. It will throw if the number of children is already
    /// at the maximum
    /// \param id The id of the child to copy
    /// \param node The node to copy the child from
    void
    copy_child(std::size_t id, Heap_Node const *const &node)
    {
      if (children.size() >= Max_Children)
        {
          throw std::runtime_error("Heap_Node::copy_child: children.size() >= Max_Children");
        }

      depth[children.size()] = 0;

      children.push_back(node->children[id]);
    }

    /// Get the content of the current node
    /// \return The content
    [[nodiscard]] auto
    get_content() const -> T const &
    {
      return content;
    };

    /// Pushes in the heap the new node and performs the required swaps to keep the heap property
    /// \param new_heap The new node to push
    void
    push(Heap_Node *new_heap)
    {
      internal_push(new_heap);
    }

    /// Gets the number of children of the children of the current node
    /// \return The related array
    [[nodiscard]] auto
    get_depth() const -> std::array<std::size_t, Max_Children> const &
    {
      return depth;
    }

    /// Gets the number of children of the children of the current node. The resulting array is a reference to the
    /// internal array
    /// \return The related array
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
    std::vector<Heap_Node *>              children;

    T content;

    /// Pushes in the heap the new node and performs the required swaps to keep the heap property
    /// \param new_heap The new node to push
    void
    internal_push(Heap_Node *new_heap)
    {
      if (children.size() < Max_Children)
        {
          depth[children.size()] = 0;
          children.push_back(new_heap);

          if (comp(children.back()->content, content))
            {
              std::swap(content, children.back()->content);
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
              std::swap(content, children[index]->content);
            }
        }
    }
  };


  /// Simple class used to represent an H_out, a 2-heap with the extra restriction that the first node may have up to a
  /// single child
  /// \tparam T The type of the content of the nodes
  /// \tparam Comparison The comparison function to use to
  /// compare the content of the nodes
  template <typename T, typename Comparison>
  class H_out_Type
  {
  public:
    using Node_Type                = Heap_Node<T, Comparison, 2>;
    using Internal_Collection_Type = std::list<Node_Type>;

    H_out_Type()
      : internal_children(std::make_unique<Internal_Collection_Type>()){};

    /// Builds an H_out from a collection of elements. The provided collection will be moved. It is equivalent to an
    /// heapify operation
    /// \param input_collection The input collection
    explicit H_out_Type(std::vector<T> &&input_collection)
      : internal_children(std::make_unique<Internal_Collection_Type>())
    {
      build_internal_children(std::move(input_collection));
    };

    /// Builds an H_out from a collection of elements. The provided collection will be moved. It is equivalent to an
    /// heapify operation
    /// \param input_collection The input collection
    explicit H_out_Type(std::list<T> &&input_collection)
      : internal_children(std::make_unique<Internal_Collection_Type>())
    {
      std::vector<T> initial_collection{std::make_move_iterator(input_collection.begin()),
                                        std::make_move_iterator(input_collection.end())};

      build_internal_children(std::move(initial_collection));
    };

    auto
    operator=(H_out_Type const &) -> H_out_Type & = delete;
    H_out_Type(H_out_Type const &)                = delete;

    auto
    operator=(H_out_Type &&) noexcept -> H_out_Type & = default;
    H_out_Type(H_out_Type &&) noexcept                = default;


    /// It adds to the heap a new element based on the provided content
    /// \tparam U The type of the content. Must be convertible to T
    /// \param elem The actual content
    template <typename U>
    void
    add_elem(U &&elem)
    {
      // If there aren't nodes, just add the element
      if (internal_children->empty())
        {
          internal_children->emplace_back(std::forward<U>(elem));
        }
      // If there is a single node...
      else if (internal_children->size() == 1)
        {
          // Check if the new node should be the root of the heap
          if (Node_Type::comp(elem, internal_children->front().get_content()))
            {
              internal_children->emplace_front(std::forward<U>(elem));
            }
          else
            {
              internal_children->emplace_back(std::forward<U>(elem));
            }

          internal_children->front().add_child(&internal_children->back());
        }
      else
        {
          // Check if the new node should be the root of the heap
          if (Node_Type::comp(elem, internal_children->front().get_content()))
            {
              // Substitute the head node with the new one
              auto val = internal_children->front();
              internal_children->pop_front();
              val.clear_children();

              internal_children->emplace_front(std::forward<U>(elem));
              internal_children->front().add_child(&(*(++internal_children->begin())));

              internal_children->emplace_back(val);
            }
          // If not, it goes in the back of the list
          else
            {
              internal_children->emplace_back(std::forward<U>(elem));
            }

          // Add the new node to the heap stored in the second node.
          (++internal_children->begin())->push(&internal_children->back());
        }
    }

    /// It will return the first node in the heap
    /// \return A pointer to the first node in the heap (as a pointer to a constant node)
    [[nodiscard]] auto
    get_head_node() const -> Node_Type const *
    {
      if (internal_children->empty())
        {
          throw std::runtime_error("H_out_test: Empty heap");
        }

      return &internal_children->front();
    }

    /// It will return the content of the first node in the heap
    /// \return A reference to the content of the first node in the heap
    [[nodiscard]] auto
    get_head_content() const -> T const &
    {
      if (internal_children->empty())
        {
          throw std::runtime_error("H_out_test: Empty heap");
        }

      return get_head_node()->get_content();
    }

    /// It will return true if the heap is empty
    /// \return True if the heap is empty, false otherwise
    [[nodiscard]] auto
    empty() const -> bool
    {
      return internal_children->empty();
    }

    auto
    operator<(H_out_Type const &rhs) -> bool
    {
      return Node_Type::comp(get_head_node(), rhs.get_head_node());
    }

    /// It will return the internal collection of nodes (as a constant reference)
    /// \return A constant reference to the internal collection of nodes
    [[nodiscard]] auto
    get_internal_children() const -> std::list<Node_Type> const &
    {
      return *internal_children;
    }

    virtual ~H_out_Type() = default;

  private:
    /// Helper function to build the internal collection of nodes from a collection of elements
    /// \param initial_collection The collection of elements to use to build the internal collection of nodes
    void
    build_internal_children(std::vector<T> &&initial_collection)
    {
      if (!initial_collection.empty())
        {
          auto const reversed_comparison = [](auto const &lhs, auto const &rhs) { return Node_Type::comp(rhs, lhs); };

          std::make_heap(initial_collection.begin(), initial_collection.end(), reversed_comparison); // O(N)
          std::pop_heap(initial_collection.begin(), initial_collection.end(), reversed_comparison);  // O(log(N))

          std::vector<typename std::list<Node_Type>::iterator> iterators;
          iterators.reserve(initial_collection.size());                          // O(N)

          internal_children->emplace_back(std::move(initial_collection.back())); // O(1)
          iterators.push_back(internal_children->begin());                       // O(1)
          initial_collection.pop_back();

          for (std::size_t i = 0; i < initial_collection.size(); ++i) // O(N)
            {
              internal_children->emplace_back(std::move(initial_collection[i]));
              iterators.push_back((++internal_children->rbegin()).base());

              iterators[(i + 1) / 2]->add_child(&internal_children->back());
            }
        }
    }

    std::unique_ptr<Internal_Collection_Type> internal_children;
  };


  /// Simple class used to represent an H_g, an heap of H_out. During its construction, several children of nodes
  /// contained in the heap do not point to nodes that are "physically" stored in the heap itself, but instead point to
  /// nodes that are stored in different H_g heaps. This is done to avoid the need to copy the entire H_g structure
  /// when it's not needed. Moreover, notice that this class does not implement a proper binary heap, since we will give
  /// priority during the insertion to the branches that have the least number of children. Thus, we lose the array
  /// representation of the heap, but, in this way, we both save memory and we can still navigate the heap through
  /// pointers. This class will still be a valid min-heap (but not a proper binary min-heap).
  /// \tparam T The type stored inside each H_out
  /// \tparam Comparison The comparison function used to compare two
  /// elements of type T
  template <typename T, typename Comparison>
  class H_g_Type
  {
  public:
    using Elem_Type = H_out_Type<T, Comparison>;

    /// Simple helper struct used to compare two pointers to elements of type Elem_Type
    struct Pointer_Less
    {
      using Ptr_Type = Elem_Type const *;

      auto
      operator()(Ptr_Type const &lhs, Ptr_Type const &rhs) const -> bool
      {
        return Elem_Type::Node_Type::comp(lhs->get_head_node()->get_content(), rhs->get_head_node()->get_content());
      }
    };

    using Node_Type                = Heap_Node<Elem_Type const *, Pointer_Less, 2>;
    using Internal_Collection_Type = std::list<Node_Type>;

    /// It will construct H_g from the given H_out
    /// \param starting_content
    explicit H_g_Type(Elem_Type const *const &starting_content)
      : internal_children(std::make_unique<Internal_Collection_Type>())
    {
      if (!starting_content->empty())
        internal_children->emplace_back(starting_content);
    };

    /// It will construct H_g from the given H_out and H_g. Pointers to the nodes in the provided H_g will be used to
    /// build the new H_g
    /// \param starting_content The H_out to use to build the new H_g
    /// \param to_copy The H_g to use to build the new H_g
    explicit H_g_Type(Elem_Type const *const &starting_content, H_g_Type const &to_copy)
      : internal_children(std::make_unique<Internal_Collection_Type>())
    {
      if (starting_content->empty() && to_copy.empty())
        {
          return;
        }
      else if (starting_content->empty())
        {
          // Copy the first node and its connections
          internal_children->push_back(to_copy.internal_children->front());
          return;
        }
      else if (to_copy.empty())
        {
          // Add the initial content
          internal_children->emplace_back(starting_content);
          return;
        }
      else
        {
          // We have to go through the whole process
          construction_from_other_h_g(starting_content, to_copy.get_head_node(), nullptr);
        }
    }

    auto
    operator=(H_g_Type const &) -> H_g_Type & = delete;
    H_g_Type(H_g_Type const &)                = delete;

    auto
    operator=(H_g_Type &&) noexcept -> H_g_Type & = default;
    H_g_Type(H_g_Type &&) noexcept                = default;


    /// It will return the head node of the heap
    /// \return The head node of the heap (as a pointer to a constant node)
    [[nodiscard]] auto
    get_head_node() const -> Node_Type const *
    {
      return &internal_children->front();
    }

    /// It will return the size of the heap
    /// \return The size of the heap
    [[nodiscard]] auto
    size() const -> std::size_t
    {
      return internal_children->size();
    }

    /// It will return true if the heap is empty, false otherwise
    /// \return True if the heap is empty, false otherwise
    [[nodiscard]] auto
    empty() const -> bool
    {
      return internal_children->empty();
    }

    ~H_g_Type() = default;

  private:
    std::unique_ptr<Internal_Collection_Type> internal_children;

    /// Helper (recursive) function to construct the new H_g from the given H_out and H_g
    /// \param starting_content The initial H_out
    /// \param other_h_g A node of the H_g used to build the new H_g
    /// \param parent The parent of the current heap
    void
    construction_from_other_h_g(Node_Type::Content_Type const &starting_content,
                                Node_Type const               *other_h_g,
                                Node_Type                     *parent)
    {
      static auto const &comp    = Node_Type::comp;
      bool const         smaller = comp(starting_content, other_h_g->get_content());

      // If the starting content is smaller than the other node, it must be inserted here
      if (smaller)
        {
          internal_children->emplace_back(starting_content);
        }
      // else, insert the content of the other_h_g node
      else
        {
          internal_children->emplace_back(other_h_g->get_content());
        }

      // Get the newly inserted node
      auto &inserted_element = internal_children->back();
      auto &depth            = inserted_element.get_depth_edit();

      // Add the newly added node
      if (parent)
        {
          parent->add_child(&inserted_element);
        }

      // Get the non-inserted content
      auto const &new_content = smaller ? other_h_g->get_content() : starting_content;

      // Check if the other_h_g has zero or one children
      if (other_h_g->get_children().size() < 2)
        {
          // In this case, the other_h_g is not a "complete" heap, but instead it's made by up to two nodes. Then, we
          // just need to insert the non-inserted content as a children of inserted_element and, if other_h_g has a
          // child (that, by construction, will not have any children) we will insert it as a child of inserted_element

          internal_children->emplace_back(new_content);
          inserted_element.add_child(&internal_children->back());
          depth[0] = 0;

          // If the node of the other H_g had an extra child, add it as a child of internal_children
          if (!other_h_g->get_children().empty())
            {
              inserted_element.copy_child(0, other_h_g);
              depth[1] = 0;
            }
        }
      // In this case, the other H_g is still a heap. Thus, we will recursively call the function using as a content
      // the non-inserted content, as the other_h_g we will use the child of the current other_h_g with the smallest
      // depth and as a parent the inserted_element
      else
        {
          depth             = other_h_g->get_depth();
          std::size_t index = std::min(depth.cbegin(), depth.cend()) - depth.cbegin();

          inserted_element.copy_child(1 - index, other_h_g);
          ++depth[index];

          construction_from_other_h_g(new_content, other_h_g->get_children()[index], &inserted_element);
        }
    }
  };

} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
