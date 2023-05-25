//
// Created by faccus on 18/05/23.
//

#ifndef NETWORK_BUTCHER_CRTP_GRATER_H
#define NETWORK_BUTCHER_CRTP_GRATER_H

namespace network_butcher::kfinder
{
  template <typename T>
  class crtp_greater
  {
  public:
    bool
    operator>(crtp_greater const &b) const
    {
      return static_cast<T const &>(b) < static_cast<T const &>(*this);
    }
  };
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_CRTP_GRATER_H
