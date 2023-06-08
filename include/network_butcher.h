//
// Created by faccus on 08/06/23.
//

#ifndef NETWORK_BUTCHER_NETWORK_BUTCHER_H
#define NETWORK_BUTCHER_NETWORK_BUTCHER_H

#include "graph_traits.h"
#include "heap_traits.h"
#include "traits.h"

#include "butcher.h"
#include "io_manager.h"
#include "computer_memory.h"
#include "kfinder_factory.h"
#include "utilities.h"



/// \namespace network_butcher is the basic namespace of the library. ALL other namespace and classes are contained here
namespace network_butcher
{}

/// \namespace kfinder is the namespace that contains ALL the methods and classes related to the K shortest path
/// algorithm
namespace network_butcher::kfinder
{}

/// \namespace types is the namespace that contains ALL the methods and classes that interact with the IO. In
/// particular, IO_Manager contains all the function that an user should use, while the remaining namespace contains
/// all helper functions available.
namespace network_butcher::io
{}

/// \namespace computer is the namespace containing the functions required to make computations with specific types.
namespace network_butcher::computer
{}

/// \namespace Computer_memory is the namespace containing the functions required to compute the memory usage of
/// various types
namespace network_butcher::computer::Computer_memory {}

/// \namespace Utilities contains some utility functions called thought all the code.
namespace network_butcher::Utilities {}




#endif // NETWORK_BUTCHER_NETWORK_BUTCHER_H
