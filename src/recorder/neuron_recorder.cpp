#include "recorder/neuron_recorder.hpp"

void snnlib::NeuronRecorder::_record_membrane_potential(std::ostream& output_stream, std::shared_ptr<snnlib::AbstractSNNNeuron> neuron){
    int neuron_count = neuron->n_neurons;
    for(int i = 0; i < neuron_count; i++){
        output_stream << neuron->x[neuron->n_states * i + neuron->OFFSET_STATE_V] << std::endl;
    }
}


