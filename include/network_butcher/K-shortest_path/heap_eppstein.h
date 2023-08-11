#ifndef NETWORK_BUTCHER_HEAP_EPPSTEIN_H
#define NETWORK_BUTCHER_HEAP_EPPSTEIN_H

#include <limits>
#include <list>
#include <memory>
#include <utility>
#include <numeric>

#include <network_butcher/K-shortest_path/crtp_greater.h>
#include <network_butcher/Traits/traits.h>

namespace network_butcher::kfinder
{
  /// Simple struct used to store some edge information (unfortunately, we lose the aggregate status to be able to use
  /// emplace in STL containers)
  /// \tparam Weight_Type The weight type
  template <typename Weight_Type = Time_Type>
  struct Templated_Edge_Info : Crtp_Greater<Templated_Edge_Info<Weight_Type>>
  {
    Edge_Type   edge;
    Weight_Type delta_weight;

    /// Perfect forwarding constructor
    /// \param in_edge The edge
    /// \param in_delta_weight The sidetrack weight
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


  /// Simple node class, used to construct a min-Heap (though pointers...). Thus, copies are allowed, but not moves.
  /// Depth can be used to keep track of the number of nodes the current node in each branch.
  /// \tparam T The node content
  /// \tparam Comparison Content comparison struct
  /// \tparam Max_Children The number of children per node
  template <typename T, typename Comparison, std::size_t Max_Children = 2>
  class Heap_Node
  {
  public:
    /// The content type
    using Content_Type = T;

    /// The comparison structure instance. It's a static member, so it's shared among all the nodes
    static inline Comparison comp{};

    /// Default constructor
    Heap_Node() = default;

    /// It constructs a node with the given content
    /// \param initial_content The content
    explicit Heap_Node(T const &initial_content)
      : content(initial_content)
      , depth()
    {
      children.reserve(Max_Children);

      for (auto &el : depth)
        el = 0;
    };

    /// It constructs a node with the given content
    /// \param initial_content The content (as an rvalue)
    explicit Heap_Node(T &&initial_content)
      : content(std::move(initial_content))
      , depth()
    {
      children.reserve(Max_Children);

      for (auto &el : depth)
        el = 0;
    };

    /// Copy constructor
    /// \param other The node to copy
    Heap_Node(Heap_Node const &other) = default;

    /// Copy assignment
    /// \param other The node to copy
    /// \return The current node
    auto operator=(Heap_Node const &other) -> Heap_Node & = default;

    /// Move constructor
    /// \param other The node to move
    Heap_Node(Heap_Node &&other) noexcept = default;

    /// Move assignment
    /// \param other The node to move
    /// \return The current node
    auto operator=(Heap_Node &&other) noexcept -> Heap_Node & = default;


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

      for (auto &el : depth)
        el = 0;
    }

    /// Add a child to the current node. It will throw if the number of children is already at the maximum
    /// \param child The child to add
    /// \param depth_update If true, the depth will be updated
    void
    add_child(Heap_Node *child, bool depth_update = true)
    {
      if (children.size() >= Max_Children)
        {
          throw std::runtime_error("Heap_Node::add_child: children.size() >= Max_Children");
        }

      if (depth_update)
        {
          depth[children.size()] = 1;
        }

      children.push_back(child);
    }

    /// Copy a child from the specified node in the current node. It will throw if the number of children is already
    /// at the maximum
    /// \param id The id of the child to copy
    /// \param node The node to copy the child from
    /// \param depth_update If true, the depth will be updated
    void
    copy_child(std::size_t id, Heap_Node const *const &node, bool depth_update = true)
    {
      if (children.size() >= Max_Children)
        {
          throw std::runtime_error("Heap_Node::copy_child: children.size() >= Max_Children");
        }

      if (depth_update)
        {
          depth[children.size()] = 1;
        }

      children.push_back(node->children[id]);
    }

