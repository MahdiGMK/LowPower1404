//! By convention, root.zig is the root source file when making a library. If
//! you are making an executable, the convention is to delete this file and
//! start with main.zig instead.
const std = @import("std");
const testing = std.testing;

const LevelSignalling = std.DynamicBitSet;
const TransitionSignalling = std.DynamicBitSet;

pub fn num2Bitset(allocator: std.mem.Allocator, val: anytype) !std.DynamicBitSet {
    // var res = std.DynamicBitSet.initEmpty(allocator, );
    const T = @TypeOf(val);
    const info = @typeInfo(T);
    switch (info) {
        .int => {
            if (info.int.signedness == .signed) {
                @compileError("Signed integers not supported");
            }
            var res = try std.DynamicBitSet.initEmpty(allocator, info.int.bits);
            for (0..info.int.bits) |i| {
                if (((@as(T, 1) << @truncate(i)) & val) != 0) {
                    res.set(i);
                }
            }
            return res;
        },
        else => @compileError("invalid convertion"),
    }
}
pub fn level2transition(level_data: LevelSignalling) !TransitionSignalling {
    var res = try TransitionSignalling.initEmpty(level_data.allocator, level_data.capacity());
    var last = @as(u1, 0);
    for (0..res.capacity()) |i| {
        const cur = @intFromBool(level_data.isSet(i));
        if ((last ^ cur) != 0) {
            res.set(i);
        }
        last = cur;
    }
    return res;
}

pub fn FBS(BS: type) type {
    return struct {
        bs: BS,
        pub fn format(
            self: @This(),
            comptime fmt: []const u8,
            options: std.fmt.FormatOptions,
            writer: anytype,
        ) !void {
            _ = options;
            const cap = self.bs.capacity();
            for (0..cap) |i| {
                _ = try writer.write(if (self.bs.isSet(if (std.mem.eql(u8, fmt, "b")) i else cap - i - 1)) "1" else "0");
            }
        }
    };
}
pub fn fbs(bitset: anytype) FBS(@TypeOf(bitset)) {
    return FBS(@TypeOf(bitset)){ .bs = bitset };
}

test "test functions" {
    var val0: LevelSignalling = try num2Bitset(testing.allocator, @as(u16, 0b0100111001010110));
    defer _ = val0.deinit();
    var val1: LevelSignalling = try num2Bitset(testing.allocator, @as(u8, 0b01101011));
    defer _ = val1.deinit();
    var val2: LevelSignalling = try num2Bitset(testing.allocator, @as(u5, 0b00111));
    defer _ = val2.deinit();
    // const x: std.io.AnyWriter = undefined;
    // fbs(val0).format("", .{}, x);
    std.debug.print("{b} {b} {b}\n", .{ fbs(val0), fbs(val1), fbs(val2) });
    var val0_trans = try level2transition(val0);
    defer _ = val0_trans.deinit();
    var val1_trans = try level2transition(val1);
    defer _ = val1_trans.deinit();
    var val2_trans = try level2transition(val2);
    defer _ = val2_trans.deinit();
    std.debug.print("{b} {b} {b}\n", .{ fbs(val0_trans), fbs(val1_trans), fbs(val2_trans) });
}

fn testNbits(comptime nbits: u16) !void {
    const Un = std.meta.Int(.unsigned, nbits);
    var worst_case: usize = 0;
    var avg_case: f32 = 0;
    var arena = std.heap.ArenaAllocator.init(testing.allocator);
    defer _ = arena.deinit();

    for (0..(1 << nbits)) |i| {
        var level = try num2Bitset(
            arena.allocator(),
            @as(Un, @truncate(i)),
        );
        defer _ = level.deinit();
        var trans: TransitionSignalling = try level2transition(level);
        defer _ = trans.deinit();
        var trans_of_trans: TransitionSignalling = try level2transition(trans);
        defer _ = trans_of_trans.deinit();
        const res = @min(trans_of_trans.count(), trans.count());
        // std.debug.print("(0){b}:{} | (0){b}:{} => {}\n", .{
        //     fbs(level),
        //     trans.count(),
        //     fbs(trans),
        //     trans_of_trans.count(),
        //     res,
        // });
        worst_case = @max(worst_case, res);
        avg_case += @as(f32, @floatFromInt(res));
    }
    std.debug.print("case {}:\n", .{nbits});
    avg_case /= (1 << nbits);
    std.debug.print("worst-case => {d}\n", .{worst_case});
    std.debug.print("avg-case => {d}\n", .{avg_case});
    std.debug.print("worst-imprv => {d}%\n", .{@as(f32, @floatFromInt(nbits - worst_case)) / nbits * 100});
    std.debug.print("avg-imprv => {d}%\n", .{(nbits * @as(f32, 0.5) - avg_case) / nbits * 2 * 100});
    // std.fmt.format(writer: anytype, comptime fmt: []const u8, args: anytype)
    std.debug.print("\n", .{});
}
test "full test" {
    try testNbits(1);
    try testNbits(2);
    try testNbits(4);
    try testNbits(8);
    try testNbits(12);
    try testNbits(16);
    try testNbits(24);
}

// test "cpe" {
//     num2Bitset(testing.allocator, @as(i8, -5));
// }
