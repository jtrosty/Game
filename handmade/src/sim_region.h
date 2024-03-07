#if !defined(CORE_SIM_REGION_H)

// TODO delete this
// #include "core.h"
// #include "core_world.h"
// #include "core.cpp"
// 1. Sim Region -
//      a. Only update entities in high frequency.
//      b. have active view window, the player sees where all entities are
//      turned high and updated. c. have sim region, which is not in view of
//      player and changes each frame to continue updating other parts of the
//      world. d. Make the sim and active view update areas larger than teh view
//      window of the player. e.  Video 63 42:39

struct Move_Spec {
  bool32 unit_max_accel_vector;
  real32 speed;
  real32 drag;
};

#define HIT_POINT_SUB_SCOUNT 4
struct Hit_Point {
  u8 flags;
  u8 filled_amount;
};

enum Entity_Type {
  entityType_Null,

  entityType_Hero,
  entityType_Wall,
  entityType_Familiar,
  entityType_Monster,
  entityType_Sword,
};

struct Sim_Entity;
union Entity_Reference {
  Sim_Entity* sim_ptr;
  u32 index;
};

enum Sim_Entity_Flags {
  Entity_Flag_Collides = (0 << 1),
  Entity_Flag_Nonspatial = (1 << 1),
  Entity_Flag_Simming = (1 << 30),
};

struct Sim_Entity {
  u32 storage_index;
  bool32 updatable;

  Entity_Type type;
  u32 flags;

  v2 p;
  u32 chunk_z;

  real32 z;
  real32 dZ;

  v2 dP;
  real32 width;
  real32 height;
  real32 t_bob; //????
  bool32 collides;
  i32 d_abs_tile_z;
  u32 hit_points;
  Hit_Point hit_point[16];

  Entity_Reference sword;
  real32 distance_remaining;
};

struct Sim_Entity_Hash {
  Sim_Entity* ptr;
  u32 index;
};

struct Sim_Region {
  World* world;

  World_Position origin;
  Rectangle2 region_bounds;

  u32 max_entity_count;
  u32 entity_count;
  Sim_Entity* entities;

  Sim_Entity_Hash hash[4096];
};

#define CORE_SIM_REGION_H
#endif
