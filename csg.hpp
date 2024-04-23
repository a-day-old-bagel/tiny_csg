#pragma once

#ifndef module_private
#define module_private private
#endif

#ifndef replace_new_delete
#define replace_new_delete
#endif

#ifdef custom_stl_allocator
#define csg_vector(T) std::vector<T, custom_stl_allocator(T)>
#define csg_set(T) std::set<T, std::less<T>, custom_stl_allocator(T)>
#else
#define csg_vector(T) std::vector<T>
#define csg_set(T) std::set<T>
#endif

#include <vector>
#include <set>
#include <functional>
#include <any>

#include <glm/glm.hpp>

namespace csg {

struct world_t;
struct face_t;
struct brush_t;
struct fragment_t;

struct plane_t {
    replace_new_delete
    glm::vec3 normal;
    float     offset;
};

struct ray_t {
    replace_new_delete
    glm::vec3 origin;
    glm::vec3 direction;
};

struct ray_hit_t {
    replace_new_delete
    brush_t    *brush;
    face_t     *face;
    fragment_t *fragment;
    float      parameter;
    glm::vec3  position;
};

struct box_t {
    replace_new_delete
    glm::vec3 min, max;
};

using volume_t = int;
using volume_operation_t = std::function<volume_t(volume_t)>;

volume_operation_t make_fill_operation(volume_t with);
volume_operation_t make_convert_operation(volume_t from, volume_t to);

struct vertex_t {
    replace_new_delete
    face_t    *faces[3];
    glm::vec3 position;
};

struct triangle_t {
    replace_new_delete
    int i,j,k;
};

struct fragment_t {
    replace_new_delete
    face_t                  *face;
    csg_vector(vertex_t)    vertices;
    volume_t                front_volume;
    volume_t                back_volume;
    brush_t                 *front_brush;
    brush_t                 *back_brush;

module_private:
    int                     relation; 
};

csg_vector(triangle_t) triangulate(const fragment_t& fragment);

struct face_t {
    replace_new_delete
    const plane_t           *plane;
    csg_vector(vertex_t)    vertices;
    csg_vector(fragment_t)  fragments;
};

struct brush_t {
    replace_new_delete
    void                        set_planes(const csg_vector(plane_t)& planes);
    const csg_vector(plane_t)&  get_planes() const;
    void                        set_volume_operation(const volume_operation_t& volume_operation);
    const csg_vector(face_t)&   get_faces() const;
    void                        set_time(int time);
    int                         get_time() const;
    int                         get_uid() const;
    box_t                       get_box() const;
    std::any                    userdata;

module_private:
    brush_t() = default;
    ~brush_t() = default;
    brush_t(const brush_t& other) = default;
    brush_t& operator=(const brush_t& other) = default;
    brush_t(brush_t&& other) = default;
    brush_t& operator=(brush_t&& other) = default;
    brush_t               *next;
    brush_t               *prev;
    world_t               *world;
    csg_vector(plane_t)   planes;
    csg_vector(brush_t*)  intersecting_brushes;
    volume_operation_t    volume_operation;
    csg_vector(face_t)    faces;
    box_t                 box;
    int                   time;
    int                   uid;
};

struct world_t {
    replace_new_delete
    world_t();
    ~world_t();
    brush_t                *first();
    brush_t                *next(brush_t *brush);
    void                   remove(brush_t *brush);
    brush_t                *add();
    csg_set(brush_t*)      rebuild();
    void                   set_void_volume(volume_t void_volume);
    volume_t               get_void_volume() const;
    // todo: accelerate queries with bvh
    csg_vector(brush_t*)   query_point(const glm::vec3& point);
    csg_vector(brush_t*)   query_box(const box_t& box);
    csg_vector(ray_hit_t)  query_ray(const ray_t& ray);
    csg_vector(brush_t*)   query_frustum(const glm::mat4& view_projection);
    std::any               userdata;

module_private:
    world_t(const world_t& other) = delete;
    world_t& operator=(const world_t& other) = delete;
    // todo: implement move constructors
    world_t(world_t&& other) = delete;
    world_t& operator=(world_t&& other) = delete;
    brush_t            *sentinel;
    csg_set(brush_t*)  need_face_and_box_rebuild;
    csg_set(brush_t*)  need_fragment_rebuild;
    volume_t           void_volume;
    int                next_uid;
};

} // end namespace csg

#undef module_private