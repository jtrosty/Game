// #include "core.h"
// #include "sim_region.h"
#if !defined(CORE_ENTITY_H)

#define InvalidPos V2(100000.0f, 1000000.0f)

static bool32 isSet(Sim_Entity* entity, u32 flag) {
  bool32 result = entity->flags & flag;

  return result;
}

static bool32 addFlag(Sim_Entity* entity, u32 flag) { entity->flags |= flag; }
static void clearFlag(Sim_Entity* entity, u32 flag) { entity->flags &= ~flag; }
static void makeEntityNonSpatial(Sim_Entity* entity, u32 flag) {
  entity->flags &= ~flag;
}

#define CORE_ENTITY_H
#endif
