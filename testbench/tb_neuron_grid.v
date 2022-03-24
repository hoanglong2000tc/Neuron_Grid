`timescale 1ns / 1ps
module tb_neuron_grid;
//Số lượng tick cần test
parameter MAX_NUM_TICK = 16;
//Link các file cần đưa vào module
parameter link_to_axon_spike = "C:/Users/hoang/Desktop/Group03/Verification/Data/axon_spikes.txt";
parameter link_to_CSRAM_data = "C:/Users/hoang/Desktop/Group03/Verification/Data/CSRAM_data.txt";
parameter link_to_neuron_instruction = "C:/Users/hoang/Desktop/Group03/Verification/Data/tc_000.mem";
parameter link_to_output_soft = "C:/Users/hoang/Desktop/Group03/Verification/Data/output_soft.txt";
parameter link_to_many_ticks_axon_spike = "C:/Users/hoang/Desktop/Group03/Verification/Data/many_tick_axon_spikes.txt";
parameter link_to_output_hardware = "C:/Users/hoang/Desktop/Group03/Verification/Data/output_hard.txt";



//Khai Báo đầu vào
reg [367:0] neuron_parameter;
reg [1:0] neuron_instruction;
reg clk;
reg reset_n;
reg tick;
reg [255:0] axon_spikes;
wire error;
wire scheduler_set;
wire scheduler_clr;
wire spike_out;
wire done;
wire [7:0] neuron_num;
wire [7:0] axon_num;
wire update_potential;
wire [8:0] potential_out;

neuron_grid uut(
    .neuron_parameter(neuron_parameter),
    .neuron_instruction(neuron_instruction),
    .clk(clk),
    .reset_n(reset_n),
    .tick(tick),
    .axon_spikes(axon_spikes),
    .error(error),
    .scheduler_set(scheduler_set),
    .scheduler_clr(scheduler_clr),
    .spike_out(spike_out),
    .done(done),
    .neuron_num(neuron_num),
    .axon_num(axon_num),
    .update_potential(update_potential),
    .potential_out(potential_out)
);

//Đọc file mem
reg [367:0] neuron_parameters [0:255];
initial $readmemb(link_to_CSRAM_data, neuron_parameters);

reg [1:0] neuron_instructions [0:255];
initial $readmemb(link_to_neuron_instruction, neuron_instructions);

//Tự động đưa data từ file mem vào module
always @(neuron_num) begin
    neuron_parameter = neuron_parameters[neuron_num];
end
always @(axon_num) begin
    neuron_instruction = neuron_instructions[axon_num];
end

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
    reset_n = 0; @(negedge clk);
    reset_n = 1;
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
always @(done) begin
    if(done) begin
        repeat(3) @(negedge clk);
        tick = 1; @(negedge clk);
        tick = 0;
    end
    else tick = tick;
end
//tự động dừng sau khi đạt đủ số tick/////////////////////////////
always @(num_tick, MAX_NUM_TICK, done) begin
    if((num_tick == MAX_NUM_TICK) & done) begin
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

