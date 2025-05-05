module NormalBench (
    input clk,
    input rst,
    output [16:0] s
);
    wire [15:0] a, b;
    NormalAdder _adr (
        .a(a),
        .b(b),
        .s(s)
    );

    INPGEN _inp (
        .rst(rst),
        .clk(clk),
        .a  (a),
        .b  (b)
    );
endmodule
