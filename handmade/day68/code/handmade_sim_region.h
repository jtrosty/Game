#if !defined(HANDMADE_SIM_REGION_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct move_spec {
  bool32 UnitMaxAccelVector;
  real32 Speed;
  real32 Drag;
};

enum entity_type {
  EntityType_Null,

  EntityType_Hero,
  EntityType_Wall,
  EntityType_Familiar,
  EntityType_Monstar,
  EntityType_Sword,
};

#define HIT_POINT_SUB_COUNT 4
struct hit_point {
  // TODO(casey): Bake this down into one variable
  uint8 Flags;
  uint8 FilledAmount;
};

// TODO(casey): Rename sim_entity to entity!
struct sim_entity;
union entity_reference {
  sim_entity* Ptr;
  uint32 Index;
};

enum sim_entity_flags {
  // TODO(casey): Does it make more sense to have the flag be for _non_
  // colliding entities?
  EntityFlag_Collides = (1 << 0),
  EntityFlag_Nonspatial = (1 << 1),

  EntityFlag_Simming = (1 << 30),
};

struct sim_entity {
  // NOTE(casey): These are only for the sim region
  uint32 StorageIndex;
  bool32 Updatable;

  //

  entity_type Type;
  uint32 Flags;

  v2 P;
  v2 dP;

  real32 Z;
  real32 dZ;

  real32 DistanceLimit;

  uint32 ChunkZ;

  real32 Width, Height;

  uint32 FacingDirection;
  real32 tBob;

  int32 dAbsTileZ;

  // TODO(casey): Should hitpoints themselves be entities?
  uint32 HitPointMax;
  hit_point HitPoint[16];

  entity_reference Sword;

  // TODO(casey): Generation index so we know how "up to date" this entity is.
};

struct sim_entity_hash {
  sim_entity* Ptr;
  uint32 Index;
};

struct sim_region {
  // TODO(casey): Need a hash table here to map stored entity indices
  // to sim entities!

  world* World;

  world_position Origin;
  rectangle2 Bounds;
  rectangle2 UpdatableBounds;

  uint32 MaxEntityCount;
  uint32 EntityCount;
  sim_entity* Entities;

  // TODO(casey): Do I really want a hash for this??
  // NOTE(casey): Must be a power of two!
  sim_entity_hash Hash[4096];
};

#define HANDMADE_SIM_REGION_H
#endif
