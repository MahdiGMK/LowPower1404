module test;
endmodule

module test_UMull;
    reg clk, rst, strt;
    reg [5:0] a, b;
    wire [11:0] res;
    wire done;
    UMull #(
        .N(6)
    ) _mul (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a(a),
        .b(b),
        .res(res),
        .done(done)
    );
    initial begin
        clk = 0;
        forever #1 clk = ~clk;
    end
    int i, j;
    initial begin
        rst = 1;
        #2 rst = 0;
        for (i = 0; i < 64; i++)
        for (j = 0; j < 64; j++) begin
            a = i;
            b = j;
            strt = 1;
            #2 strt = 0;
            #10 assert (done);
            assert (res == (i * j));
        end
        $finish();
    end
endmodule

module test_UKarat;
    reg clk, rst, strt;
    reg [5:0] a, b;
    wire [11:0] res;
    wire done;
    UKarat #(
        .N(6)
    ) _mul (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a(a),
        .b(b),
        .res(res),
        .done(done)
    );
    initial begin
        clk = 0;
        forever #1 clk = ~clk;
    end
    int i, j;
    initial begin
        rst = 1;
        #2 rst = 0;
        // $monitor(a, " * ", b, " => ", res);
        for (i = 0; i < 64; i++)
        for (j = 0; j < 64; j++) begin
            a = i;
            b = j;
            strt = 1;
            #2 strt = 0;
            #10 assert (done);
            assert (res == (i * j));
        end
        $finish();
    end
endmodule

module test_ShaddMul;
    reg clk, rst, strt;
    reg [127:0] a, b;
    wire signed [255:0] res;
    wire done;
    ShaddMul _mul (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a(a),
        .b(b),
        .res(res),
        .done(done)
    );
    initial begin
        clk = 0;
        forever #1 clk = ~clk;
    end
    int i, j;
    initial begin
        rst = 1;
        #2 rst = 0;
        for (i = -128; i < 128; i++)
        for (j = -128; j < 128; j++) begin
            a = i;
            b = j;
            strt = 1;
            #2 strt = 0;
            #20 assert (done);
            assert (res == (i * j));
        end
        $finish();
    end
endmodule

module test_KaratMul;
    reg clk, rst, strt;
    reg [127:0] a, b;
    wire signed [255:0] res;
    wire done;
    KaratMul _mul (
        .clk(clk),
        .rst(rst),
        .strt(strt),
        .a(a),
        .b(b),
        .res(res),
        .done(done)
    );
    initial begin
        clk = 0;
        forever #1 clk = ~clk;
    end
    int i, j;
    initial begin
        rst = 1;
        #2 rst = 0;
        for (i = -128; i < 128; i++)
        for (j = -128; j < 128; j++) begin
            a = i;
            b = j;
            strt = 1;
            #2 strt = 0;
            #20 assert (done);
            assert (res == (i * j));
        end
        $finish();
    end
endmodule

module test_INPGEN;
    wire [127:0] a, b;
    reg clk, rst;
    INPGEN _inp (
        .clk(clk),
        .rst(rst),
        .a  (a),
        .b  (b)
    );
    initial begin
        clk = 0;
        forever #1 clk = ~clk;
    end
    initial begin
        rst = 1;
        #2 rst = 0;
        $monitor("%b\n\n%b\n", a, b);
        #20 $finish();
    end
endmodule
