`timescale 1ns / 1ps
module tb_neuron_grid;
// //Mode giả lập
// parameter simulation_mode = 0; //0: giả lập với 1 file; 1: giả lập với nhiều file

// //Số lượng file test trong trường hợp mode 1
// parameter number_of_file = 10;

//Số lượng tick cần test
parameter MAX_NUM_TICK = 1;
//Link các file cần đưa vào module
parameter link_to_axon_spike = "C:/Users/hoang/Desktop/Group03/Verification/Data/axon_spikes.txt";
parameter link_to_CSRAM_data = "C:/Users/hoang/Desktop/Group03/Verification/Data/CSRAM_data.txt";
parameter link_to_neuron_instruction = "C:/Users/hoang/Desktop/Group03/Verification/Data/vmm/tc_000.mem";
parameter link_to_output_soft = "C:/Users/hoang/Desktop/Group03/Verification/Data/output_soft.txt";
parameter link_to_many_ticks_axon_spike = "C:/Users/hoang/Desktop/Group03/Verification/Data/many_tick_axon_spikes.txt";
parameter link_to_output_hardware = "C:/Users/hoang/Desktop/Group03/Verification/Data/output_hard.txt";

// parameter link_of_all_axon_spike = "C:/Users/ROG STRIX/Desktop/SNN/RANC/many_mem_file/link_of_all_axon_spike.txt";
// parameter link_of_all_CSRAM_data = "C:/Users/ROG STRIX/Desktop/SNN/RANC/many_mem_file/link_of_all_CSRAM_data.txt";
// parameter link_of_all_many_ticks_axon_spike = "C:/Users/ROG STRIX/Desktop/SNN/RANC/many_mem_file/link_of_all_many_ticks_axon_spike.txt";
// parameter link_of_all_output_soft = "C:/Users/ROG STRIX/Desktop/SNN/RANC/many_mem_file/link_of_all_output.txt";


// string link_axon_spike [0:number_of_file-1];
// string link_CSRAM_data [0:number_of_file-1];
// string link_output_soft [0:number_of_file-1];
// string link_many_ticks_axon_spike [0:number_of_file-1];




//Khai Báo đầu vào
parameter NUM_AXONS = 256;
parameter NUM_NEURONS = 256;
parameter NUM_WEIGHTS = 4;
parameter LEAK_WIDTH = 9;
parameter WEIGHT_WIDTH = 9;
parameter THRESHOLD_WIDTH = 9;
parameter POTENTIAL_WIDTH = 9;
parameter NUM_RESET_MODES = 2;
reg clk; 
reg rst;
reg tick;
reg [NUM_AXONS-1:0] axon_spikes; 
reg [NUM_AXONS-1:0] synapses; 
reg local_buffers_full;//=0
wire error;
wire scheduler_set; 
wire scheduler_clr;
wire update_potential;//CSRAM_write
wire [$clog2(NUM_NEURONS)-1:0] neuron_num;//CSRAM_addr
wire spike_out;

reg signed [LEAK_WIDTH-1:0] leak;
reg [WEIGHT_WIDTH*NUM_WEIGHTS-1:0] weights;
reg [THRESHOLD_WIDTH-1:0] positive_threshold;
reg [THRESHOLD_WIDTH-1:0] negative_threshold;
reg [POTENTIAL_WIDTH-1:0] reset_potential;
reg [POTENTIAL_WIDTH-1:0] current_potential;
reg [$clog2(NUM_RESET_MODES)-1:0] reset_mode;
wire [POTENTIAL_WIDTH-1:0] potential_out; //write_potential



neuron_grid neuron_grid(
    clk, 
    rst,
    tick,
    axon_spikes, 
    synapses, 
    local_buffers_full,//=0
    error,
    scheduler_set, 
    scheduler_clr,
    update_potential,
    neuron_num,
    spike_out,

    leak,
    weights,
    positive_threshold,
    negative_threshold,
    reset_potential,
    current_potential,
    reset_mode,
    potential_out
);




initial local_buffers_full = 0;



//Đọc file mem
reg [367:0] neuron_parameters [0:255];
initial $readmemb(link_to_CSRAM_data, neuron_parameters);

// reg [1:0] neuron_instructions [0:255];
// initial $readmemb(link_to_neuron_instruction, neuron_instructions);

//Tự động đưa data từ file mem vào module
always @(neuron_num) begin
    {synapses, current_potential, reset_potential, weights, leak, positive_threshold, negative_threshold, reset_mode} <= neuron_parameters[neuron_num][367:30];

end
// always @(axon_num) begin
//     neuron_instruction = neuron_instructions[axon_num];
// end

//Tự động update lại data từ file mem sau mỗi lần tính toán xong cho 1 neuron trong 1 tick
always @(posedge clk) begin
    if(update_potential) neuron_parameters[neuron_num][111:103] <= potential_out;
    else neuron_parameters[neuron_num][111:103] <= neuron_parameters[neuron_num][111:103];
end



//Đọc axon_spikes từ file
reg [255:0] mem [0:MAX_NUM_TICK - 1];
initial begin
    if(MAX_NUM_TICK == 1) $readmemb(link_to_axon_spike, mem);
    else $readmemb(link_to_many_ticks_axon_spike, mem);
end
integer row = 0;
always @(scheduler_set) begin
    if(scheduler_set) begin
        axon_spikes = mem[row];
        row = row + 1;
        // $display("%b", axon_spikes);
    end
end
//clk
initial begin
    clk = 0;
    forever #5 clk = ~clk;
end
//reset_n
initial begin
    rst = 1; @(negedge clk);
    rst = 0;
end
//initial tick
initial begin
    tick = 0; repeat(3) @(negedge clk);
    tick = 1; @(negedge clk);
    tick = 0;
end


////////AUTO//////////////////////////////////////////////////////////
integer num_tick = 0;


reg finish;
initial finish = 0;

always @(tick) begin
    num_tick = tick ? num_tick + 1 : num_tick;
end
//tự động gán tick = 1 sau khi tính xong 1 tick/////////////////////////
always @(scheduler_clr) begin
    if(scheduler_clr) begin
        repeat(3) @(negedge clk);
        tick = 1; @(negedge clk);
        tick = 0;
    end
    else tick = tick;
end
//tự động dừng sau khi đạt đủ số tick/////////////////////////////
always @(num_tick, MAX_NUM_TICK, scheduler_clr) begin
    if((num_tick == MAX_NUM_TICK) & scheduler_clr) begin
        repeat(2) @(negedge clk);
        finish = 1; 
        #2; $stop;
    end
end


////////////////write output///////////////////////////////////////
reg [255:0] output_hard [0:MAX_NUM_TICK - 1];
integer index;
initial begin
    for(index = 0; index < MAX_NUM_TICK; index = index + 1) output_hard[index] = 256'd0;
end

always @(update_potential, neuron_num, spike_out) begin
    if(update_potential && spike_out) output_hard[num_tick-1][neuron_num] = 1'b1;
end

always @(finish) begin
    if(finish) $writememb(link_to_output_hardware, output_hard);
end


///////compare with output from software////////////////////////////
reg [255:0] output_soft [0:MAX_NUM_TICK - 1];
reg wrong;
initial wrong = 0;
initial $readmemb(link_to_output_soft, output_soft);
integer i, j;
always @(finish) begin
    if(finish) begin
        for(i = 0; i < MAX_NUM_TICK; i = i + 1) begin
            for(j = 0; j < 256; j = j + 1) begin
                if(output_hard[i][j] != output_soft[i][j]) begin
                    $display("Error at neuron %d, tick %d", j, i);
                    wrong = 1;
                end
            end
        end
    end
end
always @(finish) begin
    if(finish) begin
        #1; if(~wrong) $display("Test pass without error");
    end
    
end






endmodule

