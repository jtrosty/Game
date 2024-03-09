#if !defined(CORE_ENTITY_H)

#define InvalidPos V2(100000.0f, 1000000.0f)

static bool32 isSet(Sim_Entity* entity, u32 flag) {
  bool32 result = entity->flags & flag;

  return result;
}

static void addFlag(Sim_Entity* entity, u32 flag) { entity->flags |= flag; }

static void clearFlag(Sim_Entity* entity, u32 flag) { entity->flags &= ~flag; }

static void makeEntityNonSpatial(Sim_Entity* entity) {
  addFlag(entity, Entity_Flag_Nonspatial);
  entity->p = InvalidPos;
}

static void makeEntityNonSpatial(Sim_Entity* entity, v2 p, v2 dp) {
  clearFlag(entity, Entity_Flag_Nonspatial);
  entity->p = InvalidPos;
  entity->dP = dp;
}

#define CORE_ENTITY_H
#endif
