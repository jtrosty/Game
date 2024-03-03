// #pragma once
// #include "root.unity.h"

inline void recanonicalizeCoordinate(Tile_Map *tile_map, u32 *tile,
                                     real32 *tile_rel) {
  // Assume tile_map is terrodial.
  i32 offset = roundReal32ToInt32(
      *tile_rel / tile_map->tile_side_in_meters); // this will be 1 or -1

  // NOTE(Jon): Worl dis assumed to be toroidal topology. If you step off one
  // end you come back ont he other.
  *tile_rel -= offset * tile_map->tile_side_in_meters;
  // TODO: Verify this addition happens correctly between u32 and i32
  *tile += offset;
  Assert(*tile_rel > -0.5001f * tile_map->tile_side_in_meters);
  Assert(*tile_rel < 0.5001f * tile_map->tile_side_in_meters);
}

inline Tile_Map_Position recanonicalizePosition(Tile_Map *tile_map,
                                                Tile_Map_Position pos) {
  Tile_Map_Position result = pos;

  recanonicalizeCoordinate(tile_map, &result.abs_tile_x, &result.offset.x);
  recanonicalizeCoordinate(tile_map, &result.abs_tile_y, &result.offset.y);

  return result;
}

//
//
// The GETS
//
//
//
inline Tile_Chunk_Position getChunkPositionFor(Tile_Map *tile_map,
                                               u32 abs_tile_x, u32 abs_tile_y,
                                               u32 abs_tile_z) {

  Tile_Chunk_Position result;

  result.tile_chunk_x = abs_tile_x >> tile_map->chunk_shift;
  result.tile_chunk_y = abs_tile_y >> tile_map->chunk_shift;
  result.tile_chunk_z = abs_tile_z;
  result.rel_tile_x = abs_tile_x & tile_map->chunk_mask;
  result.rel_tile_y = abs_tile_y & tile_map->chunk_mask;

  return result;
}

inline u32 getTileValueUnchecked(Tile_Map *tile_map, Tile_Chunk *tile_chunk,
                                 u32 tile_x, u32 tile_y) {
  // this unflips the tile_map
  Assert(tile_chunk);
  Assert(tile_x < tile_map->chunk_dim);
  Assert(tile_y < tile_map->chunk_dim);

  u32 tile_map_value = tile_chunk->tiles[tile_y * tile_map->chunk_dim + tile_x];
  return tile_map_value;
}

inline u32 getTileValue(Tile_Map *tile_map, Tile_Chunk *tile_chunk,
                        u32 test_tile_x, u32 test_tile_y, u32 test_tile_z) {
  u32 result = 0;
  // This funciton is guarded by checking if tile_map is zero or not.
  if (tile_chunk) {
    result =
        getTileValueUnchecked(tile_map, tile_chunk, test_tile_x, test_tile_y);
  }
  return result;
}

static u32 getTileValue(Tile_Map *tile_map, u32 abs_tile_x, u32 abs_tile_y,
                        u32 abs_tile_z) {
  // bool32 empty = false;

  Tile_Chunk_Position chunk_pos =
      getChunkPositionFor(tile_map, abs_tile_x, abs_tile_y, abs_tile_z);
  Tile_Chunk *tile_chunk =
      getTileChunk(tile_map, chunk_pos.tile_chunk_x, chunk_pos.tile_chunk_y,
                   chunk_pos.tile_chunk_z);
  u32 tile_chunk_value =
      getTileValue(tile_map, tile_chunk, chunk_pos.rel_tile_x,
                   chunk_pos.rel_tile_y, abs_tile_z);

  return tile_chunk_value;
}

inline u32 getTileValue(Tile_Map *tile_map, Tile_Map_Position test_pos) {
  u32 tile_chunk_value = getTileValue(tile_map, test_pos.abs_tile_x,
                                      test_pos.abs_tile_y, test_pos.abs_tile_z);

  return tile_chunk_value;
}

//
//
// The SETS
//
//

