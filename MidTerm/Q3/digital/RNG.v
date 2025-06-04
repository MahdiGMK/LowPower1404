module RNG (
    input clk,
    input rst,
    input [7:0] seed,
    output reg [7:0] rnd
);
    always @(posedge clk) begin
        if (rst) rnd <= seed;
        else begin
            repeat (3) begin
                {rnd[7:1], rnd[0]} = {rnd[6:0], rnd[3] ^ rnd[4] ^ rnd[5] ^ rnd[7]};
            end
        end
    end
endmodule

module INPGEN #(
    parameter int N = 16
) (
    input clk,
    input rst,
    output [N-1:0] a,
    output [N-1:0] b
);
    genvar i;
    generate
        for (i = 0; i < N / 8; i += 1) begin : gen_rng
            wire [7:0] aseed = 8'b01010101 ^ (i * 71269);
            wire [7:0] bseed = 8'b01011010 ^ (i * 190873);
            RNG _a (
                .clk (clk),
                .rst (rst),
                .seed(aseed),
                .rnd (a[i*8+:8])
            );
            RNG _b (
                .clk (clk),
                .rst (rst),
                .seed(bseed),
                .rnd (b[i*8+:8])
            );
        end
    endgenerate
endmodule
