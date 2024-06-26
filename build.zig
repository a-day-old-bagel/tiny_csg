const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});
    const target = b.standardTargetOptions(.{});

    const options = .{
        .use_custom_alloc = b.option(
            bool,
            "use_custom_alloc",
            "Replace the new and delete operators for C++",
        ) orelse true,
    };

    const options_step = b.addOptions();
    inline for (std.meta.fields(@TypeOf(options))) |field| {
        options_step.addOption(field.type, field.name, @field(options, field.name));
    }

    const options_module = options_step.createModule();

    const zcsg = b.addModule("root", .{
        .root_source_file = b.path("bindings/zig/zcsg.zig"),
        .target = target,
        .optimize = optimize,
        .imports = &.{
            .{ .name = "zcsg_options", .module = options_module },
        },
    });
    zcsg.addIncludePath(b.path("bindings/c"));

    const ccsg = b.addStaticLibrary(.{
        .name = "ccsg",
        .target = target,
        .optimize = optimize,
    });
    b.installArtifact(ccsg);

    if (options.use_custom_alloc) ccsg.defineCMacro("CSG_CUSTOM_ALLOCATOR_HEADER", "\"bindings/c/ccsg_memory.hpp\"");
    ccsg.addIncludePath(b.path("./"));
    ccsg.addIncludePath(b.path("3rdp/glm"));
    ccsg.linkLibC();
    if (target.result.abi != .msvc)
        ccsg.linkLibCpp();

    ccsg.addCSourceFiles(.{
        .files = &.{
            "bindings/c/ccsg.cpp",
            "csg.cpp",
            "query_box.cpp",
            "query_frustum.cpp",
            "query_point.cpp",
            "query_ray.cpp",
            "rebuild.cpp",
        },
        .flags = &.{
            "-std=c++20",
            "-fno-access-control",
            "-fno-sanitize=undefined",
        },
    });

    const test_step = b.step("test", "Run zcsg tests");
    const tests = b.addTest(.{
        .name = "zscg-tests",
        .root_source_file = b.path("bindings/zig/zcsg.zig"),
        .target = target,
        .optimize = optimize,
    });
    b.installArtifact(tests);

    // TODO: Problems with LTO on Windows.
    if (target.result.os.tag == .windows) {
        tests.want_lto = false;
    }

    if (options.use_custom_alloc) tests.defineCMacro("CSG_CUSTOM_ALLOCATOR_HEADER", "\"bindings/c/ccsg_memory.hpp\"");
    tests.addCSourceFile(.{
        .file = b.path("bindings/c/ccsg_tests.c"),
        .flags = &.{
            "-fno-sanitize=undefined",
        },
    });

    tests.root_module.addImport("zcsg_options", options_module);
    tests.addIncludePath(b.path("bindings/c"));
    tests.linkLibrary(ccsg);

    test_step.dependOn(&b.addRunArtifact(tests).step);
}
