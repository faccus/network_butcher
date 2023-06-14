//
// Created by faccus on 08/06/23.
//

#ifndef NETWORK_BUTCHER_NETWORK_BUTCHER_H
#define NETWORK_BUTCHER_NETWORK_BUTCHER_H

#include <network_butcher/Network/graph_traits.h>
#include <network_butcher/K-shortest_path/heap_traits.h>
#include <network_butcher/Traits/traits.h>

#include <network_butcher/Butcher/butcher.h>
#include <network_butcher/io_manager.h>
#include <network_butcher/Computer/computer_memory.h>
#include <network_butcher/K-shortest_path/kfinder_factory.h>
#include <network_butcher/utilities.h>

#include <network_butcher/APSC/GetPot>
#include <network_butcher/APSC/chrono.h>

#include <network_butcher/general_manager.h>



/// \namespace network_butcher network_butcher is the basic namespace of the library. ALL other namespace and classes
/// are contained here
namespace network_butcher
{}

/// \namespace kfinder kfinder is the namespace that contains ALL the methods and classes related to the K shortest path
/// algorithm
namespace network_butcher::kfinder
{}

/// \namespace io io is the namespace that contains ALL the methods and classes that interact with the IO. In
/// particular, IO_Manager contains all the function that an user should use, while the remaining namespace contains
/// all helper functions available.
namespace network_butcher::io
{}

/// \namespace computer computer is the namespace containing the functions required to make computations with specific types.
namespace network_butcher::computer
{}

/// \namespace types types is the namespace that contains the main data structures of the library
namespace network_butcher::types {}

/// \namespace Computer_memory Computer_memory is the namespace containing the functions required to compute the memory
/// usage of various types
namespace network_butcher::computer::Computer_memory {}

/// \namespace Utilities Utilities contains some utility functions called throughout all the library.
namespace network_butcher::Utilities {}

/// \namespace parameters parameters is the namespace that will contain all the parameter related types
namespace network_butcher::parameters {}

/// \namespace constraints constraints is the namespace containing all the constraints that can be applied during the block
/// graph construction. It also contains Graph_Constraint, the base (pure virtual) class used to represent a constraint.
namespace network_butcher::constraints {}


#endif // NETWORK_BUTCHER_NETWORK_BUTCHER_H
