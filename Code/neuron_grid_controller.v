// `timescale 1ns / 1ps
module neuron_grid_controller(
    input tick,
    input done_neuron,
    input done_axon,
    input clk,
    input reset_n,
    input spike_in,
    output reg process_spike,
    output reg scheduler_clr,
    output reg scheduler_set,
    output reg inc_neuron_num,
    output reg initial_neuron_num,
    output reg initial_axon_num,
    output reg inc_axon_num,
    output reg new_neuron,
    output reg update_potential,
    output reg done,
    output reg error,
    output reg spike_out
);


localparam IDLE     = 0;
localparam GET_DATA = 1;
localparam INITIAL  = 2;
localparam SPIKE_IN = 3;
localparam UPDATE   = 4;
localparam END      = 5;

reg [2:0] current_state, next_state;

always @(current_state, tick, done_neuron, spike_in, done_axon) begin
    initial_axon_num = 0;
    initial_neuron_num = 0;
    scheduler_set = 0;
    scheduler_clr = 0;
    new_neuron = 0;
    process_spike = 0;
    inc_axon_num = 0;
    update_potential = 0;
    done = 0;
    inc_neuron_num = 0;
    spike_out = 0;
    //////
    case(current_state)
    IDLE: begin
        next_state = tick ? GET_DATA : IDLE;
    end
    GET_DATA: begin
        initial_neuron_num = 1;
        scheduler_set = 1;
        next_state = INITIAL;
        new_neuron = 1;
    end
    INITIAL: begin
        initial_axon_num = 1;
        // new_neuron = 1;
        process_spike = 1;
        next_state = SPIKE_IN;
    end
    SPIKE_IN: begin
        process_spike = 1;
        if(done_axon) next_state = UPDATE;
        else begin
            inc_axon_num = 1;
            next_state = SPIKE_IN;
        end
    end
    UPDATE: begin
        update_potential = 1;
        spike_out = spike_in;
        next_state = END;
    end
    END: begin
        if(done_neuron) begin
            scheduler_clr = 1;
            done = 1;
            next_state = IDLE;
        end
        else begin
            new_neuron = 1;
            inc_neuron_num = 1;
            next_state = INITIAL;
        end
    end
    default: next_state = IDLE;

    endcase
end

always @(posedge clk, negedge reset_n) begin
    if(~reset_n) begin
        current_state <= IDLE;
        error <= 0;
    end
    else begin
        current_state <= next_state;
        if(~error && current_state != IDLE && tick) error <= ~error;
        else error <= error;
    end
end



endmodule