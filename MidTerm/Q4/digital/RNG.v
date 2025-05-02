module RNG (
    input clk,
    input rst,
    input [7:0] seed,
    output reg [7:0] rnd
);
    always @(posedge clk) begin
        if (rst) rnd <= seed;
        else begin
            rnd[7:1] <= rnd[6:0];
            rnd[0]   <= rnd[3] ^ rnd[4] ^ rnd[5] ^ rnd[7];
        end
    end
endmodule

module INPGEN (
    input clk,
    input rst,
    output [15:0] a,
    output [15:0] b
);
    RNG _0 (
        .clk (clk),
        .rst (rst),
        .seed(8'b01010101),
        .rnd (a[7:0])
    );

    RNG _1 (
        .clk (clk),
        .rst (rst),
        .seed(8'b01011010),
        .rnd (b[7:0])
    );

    RNG _2 (
        .clk (clk),
        .rst (rst),
        .seed(8'b01101010),
        .rnd (a[15:8])
    );

    RNG _3 (
        .clk (clk),
        .rst (rst),
        .seed(8'b10101011),
        .rnd (b[15:8])
    );

endmodule
