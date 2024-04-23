
const std = @import("std");
const options = @import("zcsg_options");
const c = @cImport({
    @cInclude("ccsg.h");
});

//--------------------------------------------------------------------------------------------------
// Init/deinit and global state
//--------------------------------------------------------------------------------------------------
const SizeAndAlignment = packed struct(u64) {
    size: u48,
    alignment: u16,
};
var mem_allocator: ?std.mem.Allocator = null;
var mem_allocations: ?std.AutoHashMap(usize, SizeAndAlignment) = null;
var mem_mutex: std.Thread.Mutex = .{};
const mem_alignment = 16;

pub fn init_allocator(allocator: std.mem.Allocator) !void {
    std.debug.assert(options.use_custom_alloc);
    if (!options.use_custom_alloc) return;

    std.debug.assert(mem_allocator == null and mem_allocations == null);

    mem_allocator = allocator;
    mem_allocations = std.AutoHashMap(usize, SizeAndAlignment).init(allocator);
    try mem_allocations.?.ensureTotalCapacity(32);

    c.CCSG_RegisterCustomAllocator(zcsgAlloc, zcsgFree, zcsgAlignedAlloc, zcsgFree);
}

pub fn deinit_allocator() void {
    std.debug.assert(options.use_custom_alloc);
    if (!options.use_custom_alloc) return;

    mem_allocations.?.deinit();
    mem_allocations = null;
    mem_allocator = null;
}

//--------------------------------------------------------------------------------------------------
// Memory allocation
//--------------------------------------------------------------------------------------------------
fn zcsgAlloc(size: usize) callconv(.C) ?*anyopaque {
    mem_mutex.lock();
    defer mem_mutex.unlock();

    const ptr = mem_allocator.?.rawAlloc(
        size,
        std.math.log2_int(u29, @as(u29, @intCast(mem_alignment))),
        @returnAddress(),
    );
    if (ptr == null) @panic("zcsg: out of memory");

    mem_allocations.?.put(
        @intFromPtr(ptr),
        .{ .size = @as(u48, @intCast(size)), .alignment = mem_alignment },
    ) catch @panic("zcsg: out of memory");

    return ptr;
}

fn zcsgAlignedAlloc(size: usize, alignment: usize) callconv(.C) ?*anyopaque {
    mem_mutex.lock();
    defer mem_mutex.unlock();

    const ptr = mem_allocator.?.rawAlloc(
        size,
        std.math.log2_int(u29, @as(u29, @intCast(alignment))),
        @returnAddress(),
    );
    if (ptr == null) @panic("zcsg: out of memory");

    mem_allocations.?.put(
        @intFromPtr(ptr),
        .{ .size = @as(u32, @intCast(size)), .alignment = @as(u16, @intCast(alignment)) },
    ) catch @panic("zcsg: out of memory");

    return ptr;
}

fn zcsgFree(maybe_ptr: ?*anyopaque) callconv(.C) void {
    if (maybe_ptr) |ptr| {
        mem_mutex.lock();
        defer mem_mutex.unlock();

        const info = mem_allocations.?.fetchRemove(@intFromPtr(ptr)).?.value;

        const mem = @as([*]u8, @ptrCast(ptr))[0..info.size];

        mem_allocator.?.rawFree(
            mem,
            std.math.log2_int(u29, @as(u29, @intCast(info.alignment))),
            @returnAddress(),
        );
    }
}

//--------------------------------------------------------------------------------------------------
// World
//--------------------------------------------------------------------------------------------------
pub const World = opaque {
    pub fn create() *World {
        return @as(*World, @ptrCast(c.CCSG_World_Create()));
    }
    pub fn destroy(world: *World) void {
        c.CCSG_World_Destroy(@as(*c.CCSG_World, @ptrCast(world)));
    }
};

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------
const expect = std.testing.expect;

extern fn JoltCTest_HelloWorld() u32;
test "jolt_c.helloworld" {
    const ret = JoltCTest_HelloWorld();
    try expect(ret == 0);
}