module Bench #(
    localparam int N = 16
) (
    input clk,
    input rst,
    output [N-1:0] a,
    output [N-1:0] b,
    output strt
);
    reg [7:0] timer;
    INPGEN _inp (
        .clk(clk | (timer > 0)),  // clk gating
        .rst(rst),
        .a  (a),
        .b  (b)
    );
    assign strt = timer == 0;
    always @(posedge clk) begin
        if (rst || timer == 0) timer <= 30;
        else timer <= timer - 1;
    end
endmodule
