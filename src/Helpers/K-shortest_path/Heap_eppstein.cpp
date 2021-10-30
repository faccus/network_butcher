//
// Created by faccus on 30/10/21.
//

#include "Heap_eppstein.h"

bool
H_g_content::operator<(const H_g_content &rhs) const
{
  return get_value() < rhs.get_value();
}

H_edge
H_g_content::get_value() const
{
  if (content_g && !content_g->children.empty())
    return (*content_g->children.begin()).get_value();
  else if (content_out && !content_out->heap.children.empty())
    return (*content_out->heap.children.begin());
  else
    return {{-1, -1}, std::numeric_limits<type_weight>::max()};
}
std::set<H_edge>
H_g_content::get_edges() const
{
  std::set<H_edge> res;
  if (content_out)
    res = content_out->heap.children;
  else if (content_g)
    {
      for (auto &child : content_g->children)
        {
          auto edges = child.get_edges();
          res.merge(std::move(edges));
        }
    }


  return res;
}