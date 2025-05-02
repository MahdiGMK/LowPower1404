module NormalAdder (
    input  [15:0] a,
    input  [15:0] b,
    output [16:0] s
);
    wire [3:-1] C;
    assign C[-1] = 0;
    genvar i;
    generate
        for (i = 0; i < 4; i++) begin : gen_fa
            FA4 _fa (
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

module FA4 (
    input [3:0] a,
    input [3:0] b,
    input cin,
    output [3:0] s,
    output cout
);
    wire [2:0] cinter;

    FullAdder _0 (
        .a(a[0]),
        .b(b[0]),
        .c(cin),
        .cout(cinter[0]),
        .s(s[0])
    );

    FullAdder _1 (
        .a(a[1]),
        .b(b[1]),
        .c(cinter[0]),
        .cout(cinter[1]),
        .s(s[1])
    );

    FullAdder _2 (
        .a(a[2]),
        .b(b[2]),
        .c(cinter[1]),
        .cout(cinter[2]),
        .s(s[2])
    );

    FullAdder _3 (
        .a(a[3]),
        .b(b[3]),
        .c(cinter[2]),
        .cout(cout),
        .s(s[3])
    );
endmodule

module FullAdder (
    input  a,
    input  b,
    input  c,
    output s,
    output cout
);
    assign {cout, s} = a + b + c;
endmodule

// module fa4test;
//     reg cin;
//     reg [3:0] a, b;
//     wire cout;
//     wire [3:0] s;
//     FA4 _fa (
//         .a(a),
//         .b(b),
//         .cin(cin),
//         .s(s),
//         .cout(cout)
//     );
//     int i, j, k;
//     initial begin
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

// module fatest;
//     reg a, b, c;
//     wire s, cout;
//     FullAdder _fa (
//         .a(a),
//         .b(b),
//         .c(c),
//         .s(s),
//         .cout(cout)
//     );
//     initial begin
//         $monitor(a, b, c, " => ", cout, s);
//         {a, b, c} = #1 0;
//         {a, b, c} = #1 1;
//         {a, b, c} = #1 2;
//         {a, b, c} = #1 3;
//         {a, b, c} = #1 4;
//         {a, b, c} = #1 5;
//         {a, b, c} = #1 6;
//         {a, b, c} = #1 7;
//     end
// endmodule


// module addrtest;
//     reg [15:0] a, b;
//     wire [16:0] s;
//     NormalAdder _addr (
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
