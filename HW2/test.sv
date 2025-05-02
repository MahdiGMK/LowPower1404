module test #(
    parameter int SZ  = 32,
    parameter int ASZ = 4,
    parameter int DSZ = 8
) (
    input _rst,
    input clk,
    output out_clk,
    input [SZ-1:0] a,
    input [SZ-1:0] b,
    output [2*SZ-1:0] res,
    // AW
    output reg [ASZ-1:0] awaddr,
    output reg awvalid,
    input awready,
    // W
    output reg [DSZ-1:0] wdata,
    output reg wvalid,
    input wready,
    // B
    input bresp,  // 1 => ok
    input bvalid,
    output reg bready,
    // AR
    output reg [ASZ-1:0] araddr,
    output reg arvalid,
    input arready,
    // R
    input [DSZ-1:0] rdata,
    input rvalid,
    output reg rready,
    input rresp  // 1 => ok
);
    assign out_clk = clk;

    wire [DSZ-1:0] outregs[(2) * (SZ / DSZ)];
    assign outregs[0] = a[7:0];
    assign outregs[1] = a[15:8];
    assign outregs[2] = a[23:16];
    assign outregs[3] = a[31:24];
    assign outregs[4] = b[7:0];
    assign outregs[5] = b[15:8];
    assign outregs[6] = b[23:16];
    assign outregs[7] = b[31:24];

    reg [DSZ-1:0] inregs[2 * SZ / DSZ];
    assign res = {
        inregs[7], inregs[6], inregs[5], inregs[4], inregs[3], inregs[2], inregs[1], inregs[0]
    };


    int i;
    always @(posedge clk, negedge _rst) begin
        if (~_rst) begin
            wdata  <= 0;
            wvalid <= 0;
            bready <= 0;
            rready <= 0;
            for (i = 0; i < 2 * SZ / DSZ; i = i + 1) begin
                inregs[i] <= 0;
            end
            // setup data write
            awaddr  <= 0;
            awvalid <= 1;
            // setup result read
            araddr  <= 0;
            arvalid <= 1;
        end else begin

            if (awvalid & awready) begin
`ifdef VERBOSE
                $display($time, " | master | ", "snd wdata : ", outregs[awaddr], " ind : %1d",
                         awaddr);
`endif
                awvalid <= 0;
                wdata   <= outregs[awaddr];
                wvalid  <= 1;
            end
            if (wvalid & wready) begin
                wvalid <= 0;
                wdata  <= 0;
                bready <= 1;
            end
            if (bready & bvalid) begin
`ifdef VERBOSE
                $display($time, " | master | ", "bresp : ", bresp);
`endif
                bready  <= 0;
                awvalid <= 1;
                awaddr  <= (awaddr + 1) & 7;
            end

            if (arvalid & arready) begin
                if (araddr == 0) begin
`ifdef VERBOSE
                    $display($time, " | master | ", "res : ", res);
`endif
                end
                arvalid <= 0;
                rready  <= 1;
            end
            if (rready & rvalid) begin
`ifdef VERBOSE
                $display($time, " | master | ", "rcv rdata : ", rdata, " ind : %1d", araddr);
`endif
                inregs[araddr] <= rdata;
                rready <= 0;
                arvalid <= 1;
                araddr <= (araddr + 1) & 7;
            end

        end
    end


endmodule
