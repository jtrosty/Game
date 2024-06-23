#if !defined(UNITY_BUILD_ERROR_REMOVER)
#include "core.h"
#endif
#include "sim_region.h"

static Sim_Entity_Hash* getHashFromStorageIndex(Sim_Region* sim_region,
                                                u32 storage_index) {
  Assert(storage_index);

  Sim_Entity_Hash* result = 0;

  u32 hash_value = storage_index;
  for (u32 offset = 0; offset < ArrayCount(sim_region->hash); ++offset) {
    u32 hash_mask = (ArrayCount(sim_region->hash) - 1);
    u32 hash_index = ((hash_value + offset) & hash_mask);

    Sim_Entity_Hash* entry = sim_region->hash + hash_index;

    if ((entry->index == 0) || (entry->index == storage_index)) {
      result = entry;
      break;
    }
  }
  return result;
}

static Move_Spec defaultMoveSpec(void) {
  Move_Spec result;
  result.unit_max_accel_vector = false;
  result.speed = 1.0f;
  result.drag = 0.0f;

  return result;
}

static void mapStorageIndexToEntity(Sim_Region* sim_region, u32 storage_index,
                                    Sim_Entity* entity) {
  Sim_Entity_Hash* entry = getHashFromStorageIndex(sim_region, storage_index);
  Assert((entry->index == 0) || entry->index == storage_index);
  entry->index = storage_index;
  entry->ptr = entity;
}

static Sim_Entity* getEntityByStorageIndex(Sim_Region* sim_region,
                                           u32 storage_index) {
  Sim_Entity_Hash* entry = getHashFromStorageIndex(sim_region, storage_index);
  Sim_Entity* result = entry->ptr;
  return result;
}

inline v2 getSimRegionSpaceP(Sim_Region* sim_region, Low_Entity* stored) {
  v2 result = InvalidPos;
  if (!isSet(&stored->sim, Entity_Flag_Nonspatial)) {
    World_Difference diff =
        worldSubtract(sim_region->world, &stored->p, &sim_region->origin);
    result = diff.dXY;
  }

  return result;
}

// NOTE: This is here to make sure the function works belwo in teh loadEntity
// Reference
static Sim_Entity* addEntity(Game_State* game_state, Sim_Region* sim_region,
                             u32 storage_index, Low_Entity* source, v2* sim_p);
static void loadEntityReference(Game_State* game_state, Sim_Region* sim_region,
                                Entity_Reference* ref) {
  if (ref->index) {
    Sim_Entity_Hash* entry = getHashFromStorageIndex(sim_region, ref->index);
    if (entry->ptr == 0) {
      entry->index = ref->index;
      Low_Entity* low_entity = getLowEntity(game_state, ref->index);
      v2 p = getSimRegionSpaceP(sim_region, low_entity);
      entry->ptr =
          addEntity(game_state, sim_region, ref->index, low_entity, &p);
    }
    ref->sim_ptr = entry->ptr;
  }
}

static void storeEntityReference(Entity_Reference* ref) {
  if (ref->sim_ptr != 0) {
    ref->index = ref->sim_ptr->storage_index;
  }
}

static Sim_Entity* addEntityRaw(Game_State* game_state, Sim_Region* sim_region,
                                u32 storage_index, Low_Entity* source) {
  Assert(storage_index);
  Sim_Entity* entity = 0;

  Sim_Entity_Hash* entry = getHashFromStorageIndex(sim_region, storage_index);
  if (entry->ptr == 0) {
    if (sim_region->entity_count < sim_region->max_entity_count) {
      entity = sim_region->entities + sim_region->entity_count++;

      entry->index = storage_index;
      entry->ptr = entity;

      if (source) {
        *entity = source->sim;
        // loadEntityReference(game_state, sim_region, &entity->sword);
        Assert(!isSet(&source->sim, Entity_Flag_Simming));
        addFlag(&source->sim, Entity_Flag_Simming);
      }
      entity->storage_index = storage_index;
      entity->updatable = false;
    } else {
      InvalidCodePath;
    }
  }
  return entity;
}

