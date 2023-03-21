#include <stdint.h>
#include <cstring>
#include <cassert>


struct JT_Arena {
    unsigned char* buffer;
    size_t bufferSize;
    size_t currentOffset;
    size_t previousOffset;
};

bool JT_isPowerOfTwo(uintptr_t size) {
    return ((size & 0x00000001) == 0);
}

void JT_arenaInitialize(JT_Arena* arena, void* memoryBuffer, size_t memSize) {
    arena->buffer = (unsigned char*)memoryBuffer;
    arena->bufferSize = memSize;
    arena->currentOffset = 0;
    arena->previousOffset = 0;
}

uintptr_t JT_alignForward(uintptr_t ptr, size_t align) {
    uintptr_t p;
    uintptr_t a;
    uintptr_t modulo;
    assert(JT_isPowerOfTwo(align));
    p = ptr;
    a = (uintptr_t)align;
    modulo = (p % a);
    if ( modulo != 0) {
        p += a - modulo;
    }
    return p;
}

void* JT_linearArenaAlloc(JT_Arena* arena, size_t size) {
    void* result = NULL;
    // Check space and allocate
    if (arena->bufferSize > (arena->currentOffset + size)) {
        result = &arena->buffer[arena->currentOffset];
        arena->previousOffset += arena->currentOffset;
        arena->currentOffset += size;
        memset(result, 0, size);
    }
    // TODO: Log? 
    return result;
}

void* JT_linearArenaAllocAlign(JT_Arena* arena, size_t size, size_t align) {
    void* result = NULL;
    // Align the offest
    uintptr_t ptr = (uintptr_t)arena->buffer + (uintptr_t)arena->currentOffset;
    uintptr_t offset = JT_alignForward(ptr, align);
    // Return offset to relative offset
    offset -= (uintptr_t)arena->buffer;

    // Check space and allocate
    if (arena->bufferSize >= offset + size) {
        result = &arena->buffer[offset];
        arena->previousOffset += offset;
        arena->currentOffset += size + offset;
        memset(result, 0, size);
    }
    // TODO: Log? 
    return result;
}

void* JT_linearArenaResizeAlign(JT_Arena* arena, void* oldMemory, size_t oldSize, size_t newSize, size_t align) {
    unsigned char* oldMem = (unsigned char*)oldMemory;
    assert(JT_isPowerOfTwo(align));
    void* result = NULL;

    if(oldMem == NULL || oldSize == 0) {
        result = JT_linearArenaAllocAlign(arena, newSize, align);
    }
    else if ((arena->buffer >= oldMem) && (oldMem < (arena->buffer + arena->bufferSize))) {
        if (arena->buffer + arena->previousOffset == oldMem) {
            arena->currentOffset = arena->previousOffset + newSize;
            if (newSize > oldSize) {
                memset(&arena->buffer[arena->currentOffset], 0, newSize - oldSize);
            }
            result = oldMemory;
        } 
        else {
            void* newMemory = JT_linearArenaAllocAlign(arena, newSize, align);
            size_t copySize = oldSize < newSize ? oldSize : newSize;
            memmove(newMemory, oldMemory, copySize);
            result = newMemory;
        }
    }
    else { 
        assert( 0 && "Memory is out of bonds of the buffer in the arena.");
    }
    return result;
}

void JT_arenaFree(JT_Arena* arena, void* ptr) {
    // Do nothing
}

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void*));
#endif

void* JT_arenaResize(JT_Arena* a, void* oldMemory, size_t oldSize, size_t newSize) {
    return JT_linearArenaResizeAlign(a, oldMemory, newSize, DEFAULT_ALIGNMENT);
}

void* JT_arenaFreeAll(JT_Arena* a) {
    a->currentOffset = 0;
    a->previousOffset = 0;
}