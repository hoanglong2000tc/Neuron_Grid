`timescale 1ns / 1ps
module neuron_grid #(
    parameter NUM_AXONS = 256,
    parameter NUM_NEURONS = 256,
    parameter NUM_WEIGHTS = 4,
    parameter LEAK_WIDTH = 9,
    parameter WEIGHT_WIDTH = 9,
    parameter THRESHOLD_WIDTH = 9,
    parameter POTENTIAL_WIDTH = 9,
    parameter NUM_RESET_MODES = 2
)(
    input clk, 
    input rst,
    input tick,
    input [NUM_AXONS-1:0] axon_spikes, 
    input [NUM_AXONS-1:0] synapses, 
    input local_buffers_full,//=0
    output error,
    output scheduler_set, 
    output scheduler_clr,
    output CSRAM_write,//update_potential
    output [$clog2(NUM_NEURONS)-1:0] CSRAM_addr,//neuron_num
    // output [$clog2(NUM_WEIGHTS)-1:0] neuron_instruction, 
    output spike_out,

    input signed [LEAK_WIDTH-1:0] leak,
    input [WEIGHT_WIDTH*NUM_WEIGHTS-1:0] weights,
    input [THRESHOLD_WIDTH-1:0] positive_threshold,
    input [THRESHOLD_WIDTH-1:0] negative_threshold,
    input [POTENTIAL_WIDTH-1:0] reset_potential,
    input [POTENTIAL_WIDTH-1:0] current_potential,
    input [$clog2(NUM_RESET_MODES)-1:0] reset_mode,
    output [POTENTIAL_WIDTH-1:0] write_potential //potential_out
);

wire spike_out_nb, next_neuron;
wire [$clog2(NUM_WEIGHTS)-1:0] neuron_instruction;
TokenController TokenController(
    clk, 
    rst,
    tick,
    axon_spikes, 
    synapses, 
    spike_out_nb,
    local_buffers_full,
    error,
    scheduler_set, 
    scheduler_clr,
    CSRAM_write,
    CSRAM_addr,
    neuron_instruction,
    spike_out,
    neuron_reg_en, 
    next_neuron,
    write_current_potential
);
NeuronBlock NeuronBlock(
    clk,
    leak,
    weights,
    positive_threshold,
    negative_threshold,
    reset_potential,
    current_potential,
    neuron_instruction,
    reset_mode,
    next_neuron,
    neuron_reg_en,
    write_current_potential,
    write_potential,
    spike_out_nb
);


endmodule