static Sim_Entity* addEntity(Game_State* game_state, Sim_Region* sim_region,
                             u32 storage_index, Low_Entity* source,
                             v2* sim_pos) {

  Sim_Entity* dest =
      addEntityRaw(game_state, sim_region, storage_index, source);
  if (dest) {
    if (sim_pos) {
      dest->p = *sim_pos;
      dest->updatable = isInRectangle(sim_region->updatable_bounds, dest->p);
    } else {
      dest->p = getSimRegionSpaceP(sim_region, source);
    }
  }
  return dest;
}

static Sim_Region* beginSim(Game_State* game_state, Memory_Arena* sim_arena,
                            World* world, World_Position origin,
                            Rectangle2 region_bounds) {
  // TODO: Think about how we place these into memory
  //
  Sim_Region* sim_region = pushStruct(sim_arena, Sim_Region);
  zeroStruct(sim_region->hash);

  // TODO: Calculate this eventually from the max value of all
  // entities radius plus their speed
  real32 update_safety_margin = 1.0f;

  sim_region->world = world;
  sim_region->origin = origin;
  sim_region->updatable_bounds = region_bounds;
  sim_region->region_bounds = addRadiusTo(
      sim_region->updatable_bounds, update_safety_margin, update_safety_margin);

  // TODO be more specidfic about entity counts
  sim_region->max_entity_count = 4096;
  sim_region->entity_count = 0;
  sim_region->entities =
      pushArray(sim_arena, sim_region->max_entity_count, Sim_Entity);

  World_Position min_chunk_p = mapIntoChunkSpace(
      world, sim_region->origin, getMinCorner(sim_region->region_bounds));
  World_Position max_chunk_p = mapIntoChunkSpace(
      world, sim_region->origin, getMaxCorner(sim_region->region_bounds));

  for (i32 chunk_y = min_chunk_p.chunk_y; chunk_y <= max_chunk_p.chunk_y;
       ++chunk_y) {

    for (i32 chunk_x = min_chunk_p.chunk_x; chunk_x <= max_chunk_p.chunk_x;
         ++chunk_x) {

      World_Chunk* chunk =
          getWorldChunk(world, chunk_x, chunk_y, sim_region->origin.chunk_z);
      if (chunk) {
        for (World_Entity_Block* block = &chunk->first_block; block;
             block = block->next) {

          for (u32 entity_index_index = 0;
               entity_index_index < block->entity_count; ++entity_index_index) {

            u32 low_entity_index = block->low_entity_index[entity_index_index];
            Low_Entity* low = game_state->low_entities + low_entity_index;
            if (!isSet(&low->sim, Entity_Flag_Nonspatial)) {

              v2 sim_space_p = getSimRegionSpaceP(sim_region, low);
              if (isInRectangle(sim_region->region_bounds, sim_space_p)) {

                addEntity(game_state, sim_region, low_entity_index, low,
                          &sim_space_p);
              }
            }
          }
        }
      }
    }
  }
  return sim_region;
}

static void endSim(Game_State* game_state, Sim_Region* region) {
  //(NOTE) If we move low entities to world, this would not need game state.
  Sim_Entity* entity = region->entities;
  for (u32 entity_index = 0; entity_index < region->entity_count;
       ++entity_index, ++entity) {

    Low_Entity* stored = game_state->low_entities + entity->storage_index;

    Assert(isSet(&stored->sim, Entity_Flag_Simming));
    stored->sim = *entity;
    Assert(!isSet(&stored->sim, Entity_Flag_Simming));

    storeEntityReference(&stored->sim.sword);

    World_Position new_pos =
        isSet(entity, Entity_Flag_Simming)
            ? nullPosition()
            : mapIntoChunkSpace(game_state->world, region->origin, entity->p);

    changeEntityLocation(&game_state->world_arena, game_state->world,
                         entity->storage_index, stored, new_pos);

    if (entity->storage_index == game_state->camera_following_entity_index) {

      World_Position new_camera_pos = game_state->camera_pos;
      new_camera_pos.chunk_z = stored->p.chunk_z;

      new_camera_pos = stored->p;
      game_state->camera_pos = new_camera_pos;
    }
  }
}

static bool32 testWall(real32 wall_x, real32 rel_x, real32 rel_y,
                       real32 player_delta_x, real32 player_delta_y,
                       real32* t_min, real32 min_y, real32 max_y) {
  bool32 hit = false;

  real32 t_epsilon = 0.01f;
  // We make sure there is movement
  if (player_delta_x != 0.0f) {
    real32 t_result = (wall_x - rel_x) / player_delta_x;
    real32 y = rel_y + t_result * player_delta_y;
    if ((t_result >= 0.0f) && (*t_min > t_result)) {
      if ((y >= min_y) && (y <= max_y)) {

        *t_min = Maximum(0.0f, t_result - t_epsilon);
        hit = true;
      }
    }
  }
  return hit;
}

