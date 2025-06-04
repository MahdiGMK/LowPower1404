module UMull #(
    parameter  int N = 16,
    localparam int R = 2 * N
) (
    input clk,
    input rst,
    input strt,
    input [N-1:0] a,
    input [N-1:0] b,
    output reg [R-1:0] res,
    output reg done
);
    reg [R-1:0] aa;  // <<
    reg [N-1:0] bb;  // >>
    reg [R-1:0] accum;
    reg [R-1:0] new_accum;
    always @(posedge clk) begin
        if (rst) begin
            done <= 0;
            res  <= 0;
        end else begin
            if (strt) begin
                // $display("req : ", a, " * ", b);
                aa <= (a << 1);
                bb <= (b >> 1);
                accum <= b[0] ? a : 0;
                done <= 0;
            end else begin
                aa <= (aa << 1);
                bb <= (bb >> 1);
                new_accum = bb[0] ? accum + aa : accum;
                accum <= new_accum;
                done  <= (bb <= 1);
                res   <= (bb <= 1) ? new_accum : res;
            end
        end
    end
endmodule


module UKarat #(
    parameter  int N = 16,
    localparam int K = N / 2,
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
    wire [N-1:0] rlow, rhigh;
    wire [N+1:0] rsum;
    wire dlow, dhigh, dsum;

    UMull #(
        .N(K)
    ) _low (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a(a[K-1:0]),
        .b(b[K-1:0]),
        .res(rlow),
        .done(dlow)
    );

    UMull #(
        .N(K)
    ) _high (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a(a[N-1:K]),
        .b(b[N-1:K]),
        .res(rhigh),
        .done(dhigh)
    );

    UMull #(
        .N(K + 1)
    ) _sum (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a({1'b0, a[N-1:K]} + a[K-1:0]),
        .b({1'b0, b[N-1:K]} + b[K-1:0]),
        .res(rsum),
        .done(dsum)
    );
    assign done = &{dlow, dhigh, dsum};
    assign res  = done ? (rhigh << N) + ((rsum - rlow - rhigh) << K) + rlow : res;  // power trick
    // (aa a) * (bb b) = aa bb xx + a b + aa b x + a bb x
    // (a + aa) (b + bb) = a b + aa bb + a bb + bb a
endmodule
