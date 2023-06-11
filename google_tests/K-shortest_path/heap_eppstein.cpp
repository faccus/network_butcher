//
// Created by faccus on 05/06/23.
//

#include "heap_eppstein.h"
#include <gtest/gtest.h>


namespace
{
  using namespace network_butcher::kfinder;


  TEST(HeapEppsteinTest, PriorityQueue)
  {
    std::vector<int>                       vect{4, 7, 2, 3, 6, 8, 1, 9, 0};
    std::list<Heap_Node<int, std::less<>>> nodes;

    nodes.emplace_back(vect.front());
    for (std::size_t i = 1; i < vect.size(); ++i)
      {
        nodes.emplace_back(vect[i]);
        nodes.front().push(&nodes.back());
      }

    std::vector<int> content_heap, content_vect;
    content_heap.reserve(vect.size());
    content_vect.reserve(vect.size());

    std::list<Heap_Node<int, std::less<>> const *> to_visit{&nodes.front()};

    while (!to_visit.empty())
      {
        auto *el = to_visit.front();

        content_heap.push_back(el->get_content());
        content_vect.push_back(el->get_content());

        std::push_heap(content_vect.begin(), content_vect.end(), std::greater<>{});

        for (auto const &child : el->get_children())
          to_visit.push_back(child);

        to_visit.pop_front();
      }

    ASSERT_EQ(content_vect, content_heap);
  }

  TEST(HeapEppsteinTest, H_out)
  {
    std::vector<int> vect{4, 7, 2, 3, 6, 8, 1, 9, 0}, content_vect, content_vect_2 = vect;
    content_vect.reserve(vect.size());

    H_out_Type<int, std::less<>> h_out;
    for (auto const &el : vect)
      {
        h_out.add_elem(el);

        content_vect.push_back(el);

        if (content_vect.size() > 1)
          {
            if (content_vect.back() < content_vect.front())
              std::swap(content_vect.front(), content_vect.back());

            std::push_heap(++content_vect.begin(), content_vect.end(), std::greater<>{});
          }
      }

    std::make_heap(content_vect_2.begin(), content_vect_2.end(), std::greater<>{});
    std::pop_heap(content_vect_2.begin(), content_vect_2.end(), std::greater<>{});
    content_vect_2.insert(content_vect_2.begin(), content_vect_2.back());
    content_vect_2.pop_back();

    std::vector<int> content_heap, content_heap_2;
    content_heap.reserve(vect.size());
    content_heap_2.reserve(vect.size());

    H_out_Type<int, std::less<>> h_out_2(std::move(vect));

    std::list<std::pair<std::size_t, Heap_Node<int, std::less<>> const *>> to_visit{
      std::make_pair(0, h_out.get_head_node())};
    std::vector<Heap_Node<int, std::less<>> const *> ptrs;

    for (auto const &el : h_out.get_internal_children())
      ptrs.push_back(&el);

    content_heap.push_back(to_visit.front().second->get_content());
    to_visit.emplace_back(1, to_visit.front().second->get_children().front());
    to_visit.pop_front();

    while (!to_visit.empty())
      {
        auto       *el    = to_visit.front().second;
        std::size_t index = to_visit.front().first;

        content_heap.push_back(el->get_content());

        for (std::size_t j = 0; j < el->get_children().size(); ++j)
          {
            to_visit.emplace_back(2 * index + j, el->get_children()[j]);
            ASSERT_EQ(ptrs[2 * index + j], el->get_children()[j]);
          }

        to_visit.pop_front();
      }

    ASSERT_EQ(content_vect, content_heap);
    ptrs.clear();

    for (auto const &el : h_out_2.get_internal_children())
      ptrs.push_back(&el);

    to_visit.emplace_back(0, h_out_2.get_head_node());
    content_heap_2.push_back(to_visit.front().second->get_content());
    to_visit.emplace_back(1, to_visit.front().second->get_children().front());
    to_visit.pop_front();

    while (!to_visit.empty())
      {
        auto       *el    = to_visit.front().second;
        std::size_t index = to_visit.front().first;

        content_heap_2.push_back(el->get_content());

        for (std::size_t j = 0; j < el->get_children().size(); ++j)
          {
            to_visit.emplace_back(2 * index + j, el->get_children()[j]);
            ASSERT_EQ(ptrs[2 * index + j], el->get_children()[j]);
          }

        to_visit.pop_front();
      }

    ASSERT_EQ(content_vect_2, content_heap_2);
  }

  TEST(HeapEppsteinTest, H_g)
  {
    using H_out_container_type = std::vector<H_out_Type<int, std::less<>>>;
    std::vector<int> vect{0, 1, 2, 3, 4, -1, -2, 5, -3, 0};

    H_out_container_type h_outs;

    for (auto const &val : vect)
      {
        h_outs.emplace_back();
        h_outs.back().add_elem(val);
      }

    std::vector<H_g_Type<int, std::less<>>> h_gs;
    h_gs.emplace_back(&h_outs.front());

    for (std::size_t i = 1; i < h_outs.size(); ++i)
      {
        h_gs.emplace_back(&h_outs[i], h_gs.back());
      }

    std::vector<int>                                         content_heap;
    std::list<H_g_Type<int, std::less<>>::Node_Type const *> to_visit{h_gs.back().get_head_node()};
    while (!to_visit.empty())
      {
        auto const &node = to_visit.front();

        content_heap.push_back(node->get_content()->get_head_content());

        for (auto const &child : node->get_children())
          to_visit.push_back(child);

        to_visit.pop_front();
      }
  }


} // namespace