internal void moveEntity(Sim_Region* sim_region, Sim_Entity* entity, real32 dt,
                         Move_Spec* move_spec, v2 ddp) {

  Assert(!isSet(entity, Entity_Flag_Nonspatial));

  World* world = sim_region->world;

  if (move_spec->unit_max_accel_vector) {
    real32 ddp_length = math_lenghtSq(ddp);
    if (ddp_length > 1.0f) {
      ddp *= (1.0f / squareRoot(ddp_length));
    }
  }
  // ODEs here
  ddp *= move_spec->speed;
  ddp += -move_spec->drag * entity->dP;

  v2 old_player_pos = entity->p;
  v2 player_delta = (0.5f * ddp * math_square(dt) + entity->dP * dt);
  entity->dP = ddp * dt + entity->dP;
  v2 new_player_pos = old_player_pos + player_delta;

  for (u32 iteration = 0; (iteration < 4); ++iteration) {
    real32 t_min = 1.0f;
    v2 wall_normal = {};
    Sim_Entity* hit_entity = 0;

    v2 desired_position = entity->p + player_delta;

    if (entity->collides) {

      for (u32 test_sim_entity_index = 0;
           test_sim_entity_index < sim_region->entity_count;
           ++test_sim_entity_index) {

        Sim_Entity* test_entity = sim_region->entities + test_sim_entity_index;
        if (entity != test_entity) {

          if (test_entity->collides) {

            real32 diameter_w = test_entity->width + entity->width;
            real32 diameter_h = test_entity->height + entity->height;

            v2 min_corner = -0.5f * v2{diameter_w, diameter_h};
            v2 max_corner = 0.5f * v2{diameter_w, diameter_h};

            v2 rel = entity->p - test_entity->p;

            if (testWall(min_corner.x, rel.x, rel.y, player_delta.x,
                         player_delta.y, &t_min, min_corner.y, max_corner.y)) {
              wall_normal = v2{-1, 0};
              hit_entity = test_entity;
            }
            if (testWall(max_corner.x, rel.x, rel.y, player_delta.x,
                         player_delta.y, &t_min, min_corner.y, max_corner.y)) {

              wall_normal = v2{1, 0};
              hit_entity = test_entity;
            }
            if (testWall(min_corner.y, rel.y, rel.x, player_delta.y,
                         player_delta.x, &t_min, min_corner.x, max_corner.x)) {

              wall_normal = v2{0, -1};
              hit_entity = test_entity;
            }
            if (testWall(max_corner.y, rel.y, rel.x, player_delta.y,
                         player_delta.x, &t_min, min_corner.x, max_corner.x)) {

              wall_normal = v2{0, 1};
              hit_entity = test_entity;
            }
          }
        }
      }
    }
    entity->p += t_min * player_delta;
    if (hit_entity) {
      entity->dP =
          entity->dP - 1 * math_inner(entity->dP, wall_normal) * wall_normal;
      player_delta = desired_position - entity->p;
      player_delta = player_delta -
                     1 * math_inner(player_delta, wall_normal) * wall_normal;
    } else {
      break;
    }
  }
  // NOTE: Update camera/player Z based on last movement
  // TODO(casey): Change to using the acceleration vector
  if ((entity->dP.x == 0.0f) && (entity->dP.y == 0.0f)) {
    // NOTE(casey): Leave FacingDirection whatever it was
  } else if (absoluteValue(entity->dP.x) > absoluteValue(entity->dP.y)) {
    // NOTE (Jon) facing directions
    if (entity->dP.x > 0) {
    } else {
    }
  } else {
    if (entity->dP.y > 0) {
    } else {
    }
  }

  // TODO delete the below, irrellavent?
  /*
  World_Position new_p = mapIntoChunkSpace(game_state->world,
  game_state->camera_pos, entity.high->p);

  changeEntityLocation(&game_state->world_arena, game_state->world,
  entity.low_index, entity.low, &entity.low->p, &new_p);
  */
}
