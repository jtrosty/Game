
#if !defined(ERROR_REMOVER)
#include "core.h"
#endif

#if !defined(CORE_WORLD_H)

struct World_Entity_Block {
  u32 entity_count;
  u32 low_entity_index[16];
  World_Entity_Block *next;
};

struct World_Difference {
  v2 dXY;
  real32 dz;
};

struct World_Chunk {
  i32 chunk_x;
  i32 chunk_y;
  i32 chunk_z;

  World_Entity_Block first_block;

  World_Chunk *next_in_hash;
};

struct World_Position {
  // These are fixed point tile locations.
  // High bits are tile chunk index, and the low bits are
  // the tile index in the chunk.
  i32 chunk_x;
  i32 chunk_y;
  i32 chunk_z;

  v2 offset;
};

struct World {
  i32 tile_side_in_pixels;
  real32 tile_side_in_meters;
  real32 chunk_side_in_meters;
  ;

  World_Entity_Block *first_free;
  /*
  u32 chunk_shift;
  u32 chunk_mask;
  u32 chunk_dim;
  */

  World_Chunk chunk_hash[4096];
};

// struct Tile_Map_Position {
//     // NOTE: These are fixed point tile locations. The hibh bits
//     // are the tile chunk index, and the low bits are the tile
//     // index in the chucnk.
//     u32 abs_tile_x;
//     u32 abs_tile_y;
//     u32 abs_tile_z;
//
//     // These are offset from the tile center.
//     real32 offset_x;
//     real32 offset_y;
// };

#define CORE_WORLD_H
#endif
