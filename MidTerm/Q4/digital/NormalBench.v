module NormalBench (
    input clk,
    input rst
);
    wire [15:0] a, b;
    wire [16:0] s;
    NormalAdder _mcc (
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
