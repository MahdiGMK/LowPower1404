module ShaddMul #(
    localparam int N = 128,
    localparam int R = 2 * N
) (
    input clk,
    input rst,
    input strt,
    input [N-1:0] a,
    input [N-1:0] b,
    output [R-1:0] res,
    output done
);
    wire [R-1:0] res_mod;
    UMull #(
        .N(N)
    ) _mul (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a(a[N-1] ? -a : a),
        .b(b[N-1] ? -b : b),
        .res(res_mod),
        .done(done)
    );
    reg output_inv;
    assign res = output_inv ? -res_mod : res_mod;
    always @(posedge clk) begin
        if (rst) begin
            output_inv <= 0;
        end else begin
            if (strt) begin
                output_inv <= a[N-1] ^ b[N-1];
            end
        end
    end
endmodule
