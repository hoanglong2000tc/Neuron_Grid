// `timescale 1ns / 1ps
module neuron_grid(
    input [367:0] neuron_parameter,
    input [1:0] neuron_instruction,
    input clk,
    input reset_n,
    input tick,
    input [255:0] axon_spikes,
    output error,
    output scheduler_set,
    output scheduler_clr,
    output spike_out,
    output done,
    output [7:0] neuron_num,
    output [7:0] axon_num,
    output update_potential,
    output [8:0] potential_out
);

wire new_neuron, process_spike, inc_neuron_num, initial_neuron_num, inc_axon_num, initial_axon_num, spike_out_datapath, done_axon;

neuron_grid_controller controller(
    .tick(tick),
    .done_neuron(done_neuron),
    .done_axon(done_axon),
    .clk(clk),
    .reset_n(reset_n),
    .spike_in(spike_out_datapath),
    .process_spike(process_spike),
    .scheduler_clr(scheduler_clr),
    .scheduler_set(scheduler_set),
    .inc_neuron_num(inc_neuron_num),
    .initial_neuron_num(initial_neuron_num),
    .initial_axon_num(initial_axon_num),
    .inc_axon_num(inc_axon_num),
    .new_neuron(new_neuron),
    .update_potential(update_potential),
    .done(done),
    .error(error),
    .spike_out(spike_out)    
);

neuron_grid_datapath datapath(
    .axon_spikes(axon_spikes),
    .neuron_parameter(neuron_parameter),
    .clk(clk),
    .reset_n(reset_n),
    .initial_axon_num(initial_axon_num),
    .inc_axon_num(inc_axon_num),
    .initial_neuron_num(initial_neuron_num),
    .inc_neuron_num(inc_neuron_num),
    .new_neuron(new_neuron),
    .neuron_instruction(neuron_instruction),
    .process_spike(process_spike),
    .axon_num(axon_num),
    .done_neuron(done_neuron),
    .spike_out(spike_out_datapath),
    .neuron_num(neuron_num),
    .potential_out(potential_out),
    .done_axon(done_axon)
);

endmodule

