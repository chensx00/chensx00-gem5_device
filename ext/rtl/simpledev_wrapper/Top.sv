module Top(
    input clk,
    input rst,
    input ena,
    input[7:0] inA,
    input[7:0] inB,
    output[7:0] out,
    output ok
);

SimDev counter(
    .clk(clk),
    .rst(rst),
    .ena(ena),
    .inA(inA),
    .inB(inB),
    .out(out),
    .ok(ok)
);

endmodule
