module MCCBench (
    input clk,
    input rst,
    output [16:0] s
);
    wire [15:0] a, b;
    MCCAdder _mcc (
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


module MCCtest;
    reg clk, rst;
    wire [16:0] s;
    MCCBench _bnc (
        .clk(clk),
        .rst(rst),
        .s  (s)
    );
    initial begin
        clk = 0;
        forever #1 clk = ~clk;
    end
    initial begin
        rst = 1;
        #4 rst = 0;
        $monitor("%b", s[7:0]);
        #40;
        $finish();
    end
endmodule
