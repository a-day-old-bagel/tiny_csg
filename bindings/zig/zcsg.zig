
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
// Reinterpreted Types - Must maintain these in sync with csg types
//--------------------------------------------------------------------------------------------------
pub const Volume = i32;
pub const Vec3 = [3]f32;
pub const Mat4 = [16]f32;

pub const Plane = extern struct {
    normal: Vec3,
    offset: f32,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_Plane)); }
};

pub const Face = extern struct {
    plane: *const Plane,
    _pad0: [3]*const anyopaque,
    _pad1: [3]*const anyopaque,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_Face)); }

    pub fn getVertices(face: *const Face) ?[]const Vertex {
        var ptr: [*c]Vertex = null;
        const len = c.CCSG_Face_GetVerticesPtr(
            @as(*const c.CCSG_Face, @ptrCast(face)),
            @as([*c][*c] c.CCSG_Vertex, @ptrCast(&ptr)),
        );
        if (ptr) |array| {
            return array[0..len];
        }
        return null;
    }
    pub fn getFragments(face: *const Face) ?[]const Fragment {
        var ptr: [*c]Fragment = null;
        const len = c.CCSG_Face_GetFragmentsPtr(
            @as(*const c.CCSG_Face, @ptrCast(face)),
            @as([*c][*c] c.CCSG_Fragment, @ptrCast(&ptr)),
        );
        if (ptr) |array| {
            return array[0..len];
        }
        return null;
    }
};

pub const Fragment = extern struct {
    face: *Face,
    _pad0: [3]*const anyopaque,
    front_volume: Volume,
    back_volume: Volume,
    front_brush: *Brush,
    back_brush: *Brush,
    _pad1: i32,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_Fragment)); }

    pub fn getVertices(fragment: *const Fragment) ?[]const Vertex {
        var ptr: [*c]Vertex = null;
        const len = c.CCSG_Fragment_GetVerticesPtr(
            @as(*const c.CCSG_Fragment, @ptrCast(fragment)),
            @as([*c][*c] c.CCSG_Vertex, @ptrCast(&ptr)),
        );
        if (ptr) |array| {
            return array[0..len];
        }
        return null;
    }
};

pub const Ray = extern struct {
    origin: Vec3,
    direction: Vec3,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_Ray)); }
};

pub const RayHit = extern struct {
    brush: *Brush,
    face: *Face,
    fragment: *Fragment,
    parameter: f32,
    position: Vec3,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_RayHit)); }
};

pub const Box = extern struct {
    min: Vec3,
    max: Vec3,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_Box)); }
};

pub const Vertex = extern struct {
    faces: [3]*const Face,
    position: Vec3,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_Vertex)); }
};

pub const Triangle = extern struct {
    i: i32,
    j: i32,
    k: i32,

    comptime { std.debug.assert(@sizeOf(@This()) == @sizeOf(c.CCSG_Triangle)); }
};

//--------------------------------------------------------------------------------------------------
// VolumeOperation
//--------------------------------------------------------------------------------------------------
pub const VolumeOperation = opaque {
    pub fn initFill(volume: Volume) *VolumeOperation {
        return @as(*VolumeOperation, @ptrCast(c.CCSG_MakeFillOperation(volume)));
    }
    pub fn initConvert(from: Volume, to: Volume) *VolumeOperation {
        return @as(*VolumeOperation, @ptrCast(c.CCSG_MakeConvertOperation(from, to)));
    }
    pub fn deinit(operation: *VolumeOperation) void {
        c.CCSG_VolumeOperation_Destroy(@as(*c.CCSG_VolumeOperation, @ptrCast(operation)));
    }
};

//--------------------------------------------------------------------------------------------------
// World
//--------------------------------------------------------------------------------------------------
pub const World = opaque {
    pub fn init() *World {
        return @as(*World, @ptrCast(c.CCSG_World_Create()));
    }
    pub fn deinit(world: *World) void {
        c.CCSG_World_Destroy(@as(*c.CCSG_World, @ptrCast(world)));
    }

    pub fn first(world: *World) ?*Brush {
        return @as(*Brush, @ptrCast(c.CCSG_World_First(@as(*c.CCSG_World, @ptrCast(world)))));
    }
    pub fn next(world: *World, brush: *Brush) ?*Brush {
        return @as(*Brush, @ptrCast(
            c.CCSG_World_Next(@as(*c.CCSG_World, @ptrCast(world)), @as(*c.CCSG_Brush, @ptrCast(brush)))
        ));
    }
    pub fn remove(world: *World, brush: *Brush) void {
        c.CCSG_World_Remove(@as(*c.CCSG_World, @ptrCast(world)), @as(*c.CCSG_Brush, @ptrCast(brush)));
    }
    pub fn add(world: *World) *Brush {
        return @as(*Brush, @ptrCast(c.CCSG_World_Add(@as(*c.CCSG_World, @ptrCast(world)))));
    }

    pub fn rebuild(world: *World) *BrushSet {
        return @as(*BrushSet, @ptrCast(c.CCSG_World_Rebuild(@as(*c.CCSG_World, @ptrCast(world)))));
    }
};

