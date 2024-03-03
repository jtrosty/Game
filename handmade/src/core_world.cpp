#pragma once
#include "core_world.h"
#if !defined(ERROR_REMOVER)
#include "core.h"
#endif

#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX / 64)
#define TILE_CHUNK_UNINITIALIZED INT32_MAX

#define TILES_PER_CHUNK 16

inline World_Position nullPosition(void) {
  World_Position result = {};
  result.chunk_x = TILE_CHUNK_UNINITIALIZED;

  return result;
}

inline bool32 isValid(World_Position p) {
  bool32 result = (p.chunk_x != TILE_CHUNK_UNINITIALIZED);
  return result;
}

inline bool32 worldPosIsValid(World_Position p) {
  bool32 result = (p.chunk_x != TILE_CHUNK_UNINITIALIZED);
  return result;
}

inline bool32 isCanonical(World *world, real32 tile_rel) {
  bool32 result = ((tile_rel >= -0.5f * world->chunk_side_in_meters) &&
                   (tile_rel <= 0.5f * world->chunk_side_in_meters));

  return result;
}

inline bool32 isCanonical(World *world, v2 offset) {
  bool32 result = isCanonical(world, offset.x) && isCanonical(world, offset.y);

  return result;
}

inline bool32 areInSameChunk(World *world, World_Position *a,
                             World_Position *b) {
  Assert(isCanonical(world, a->offset));
  Assert(isCanonical(world, b->offset));

  bool32 result = ((a->chunk_x == b->chunk_x) && (a->chunk_y == b->chunk_y) &&
                   (a->chunk_z == b->chunk_z));

  return result;
}

inline World_Chunk *getWorldChunk(World *world, i32 chunk_x, i32 chunk_y,
                                  i32 chunk_z, Memory_Arena *arena = 0) {

  Assert(chunk_x > -TILE_CHUNK_SAFE_MARGIN);
  Assert(chunk_y > -TILE_CHUNK_SAFE_MARGIN);
  Assert(chunk_z > -TILE_CHUNK_SAFE_MARGIN);
  Assert(chunk_x < TILE_CHUNK_SAFE_MARGIN);
  Assert(chunk_y < TILE_CHUNK_SAFE_MARGIN);
  Assert(chunk_z < TILE_CHUNK_SAFE_MARGIN);

  u32 hash_value = 19 * chunk_x + 7 * chunk_y + 3 * chunk_z;
  u32 hash_slot = hash_value & (ArrayCount(world->chunk_hash) - 1);

  Assert(hash_slot < ArrayCount(world->chunk_hash));

  World_Chunk *chunk = world->chunk_hash + hash_slot;

  do {
    if ((chunk_x == chunk->chunk_x) && (chunk_y == chunk->chunk_y) &&
        (chunk_z == chunk->chunk_z)) {
      break;
    }

    if (arena && (chunk->chunk_x != TILE_CHUNK_UNINITIALIZED) &&
        (!chunk->next_in_hash)) {
      chunk->next_in_hash = pushStruct(arena, World_Chunk);
      chunk = chunk->next_in_hash;
      chunk->chunk_x = TILE_CHUNK_UNINITIALIZED;
    }

    if (arena && (chunk->chunk_x == TILE_CHUNK_UNINITIALIZED)) {

      chunk->chunk_x = chunk_x;
      chunk->chunk_y = chunk_y;
      chunk->chunk_z = chunk_z;

      chunk->next_in_hash = 0;

      break;
    }
    chunk = chunk->next_in_hash;
  } while (chunk);

  return chunk;
}
#if 0
inline world_chunk_position
GetChunkPositionFor(world *World, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    tile_chunk_position Result;

    Result.ChunkX = AbsTileX >> World->ChunkShift;
    Result.ChunkY = AbsTileY >> World->ChunkShift;
    Result.ChunkZ = AbsTileZ;
    Result.RelTileX = AbsTileX & World->ChunkMask;
    Result.RelTileY = AbsTileY & World->ChunkMask;

    return(Result);
}
    world->chunk_shift = 4;
    world->chunk_mask = (1 << world->chunk_shift) - 1;
    world->chunk_dim = (1 << world->chunk_shift);
#endif

static void initializeWorld(World *world, real32 tile_side_in_meters) {
  world->tile_side_in_meters = tile_side_in_meters;
  world->chunk_side_in_meters = (real32)TILES_PER_CHUNK * tile_side_in_meters;
  world->first_free = 0;

  for (u32 chunk_index = 0; chunk_index < ArrayCount(world->chunk_hash);
       ++chunk_index) {
    world->chunk_hash[chunk_index].chunk_x = TILE_CHUNK_UNINITIALIZED;
    world->chunk_hash[chunk_index].first_block.entity_count = 0;
  }
}

inline void recanonicalizeCoordinate(World *world, i32 *tile,
                                     real32 *tile_rel) {
  // Assume tile_map is terrodial. tile_map
  // TODO(casey): Need to do something that doesn't use the divide/multiply
  // method for recanonicalizing because this can end up rounding back on to the
  // tile you just came from.

  // NOTE(casey): World is assumed to be toroidal topology, if you
  // step off one end you come back on the other!
  //
  i32 offset = roundReal32ToInt32(
      *tile_rel / world->chunk_side_in_meters); // this will be 1 or -1

  // NOTE(Jon): Worl dis assumed to be toroidal topology. If you step off one
  // end you come back ont he other.
  *tile_rel -= offset * world->chunk_side_in_meters;
  // TODO: Verify this addition happens correctly between u32 and i32
  *tile += offset;

  Assert(isCanonical(world, *tile_rel));
}

