// `timescale 1ns / 1ps
module neuron_grid_datapath(
    input [255:0] axon_spikes,
    input [367:0] neuron_parameter,
    input clk,
    input reset_n,
    input initial_axon_num,
    input inc_axon_num,
    input initial_neuron_num,
    input inc_neuron_num,
    input new_neuron,
    input [1:0] neuron_instruction,
    input process_spike,
    output reg [7:0] axon_num,
    output reg done_neuron,
    output spike_out,
    output reg [7:0] neuron_num,
    output [8:0] potential_out,
    output done_axon
);

assign done_axon = (axon_num == 255);

always @(negedge clk, negedge reset_n) begin
    if(~reset_n) axon_num <= 8'd0;
    else if(initial_axon_num) axon_num <= 8'd0;
    else if(inc_axon_num) axon_num <= axon_num + 1'b1;
    else axon_num <= axon_num;
end


always @(negedge clk, negedge reset_n) begin
    if(~reset_n) neuron_num <= 8'd0;
    else if(initial_neuron_num) neuron_num <= 8'd0;
    else if(inc_neuron_num) neuron_num <= neuron_num + 1'b1;
    else neuron_num <= neuron_num;
end


always @(posedge clk, negedge reset_n) begin
    if(~reset_n) done_neuron <= 0;
    else if(neuron_num == 255) done_neuron <= 1;
    else done_neuron <= 0;
end

wire reg_en;
assign reg_en = (neuron_parameter[112 + axon_num] & axon_spikes[axon_num]);
neuron_block neuron_block(
    .clk(clk),
    .reset_n(reset_n),
    .leak(neuron_parameter[57:49]),
    .weights(neuron_parameter[93:58]),
    .positive_threshold(neuron_parameter[48:40]),
    .negative_threshold(neuron_parameter[39:31]),
    .reset_potential(neuron_parameter[102:94]),
    .current_potential(neuron_parameter[111:103]),
    .neuron_instruction(neuron_instruction),
    .reset_mode(neuron_parameter[30]),
    .new_neuron(new_neuron),
    .process_spike(process_spike),
    .reg_en(reg_en),
    .potential_out(potential_out),
    .spike_out(spike_out)
);

endmodule
