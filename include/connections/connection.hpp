#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include <memory>
#include "synapse_models/synapse.hpp"
#include "network/initializer/initializer.hpp"

namespace snnlib{
    struct AbstractSNNConnectionInitializer;
}
namespace snnlib{
    struct AbstractSNNConnection
    {
        public:
            std::shared_ptr<snnlib::AbstractSNNSynapse> synapses;
            std::vector<double> weights;
            std::vector<bool> connected;
            std::vector<double> P;

            // currently connections are not a dynamical systems.
            // std::vector<double> states;
            // std::vector<double> state_buffer;
            // int n_states = 1;

            AbstractSNNConnection(std::shared_ptr<snnlib::AbstractSNNSynapse> synapses): 
                synapses(synapses){
                weights.assign(synapses->n_presynapse_neurons() * synapses->n_postsynapse_neurons(), 0);
            };

            void initialize_weights(std::shared_ptr<snnlib::AbstractSNNConnectionInitializer> initializer);
            void forward_states_to_buffer(const std::vector<double>& S, double t, double* P, double dt);
            void update_states_from_buffer();
            virtual void initialize(){}
    };
}
#endif