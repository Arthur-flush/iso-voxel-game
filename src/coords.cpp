#include <coords.hpp>

bool block_coordonate::operator<(const block_coordonate& other) const{
    if (chunk.x < other.chunk.x) return true;
    else if (chunk.x > other.chunk.x) return false;
    else if (chunk.y < other.chunk.y) return true;
    else if (chunk.y > other.chunk.y) return false;
    else if (chunk.z < other.chunk.z) return true;
    else if (chunk.z > other.chunk.z) return false;
    else if (x < other.x) return true;
    else if (x > other.x) return false;
    else if (y < other.y) return true;
    else if (y > other.y) return false;
    else if (z < other.z) return true;
    else if (z > other.z) return false;
    else return false;
    

    // cursed je sais 
}

bool block_coordonate::operator==(const block_coordonate& other) const {
    return x == other.x && y == other.y && z == other.z && chunk.x == other.chunk.x && chunk.y == other.chunk.y && chunk.z == other.chunk.z;
}

block_coordonate block_coordonate::operator+(const coord3D& other) const {
    block_coordonate result;

    result.x = x + other.x;
    result.y = y + other.y;
    result.z = z + other.z;

    result.chunk.x = chunk.x;
    result.chunk.y = chunk.y;
    result.chunk.z = chunk.z;

    if (result.x < 0) {
        result.x = CHUNK_SIZE - 1;
        result.chunk.x--;
    }
    else if (result.x >= CHUNK_SIZE) {
        result.x = 0;
        result.chunk.x++;
    }

    if (result.y < 0) {
        result.y = CHUNK_SIZE - 1;
        result.chunk.y--;
    }
    else if (result.y >= CHUNK_SIZE) {
        result.y = 0;
        result.chunk.y++;
    }

    if (result.z < 0) {
        result.z = CHUNK_SIZE - 1;
        result.chunk.z--;
    }
    else if (result.z >= CHUNK_SIZE) {
        result.z = 0;
        result.chunk.z++;
    }

    return result;
}