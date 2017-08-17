#ifndef LULESH_HPP_
#define LULESH_HPP_

#include "ocxxr.hpp"

extern "C" {
#include "lulesh.h"
}

// Define some more stuff to use in our C++ code only

struct Domain {
    // Remains constant
    ocxxr::RelPtr<double> node_mass;
    ocxxr::RelPtr<double> element_mass;
    ocxxr::RelPtr<double> element_volume;
    // Initial per iteration values
    double initial_delta_time;
    // Initial node values
    ocxxr::RelPtr<vector> initial_force;
    ocxxr::RelPtr<vector> initial_velocity;
    ocxxr::RelPtr<vertex> initial_position;
    // Initial element
    ocxxr::RelPtr<double> initial_volume;
    ocxxr::RelPtr<double> initial_viscosity;
    ocxxr::RelPtr<double> initial_pressure;
    ocxxr::RelPtr<double> initial_energy;
    ocxxr::RelPtr<double> initial_speed_sound;
};

struct Mesh {
    int number_nodes;
    int number_elements;
    ocxxr::NestedRelPtr<int, 2> nodes_node_neighbors; 					// 6 * number_nodes
    ocxxr::NestedRelPtr<int, 2> nodes_element_neighbors;				// 8 * number_nodes
    ocxxr::NestedRelPtr<int, 2> elements_node_neighbors;			// 8 * number_elements
    ocxxr::NestedRelPtr<int, 2> elements_element_neighbors;	// 6 * number_elements
};

struct LuleshCtxIndirect {
    Domain domain;
    Mesh mesh;
};

struct LuleshCtx : public luleshCtx {
    LuleshCtxIndirect ii;
    // XXX - Include enough extra memory for my pointers
    ocxxr::RelPtr<int> _int_ptrs[NODES+NODES+ELEMENTS+ELEMENTS];
};

#endif // LULESH_HPP_