static World_Position mapIntoChunkSpace(World *world, World_Position base_pos,
                                        v2 offset) {
  World_Position result = base_pos;

  result.offset += offset;
  recanonicalizeCoordinate(world, &result.chunk_x, &result.offset.x);
  recanonicalizeCoordinate(world, &result.chunk_y, &result.offset.y);

  return result;
}

inline World_Position chunkPositionFromTilePosition(World *world,
                                                    i32 abs_tile_x,
                                                    i32 abs_tile_y,
                                                    i32 abs_tile_z) {
  World_Position result = {};

  result.chunk_x = abs_tile_x / TILES_PER_CHUNK;
  result.chunk_y = abs_tile_y / TILES_PER_CHUNK;
  result.chunk_z = abs_tile_z / TILES_PER_CHUNK;

  if (abs_tile_x < 0) {
    --result.chunk_x;
  }
  if (abs_tile_y < 0) {
    --result.chunk_y;
  }
  if (abs_tile_z < 0) {
    --result.chunk_z;
  }

  result.offset.x = (real32)((abs_tile_x - TILES_PER_CHUNK / 2) -
                             (result.chunk_x * TILES_PER_CHUNK)) *
                    world->tile_side_in_meters;
  result.offset.y = (real32)((abs_tile_y - TILES_PER_CHUNK / 2) -
                             (result.chunk_y * TILES_PER_CHUNK)) *
                    world->tile_side_in_meters;

  Assert(isCanonical(world, result.offset));

  return result;
}

/*
static bool32 areOnSameTile(World_Position* a, World_Position* b) {
    bool32 result =  (a->abs_tile_x == b->abs_tile_x) &&
                     (a->abs_tile_y == b->abs_tile_y) &&
                     (a->abs_tile_z == b->abs_tile_z);
    return result;
}
*/

inline World_Difference worldSubtract(World *world, World_Position *a,
                                      World_Position *b) {
  World_Difference result;
  v2 diff_tile_xy = {(real32)a->chunk_x - (real32)b->chunk_x,
                     (real32)a->chunk_y - (real32)b->chunk_y};
  real32 diff_tile_z = (real32)a->chunk_z - (real32)b->chunk_z;

  result.dXY =
      world->tile_side_in_meters * diff_tile_xy + (a->offset - b->offset);
  result.dz = world->tile_side_in_meters * diff_tile_z;

  return result;
}

inline World_Position centeredChunkPoint(i32 chunk_x, i32 chunk_y,
                                         i32 chunk_z) {
  World_Position result = {};

  result.chunk_x = chunk_x;
  result.chunk_y = chunk_y;
  result.chunk_z = chunk_z;

  return result;
}

inline World_Position centeredChunkPoint(u32 chunk_x, u32 chunk_y,
                                         u32 chunk_z) {
  World_Position result = {};

  result.chunk_x = chunk_x;
  result.chunk_y = chunk_y;
  result.chunk_z = chunk_z;

  return result;
}

#define InvalidCodePath Assert(!"InvalidCodePath");
inline void changeEntityLocationRaw(Memory_Arena *arena, World *world,
                                    u32 low_entity_index, World_Position *old_p,
                                    World_Position *new_p) {
  // TODO(casey): If this moves an entity into the camera bounds, should it
  // automatically go into the high set immediately? If it moves _out_ of the
  // camera bounds, should it be removed from the high set immediately?

  Assert(!old_p || isValid(*old_p));
  Assert(!new_p || isValid(*new_p));

  if (old_p && new_p && areInSameChunk(world, old_p, new_p)) {

  } else {
    if (old_p) {

      World_Chunk *chunk = getWorldChunk(world, new_p->chunk_x, new_p->chunk_y,
                                         new_p->chunk_z, arena);
      Assert(chunk);

      if (chunk) {
        bool32 not_found = true;
        World_Entity_Block *first_block = &chunk->first_block;

        for (World_Entity_Block *block = first_block; block && not_found;
             block = block->next) {

          for (u32 index = 0; (index < block->entity_count) && not_found;
               ++index) {

            if (block->low_entity_index[index] == low_entity_index) {
              Assert(first_block->entity_count > 0);
              block->low_entity_index[index] =
                  first_block->low_entity_index[--first_block->entity_count];

              if (first_block->entity_count == 0) {

                if (first_block->next) {
                  World_Entity_Block *next_block = first_block->next;
                  *first_block = *next_block;

                  next_block->next = world->first_free;
                  world->first_free = next_block;
                }
              }
              not_found = false;
            }
          }
        }
      }
    }

    if (new_p) {
      World_Chunk *chunk = getWorldChunk(world, new_p->chunk_x, new_p->chunk_y,
                                         new_p->chunk_z, arena);
      Assert(chunk);

      World_Entity_Block *block = &chunk->first_block;
      if (block->entity_count == ArrayCount(block->low_entity_index)) {
        World_Entity_Block *old_block = world->first_free;
        if (old_block) {
          world->first_free = old_block->next;
        } else {
          old_block = pushStruct(arena, World_Entity_Block);
        }
        *old_block = *block;
        block->next = old_block;
        block->entity_count = 0;
      }
      Assert(block->entity_count < ArrayCount(block->low_entity_index));
      block->low_entity_index[block->entity_count++] = low_entity_index;
    }
  }
}

static void changeEntityLocation(Memory_Arena *arena, World *world,
                                 u32 low_entity_index, Low_Entity *low_entity,
                                 World_Position *old_p, World_Position *new_p) {

  changeEntityLocationRaw(arena, world, low_entity_index, old_p, new_p);
  if (new_p) {
    low_entity->p = *new_p;
  } else {
    low_entity->p = nullPosition();
  }
}
