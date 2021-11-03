//
// Created by faccus on 03/11/21.
//

#include ""

template <class T, typename id_content = io_id_type>
std::ostream &
operator<<(std::ostream                    &op,
           KFinder_Eppstein::<T, id_content>implicit_path_info)
{
  op << "Path lengh: " << length;
  if (!sidetracks.empty())
    {
      op << " (" << sidetracks.front().first << "-"
         << sidetracks.front().second;
      for (auto it = ++sidetracks.cbegin(); it != sidetracks.cend(); ++it)
        op << ", " << it->first << "-" << it->second;
      op << ")";
    }

  return op;
}