    /// Get the content of the current node
    /// \return The content
    [[nodiscard]] auto
    get_content() const -> T const &
    {
      return content;
    };

    /// Get the content of the current node
    /// \return The content
    [[nodiscard]] auto
    get_content() -> T const &
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

    /// Gets the number of nodes below the current one in each branch
    /// \return The related array
    [[nodiscard]] auto
    get_depth() const -> std::array<std::size_t, Max_Children> const &
    {
      return depth;
    }

    /// Gets the number of nodes below the current one in each branch. The resulting array is a reference to the
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

  private:
    std::array<std::size_t, Max_Children> depth;
    std::vector<Heap_Node *>              children;

    T content;

    /// Pushes in the heap the new node and performs the required swaps to keep the heap property
    /// \param new_heap The new node to push
    void
    internal_push(Heap_Node *new_heap)
    {
      // If we are not at capacity
      if (children.size() < Max_Children)
        {
          // Add the node
          depth[children.size()] = 1;
          children.push_back(new_heap);

          // Swap it with a child if needed
          if (comp(children.back()->content, content))
            {
              std::swap(content, children.back()->content);
            }
        }
      else
        {
          // Find the child with the maximum depth % Max_Children
          std::size_t index = std::max_element(depth.cbegin(),
                                               depth.cend(),
                                               [](std::size_t const &lhs, std::size_t const &rhs) {
                                                 return ((lhs - 1) % Max_Children) < ((rhs - 1) % Max_Children);
                                               }) -
                              depth.cbegin();

          // If index == 0, then there are two possibilities: either the new insertion should take place in the first
          // branch, or the first branch is full and we should insert in another branch (with minimum depth).
          if (index == 0 && (depth.front() - 1) % Max_Children == 0)
            {
              // If we are here, then we just have to consider the element with the smallest depth
              index = std::min_element(depth.cbegin(), depth.cend()) - depth.cbegin();
            }

          // Push the new node in the selected branch
          children[index]->internal_push(new_heap);

          // Increase the depth of the selected branch
          ++depth[index];

          // Swap it with the node in the selected branch
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
    /// The type of the nodes of the heap
    using Node_Type = Heap_Node<T, Comparison, 2>;

    /// The type of the internal collection of nodes
    using Internal_Collection_Type = std::list<Node_Type>;

    /// Default constructor. It prepares an empty heap
    H_out_Type()
      : internal_children(std::make_unique<Internal_Collection_Type>()){};

    /// Builds an H_out from a collection of elements. The provided collection will be moved. It is equivalent to an
    /// heapify operation.
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

    /// Copy assignment operator is deleted since the node collection is stored in a unique_ptr
    auto
    operator=(H_out_Type const &) -> H_out_Type & = delete;

    /// Copy constructor is deleted since the node collection is stored in a unique_ptr
    H_out_Type(H_out_Type const &) = delete;

    /// Move assignment operator
    auto
    operator=(H_out_Type &&) noexcept -> H_out_Type & = default;

    /// Move constructor
    H_out_Type(H_out_Type &&) noexcept = default;


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

          // Link the root node with the last node
          internal_children->front().add_child(&internal_children->back());
        }
      else
        {
          // Check if the new node should be the root of the heap
          if (Node_Type::comp(elem, internal_children->front().get_content()))
            {
              // Substitute the head node with the new one and insert the old head node to the back of the list
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

          auto &heap_head = *(++internal_children->begin());

          // Add the last node to the heap (whose head is in the second node)
          heap_head.push(&internal_children->back());

          internal_children->begin()->get_depth_edit()[0] = internal_children->size() - 1;
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

    /// It will return the content of the first node in the heap
    /// \return A reference to the content of the first node in the heap
    [[nodiscard]] auto
    get_head_content() -> T const &
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

    /// It will return the internal collection of nodes (as a constant reference). The collection is NOT heap ordered
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

          // Use make_heap to heapify the collection
          std::make_heap(initial_collection.begin(), initial_collection.end(), reversed_comparison); // O(N)

          // Use pop_heap to extract the top element of the heap
          std::pop_heap(initial_collection.begin(), initial_collection.end(), reversed_comparison); // O(log(N))

          // Collection of iterators to the nodes in the internal collection
          std::vector<typename std::list<Node_Type>::iterator> iterators;
          iterators.reserve(initial_collection.size()); // O(N)

          // Add the the last element of initial_collection to the internal collection. It should be the minimum element
          internal_children->emplace_back(initial_collection.back()); // O(1)
          iterators.push_back(internal_children->begin());                       // O(1)

          // Remove the last element of initial_collection
          initial_collection.pop_back();

          // Add the remaining nodes to the heap. Links will also be generated based on the position of the nodes in the
          // initial_collection heap
          for (std::size_t i = 0; i < initial_collection.size(); ++i) // O(N)
            {
              internal_children->emplace_back(initial_collection[i]);
              iterators.push_back((++internal_children->rbegin()).base());

              iterators[(i + 1) / 2]->add_child(&internal_children->back());
            }

          for (std::size_t i = internal_children->size() - 1; ; --i)
            {
              auto       &iterator = iterators[i];
              auto       &depth    = iterator->get_depth_edit();
              auto const &children = iterator->get_children();

              for (std::size_t j = 0; j < children.size(); ++j)
                {
                  depth[j] = std::reduce(children[j]->get_depth().cbegin(), children[j]->get_depth().cend(), 1);
                }

              if(i == 0)
                {
                  break;
                }
            }
        }
    }

    /// Internal collection of nodes
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

    /// Node type used to represent the nodes in the heap
    using Node_Type = Heap_Node<Elem_Type const *, Pointer_Less, 2>;

    /// Type of the internal collection of nodes
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

    /// Deleted copy assignment operator because the node collection is stored in a unique_ptr
    auto
    operator=(H_g_Type const &) -> H_g_Type & = delete;

    /// Deleted copy constructor because the node collection is stored in a unique_ptr
    H_g_Type(H_g_Type const &) = delete;

    /// Default move assignment operator
    auto
    operator=(H_g_Type &&) noexcept -> H_g_Type & = default;

    /// Default move constructor
    H_g_Type(H_g_Type &&) noexcept = default;


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
    /// Internal collection of nodes
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
          parent->add_child(&inserted_element, false);
        }