//--------------------------------------------------------------------------------------------------
// Brush
//--------------------------------------------------------------------------------------------------
pub const Brush = opaque {
    pub fn setPlanes(brush: *Brush, planes: []const Plane) void {
        c.CCSG_Brush_SetPlanes(
            @as(*c.CCSG_Brush, @ptrCast(brush)),
            @as(*const c.CCSG_Plane, @ptrCast(planes.ptr)),
            planes.len,
        );
    }
    pub fn getPlanes(brush: *const Brush) *BrushList {
        _ = brush;
    }

    pub fn setVolumeOperation(brush: *Brush, op: *const VolumeOperation) void {
        c.CCSG_Brush_SetVolumeOperation(
            @as(*c.CCSG_Brush, @ptrCast(brush)),
            @as(*const c.CCSG_VolumeOperation, @ptrCast(op)),
        );
    }

    pub fn getFaces(brush: *const Brush) ?[]const Face {
        const vec = c.CCSG_Brush_GetFaces(@as(*const c.CCSG_Brush, @ptrCast(brush))) orelse return null;
        var ptr: [*c]Face = null;
        const len = c.CCSG_FaceVec_GetPtr(vec, @as([*c][*c] c.CCSG_Face, @ptrCast(&ptr)));
        if (ptr) |array| {
            return array[0..len];
        }
        return null;
    }
};

//--------------------------------------------------------------------------------------------------
// STL Container Wrappers
//--------------------------------------------------------------------------------------------------
pub const BrushSet = opaque {
    pub fn deinit(set: *BrushSet) void {
        c.CCSG_BrushSet_Destroy(@as(*c.CCSG_BrushSet, @ptrCast(set)));
    }
    pub fn iterator(set: *BrushSet) *Iterator {
        return @as(*Iterator, @ptrCast(c.CCSG_BrushSet_Iterator_Begin(@as(*c.CCSG_BrushSet, @ptrCast(set)))));
    }

    pub const Iterator = opaque {
        pub fn deinit(self: *Iterator) void {
            c.CCSG_BrushSet_Iterator_Destroy(@as(*c.CCSG_BrushSet_Iterator, @ptrCast(self)));
        }
        pub fn next(self: *Iterator, set: *BrushSet) ?*const Brush {
            const current = c.CCSG_BrushSet_Iterator_Read(@as(*const c.CCSG_BrushSet_Iterator, @ptrCast(self)));
            const exists = c.CCSG_BrushSet_Iterator_Next(
                @as(*c.CCSG_BrushSet, @ptrCast(set)),
                @as(*c.CCSG_BrushSet_Iterator, @ptrCast(self)),
            );
            if (exists == 0) return null;
            return @as(*const Brush, @ptrCast(current));
        }
    };
};

pub const BrushList = opaque {
    pub fn deinit(list: *BrushList) void {
        _ = list;
    }
    pub fn getSlice(list: *BrushList) []*const Brush {
        _ = list;
    }
};

pub const RayHitList = opaque {
    pub fn deinit(list: *RayHitList) void {
        _ = list;
    }
    pub fn getSlice(list: *RayHitList) []*const RayHit {
        _ = list;
    }
};

pub const TriangleList = opaque {
    pub fn deinit(list: *TriangleList) void {
        c.CCSG_TriangleVec_Destroy(@as(*c.CCSG_TriangleVec, @ptrCast(list)));
    }
    pub fn getSlice(list: *TriangleList) ?[]const Triangle {
        var ptr: [*c]Triangle = null;
        const len = c.CCSG_TriangleVec_GetPtr(
            @as(*const c.CCSG_TriangleVec, @ptrCast(list)),
            @as([*c][*c] c.CCSG_Triangle, @ptrCast(&ptr)),
        );
        if (ptr) |array| {
            return array[0..len];
        }
        return null;
    }
};

//--------------------------------------------------------------------------------------------------
// Misc.
//--------------------------------------------------------------------------------------------------
pub fn triangulate(fragment: *const Fragment) *TriangleList {
    return @as(*TriangleList, @ptrCast(c.CCSG_Triangulate(@as(*const c.CCSG_Fragment, @ptrCast(fragment)))));
}

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------
const expect = std.testing.expect;

test "decls" {
    std.testing.refAllDeclsRecursive(@This());
}

extern fn CCSG_Test_HelloWorld() u32;
test "ccsg.helloworld" {
    if (options.use_custom_alloc) try init_allocator(std.testing.allocator);
    const ret = CCSG_Test_HelloWorld();
    try expect(ret == 0);
    if (options.use_custom_alloc) deinit_allocator();
}

test "helloworld" {
    if (options.use_custom_alloc) try init_allocator(std.testing.allocator);
    const world = World.init();
    world.deinit();
    if (options.use_custom_alloc) deinit_allocator();
}