inline void setTileValueUnchecked(Tile_Map *tile_map, Tile_Chunk *tile_chunk,
                                  u32 tile_x, u32 tile_y, u32 tile_value) {
  // this unflips the tile_map

  Assert(tile_chunk);
  Assert(tile_x < tile_map->chunk_dim);
  Assert(tile_y < tile_map->chunk_dim);

  tile_chunk->tiles[tile_y * tile_map->chunk_dim + tile_x] = tile_value;
}

inline void setTileValue(Tile_Map *tile_map, Tile_Chunk *tile_chunk,
                         u32 test_tile_x, u32 test_tile_y, u32 tile_value) {
  u32 result = 0;
  // This funciton is guarded by checking if tile_map is zero or not.
  if (tile_chunk) {
    setTileValueUnchecked(tile_map, tile_chunk, test_tile_x, test_tile_y,
                          tile_value);
  }
}

static void setTileValue(Memory_Arena *arena, Tile_Map *tile_map,
                         u32 abs_tile_x, u32 abs_tile_y, u32 abs_tile_z,
                         u32 tile_value) {
  Tile_Chunk_Position chunk_pos =
      getChunkPositionFor(tile_map, abs_tile_x, abs_tile_y, abs_tile_z);
  Tile_Chunk *tile_chunk =
      getTileChunk(tile_map, chunk_pos.tile_chunk_x, chunk_pos.tile_chunk_y,
                   chunk_pos.tile_chunk_z);
  // TODO: On demand tile chunk creation
  Assert(tile_chunk);
  if (tile_chunk) {
    setTileValue(tile_map, tile_chunk, chunk_pos.rel_tile_x,
                 chunk_pos.rel_tile_y, tile_value);
  }
}

//
//
// The IS ARE section
//
//

static bool32 isTileValueEmpty(u32 tile_value) {
  // if not one of the specified values it is empty.
  bool32 result = ((tile_value == 1) || (tile_value == 3) || (tile_value == 4));
  return result;
}

static bool32 areOnSameTile(Tile_Map_Position *a, Tile_Map_Position *b) {
  bool32 result = (a->abs_tile_x == b->abs_tile_x) &&
                  (a->abs_tile_y == b->abs_tile_y) &&
                  (a->abs_tile_z == b->abs_tile_z);
  return result;
}

inline bool32 isTileChunkTilePointEmpty(Tile_Map *tile_map,
                                        Tile_Chunk *tile_chunk, u32 test_tile_x,
                                        u32 test_tile_y) {
  bool32 result = false;
  // This funciton is guarded by checking if tile_map is zero or not.
  if (tile_map) {
    u32 tile_map_value =
        getTileValueUnchecked(tile_map, tile_chunk, test_tile_x, test_tile_y);
    result = (tile_map_value == 1);
  }
  return result;
}

static bool32 isTileMapPointEmpty(Tile_Map *tile_map,
                                  Tile_Map_Position *test_pos) {
  bool32 empty = false;

  u32 tile_chunk_value = getTileValue(tile_map, *test_pos);
  empty = (tile_chunk_value == 1);
  return empty;
}

inline Tile_Map_Difference
tileSubtract(Tile_Map *tile_map, Tile_Map_Position *a, Tile_Map_Position *b) {
  Tile_Map_Difference result;
  v2 diff_tile_xy = {(real32)a->abs_tile_x - (real32)b->abs_tile_x,
                     (real32)a->abs_tile_y - (real32)b->abs_tile_y};
  real32 diff_tile_z = (real32)a->abs_tile_z - (real32)b->abs_tile_z;

  result.dXY =
      tile_map->tile_side_in_meters * diff_tile_xy + (a->offset - b->offset);
  result.dz = tile_map->tile_side_in_meters * diff_tile_z;

  return result;
}

inline Tile_Map_Position centeredTilePoint(u32 abs_tile_x, u32 abs_tile_y,
                                           u32 abs_tile_z) {
  Tile_Map_Position result = {};

  result.abs_tile_x = abs_tile_x;
  result.abs_tile_y = abs_tile_y;
  result.abs_tile_z = abs_tile_z;

  return result;
}

inline Tile_Map_Position offsetTilePosition(Tile_Map *tile_map,
                                            Tile_Map_Position p, v2 offset) {
  p.offset += offset;
  p = recanonicalizePosition(tile_map, p);

  return p;
}