      // Get the non-inserted content
      auto const &new_content = smaller ? other_h_g->get_content() : starting_content;

      // Check if the other_h_g has zero or one children
      if (other_h_g->get_children().size() < 2)
        {
          // In this case, the other_h_g is not a "complete" heap, but instead it's made by up to two nodes. Then, we
          // just need to insert the non-inserted content as a children of inserted_element and, if other_h_g has a
          // child (that, by construction, will not have any child), we will insert it as a child of inserted_element

          internal_children->emplace_back(new_content);
          inserted_element.add_child(&internal_children->back());

          // If the node of the other H_g had an extra child, add it as a child of internal_children
          if (!other_h_g->get_children().empty())
            {
              inserted_element.copy_child(0, other_h_g);
            }
        }
      // In this case, the other H_g is still a heap. Thus, we will recursively call the function using as a content
      // the non-inserted content, as the other_h_g we will use the child of the current other_h_g with the smallest
      // depth and as a parent the inserted_element
      else
        {
          depth             = other_h_g->get_depth();
          std::size_t index = std::min_element(depth.cbegin(), depth.cend()) - depth.cbegin();

          if (index == 0)
            {
              construction_from_other_h_g(new_content, other_h_g->get_children()[index], &inserted_element);
              inserted_element.copy_child(1 - index, other_h_g, false);
            }
          else
            {
              inserted_element.copy_child(1 - index, other_h_g, false);
              construction_from_other_h_g(new_content, other_h_g->get_children()[index], &inserted_element);
            }

          ++depth[index];
        }
    }
  };

} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_HEAP_EPPSTEIN_H
