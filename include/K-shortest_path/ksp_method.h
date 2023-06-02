//
// Created by faccus on 22/04/23.
//

#ifndef NETWORK_BUTCHER_KSP_METHOD_H
#define NETWORK_BUTCHER_KSP_METHOD_H

namespace network_butcher::parameters
{
  /// Enumerator for the different KSP methods
  enum struct KSP_Method
  {
    Eppstein,
    Lazy_Eppstein
  };
} // namespace network_butcher::parameters

#endif // NETWORK_BUTCHER_KSP_METHOD_H
