module MCCAdder (
    input  [15:0] a,
    input  [15:0] b,
    output [16:0] s
);
    wire [3:-1] C;
    assign C[-1] = 0;
    genvar i;
    generate
        for (i = 0; i < 4; i++) begin : gen_mcc
            MCC4 _mcc (
                .a(a[i*4+:4]),
                .b(b[i*4+:4]),
                .cin(C[i-1]),
                .cout(C[i]),
                .s(s[i*4+:4])
            );
        end
    endgenerate
    assign s[16] = C[3];
endmodule

module MCC4 (
    input [3:0] a,
    input [3:0] b,
    input cin,
    output [3:0] s,
    output cout
);
    wire [3:-1] C;
    assign C[-1] = ~cin;
    genvar i;
    generate
        for (i = 0; i < 4; i += 1) begin : gen_mcc
            MCC1 _1 (
                .a(a[i]),
                .b(b[i]),
                ._cin(C[i-1]),
                ._cout(C[i]),
                .s(s[i])
            );
        end
    endgenerate
    assign cout = ~C[3];
endmodule

module MCC1 (
    input  a,
    input  b,
    input  _cin,
    output s,
    output _cout
);
    wire P;
    wire G;
    xor (P, a, b);
    and (G, a, b);
    wire _wk1;
    wire _st0;
    // assign (pull1, weak0) _wk1 = 1;
    assign _wk1 = 1;
    assign _st0 = 0;
    pmos _p0 (_cout, _wk1, G);
    wire _inter;
    nmos _n0 (_cout, _inter, G);
    nmos _n1 (_inter, _st0, ~P);
    nmos _n2 (_cout, _cin, P);
    xnor (s, P, _cin);
endmodule

// module mcc4test;
//     reg [3:0] a, b;
//     reg cin;
//     wire [3:0] s;
//     wire cout;
//     MCC4 _mcc (
//         .a(a),
//         .b(b),
//         .cin(cin),
//         .cout(cout),
//         .s(s)
//     );
//     int i, j, k;
//     initial begin
//         // $monitor("%b %b %b => %b %b", a, b, cin, cout, s);
//         for (i = 0; i < 16; i += 1)
//         for (j = 0; j < 16; j += 1)
//         for (k = 0; k < 2; k += 1) begin
//             a   = i;
//             b   = j;
//             cin = k;
//             #1;
//             assert (a + b + cin == {cout, s});
//         end
//     end
// endmodule

// module mcctest;
//     reg [15:0] a, b;
//     wire [16:0] s;
//     MCCAdder _mcc (
//         .a(a),
//         .b(b),
//         .s(s)
//     );

//     int i, j, k;
//     initial begin
//         // $monitor("%b %b %b => %b %b", a, b, cin, cout, s);
//         for (i = 0; i < 500; i += 1)
//         for (j = 0; j < 500; j += 1) begin
//             a = i;
//             b = j;
//             #1;
//             assert (a + b == s);
//         end

//         for (i = 65000; i < 65536; i += 1)
//         for (j = 65000; j < 65536; j += 1) begin
//             a = i;
//             b = j;
//             #1;
//             assert (a + b == s);
//         end
//     end
// endmodule
