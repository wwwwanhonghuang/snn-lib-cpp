#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <yaml-cpp/yaml.h>

#include "neuron_models/neuron.hpp"
#include "neuron_models/lif_neuron.hpp"
#include "neuron_models/possion_neuron.hpp"
#include "neuron_models/initializer.hpp"

#include "network/network.hpp"
#include "network/network_builder.hpp"

#include "synapse_models/synapse.hpp"

#include "simulator/snn_simulator.hpp"

#include "network/initializer/normal_weight_initializer.hpp"

#include "recorder/recorder.hpp"
#include "recorder/connection_recorder.hpp"
#include "recorder/neuron_recorder.hpp"

#include "connections/all_to_all_conntection.hpp"



void build_neurons(snnlib::NetworkBuilder& network_builder){
    std::shared_ptr<snnlib::AbstractSNNNeuron> input_neurons = 
        std::make_shared<snnlib::PossionNeuron>(200, 80);
    std::shared_ptr<snnlib::AbstractSNNNeuron> output_neurons = 
        std::make_shared<snnlib::LIFNeuron>(400);
    network_builder.add_neuron("inputs", input_neurons);
    network_builder.add_neuron_with_initializer("outputs", output_neurons, "rest_potential_initializer");
}

void create_synapse(snnlib::NetworkBuilder& network_builder){
    std::shared_ptr<snnlib::AbstractSNNSynapse> input_output_synapse = 
        std::make_shared<snnlib::CurrentBasedKernalSynapse>(network_builder.get_neuron("inputs"), 
            network_builder.get_neuron("outputs"), "single_exponential", 0.1, 0, 0, 0);
    network_builder.record_synapse("syn_input_output", input_output_synapse);
}

void establish_connections(snnlib::NetworkBuilder& network_builder){
    std::shared_ptr<snnlib::AbstractSNNConnection> connection_input_output = 
        std::make_shared<snnlib::AllToAllConnection>(network_builder.get_synapse("syn_input_output"));
    
    network_builder.add_connection("conn-input-output", connection_input_output);
    network_builder.add_connection_with_initializer("conn-input-output", connection_input_output, "connection_normal_initializer");
}

// Function to run the simulation
void run_simulation(std::shared_ptr<snnlib::SNNNetwork> network, int time_steps, double dt,  std::shared_ptr<snnlib::RecorderFacade> recorder_facade = nullptr){
    snnlib::SNNSimulator simulator;
    simulator.simulate(network, time_steps, dt, recorder_facade);
}


int main(){
    YAML::Node config;
    try {
        config = YAML::LoadFile("config.yaml");
    } catch (const std::exception& e) {
        std::cerr << "Error loading config.yaml: " << e.what() << std::endl;
        return -1;
    }

    // Read configuration
    int time_steps = config["snn-main"]["time-steps"].as<int>();
    double dt = config["snn-main"]["dt"].as<double>();

    std::cout << "time_steps = " << time_steps << std::endl;
    std::cout << "dt = " << dt << std::endl;

    // Building network
    snnlib::NetworkBuilder network_builder;
    build_neurons(network_builder);
    create_synapse(network_builder);
    establish_connections(network_builder);

    std::shared_ptr<snnlib::SNNNetwork> network = network_builder.build_network();

    snnlib::ConnectionRecordCallback weight_recorder = [](const std::string& connection_name, std::shared_ptr<snnlib::AbstractSNNConnection> connection, int t, int dt) -> void{
        if(t == 0)
            snnlib::WeightRecorder::record_connection_weights_to_file(std::string("data/logs/") + connection_name + std::string(".weights"), connection);
    };

    snnlib::NeuroRecordCallback membrane_potential_recorder = [](const std::string& neuron_name, std::shared_ptr<snnlib::AbstractSNNNeuron> neuron, int t, int dt) -> void{
        snnlib::NeuronRecorder::record_membrane_potential_to_file(
            std::string("data/logs/")  + neuron_name
            + std::string("_t_") + std::to_string(t)
            + std::string(".v"), neuron
        );
    };


    // Build Recorder
    std::shared_ptr<snnlib::RecorderFacade> recorder_facade = std::make_shared<snnlib::RecorderFacade>();
    recorder_facade->add_connection_record_item("conn-input-output", weight_recorder);
    recorder_facade->add_neuron_record_item("outputs", membrane_potential_recorder);
    
    // Run simulation
    run_simulation(network, time_steps, dt, recorder_facade);
    config.reset();
}