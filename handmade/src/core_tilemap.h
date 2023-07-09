#if !defined(CORE_TILE_H)

struct Tile_Chunk {
    u32* tiles;
};

struct Tile_Map {
    u32 chunk_shift;
    u32 chunk_mask;
    u32 chunk_dim;

    real32 tile_side_in_meters;
    real32 tile_side_in_pixels;
    real32 meters_to_pixels;

    i32 tile_chunk_count_x;
    i32 tile_chunk_count_y;

    Tile_Chunk* tile_chunks;
};

struct Tile_Chunk_Position {
    u32 tile_chunk_x;
    u32 tile_chunk_y;

    u32 rel_tile_x;
    u32 rel_tile_y;
};

struct Tile_Map_Position {
    // These are fixed point tile locations.
    // High bits are tile chunk index, and the low bits are
    // the tile index in the chunk.
    u32 abs_tile_x;
    u32 abs_tile_y;
    u32 abs_tize_z;

    real32 offset_x;
    real32 offset_y;
};

//struct Tile_Map_Position {
//    // NOTE: These are fixed point tile locations. The hibh bits
//    // are the tile chunk index, and the low bits are the tile
//    // index in the chucnk.
//    u32 abs_tile_x;
//    u32 abs_tile_y;
//    u32 abs_tile_z;
//
//    // These are offset from the tile center.
//    real32 offset_x;
//    real32 offset_y;
//};

#define CORE_TILE_H
#endif