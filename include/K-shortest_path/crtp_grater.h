//
// Created by faccus on 18/05/23.
//

#ifndef NETWORK_BUTCHER_CRTP_GRATER_H
#define NETWORK_BUTCHER_CRTP_GRATER_H

namespace network_butcher::kfinder
{
  /// A simple class that employs the CRTP pattern to implement the > operator
  /// \tparam T The class that inherits from this one
  template <typename T>
  class Crtp_Greater
  {
  public:
    bool
    operator>(Crtp_Greater const &b) const
    {
      return static_cast<T const &>(b) < static_cast<T const &>(*this);
    }
  };
} // namespace network_butcher::kfinder


#endif // NETWORK_BUTCHER_CRTP_GRATER_H
