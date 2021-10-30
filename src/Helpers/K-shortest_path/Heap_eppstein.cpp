//
// Created by faccus on 30/10/21.
//

#include "Heap_eppstein.h"

bool
H_g_content::operator<(const H_g_content &rhs)
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


const std::shared_ptr<H_out_helper> &
H_g_content::getContentOut() const
{
  return content_out;
}
const std::shared_ptr<H_g> &
H_g_content::getContentG() const
{
  return content_g;
}
bool
H_g_content::isOut() const
{
  if (content_out)
    return true;
  else
    return false;
}
bool
H_g_content::isG() const
{
  if (content_g)
    return true;
  else
    return false;
}
