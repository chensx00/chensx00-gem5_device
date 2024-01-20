
module SimDev
(
    input wire clk,
    input wire rst,
    input wire ena,
    input reg[7:0] inA,
    input reg[7:0] inB,
    output reg[7:0] out,
    output reg ok/*,
    output reg rdy,
    output reg[2:0]cnt*/
);


reg rdy;
reg[2:0] cnt;

always_ff @( posedge clk ) begin : _Add
    if(rst == 1'b1) begin
        out <= 8'h0;
    end else if(ena == 1'b1) begin
        out <= inA + inB;
    end else begin
        out <= out;
    end
end


always_ff @( posedge clk ) begin : _rdy
    if(rst == 1'b1) begin
        rdy <= 1'b0;
    end else if(ena == 1'b1) begin
        rdy <= 1'b1;
    end /*else if(ok == 1'b1)begin
        rdy <= 1'b0;
    end */else begin
        rdy <= 1'b0;
    end
end


always_ff @( posedge clk ) begin : _cnt
    if(rst == 1'b1) begin
        cnt <= 3'b111;
    end else if(rdy) begin
        cnt <= cnt - 1;
    end else if(cnt == 3'b111)begin
        cnt <= cnt;
    end else begin
        cnt <= cnt -1;
    end
end

always_ff @( posedge clk ) begin : _ok
    if(rst == 1'b1) begin
        ok <= 1'b0;
    end else if(cnt == 3'h0) begin
        ok <= 1'b1;
    end else begin
        ok <= 1'b0;
    end
end


endmodule
