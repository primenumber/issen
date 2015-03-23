#pragma once
#include "board.hpp"

namespace bit_manipulations {

void init();

board flipVertical(board);
uint64_t flipVertical(uint64_t);
board mirrorHorizontal(board);
uint64_t mirrorHorizontal(uint64_t);
board flipDiagA1H8(board);
uint64_t flipDiagA1H8(uint64_t);
board flipDiagA8H1(board);
uint64_t flipDiagA8H1(uint64_t);

board rotate180(board);
uint64_t rotate180(uint64_t);
board rotate90clockwise(board);
uint64_t rotate90clockwise(uint64_t);
board rotate90antiClockwise(board);
uint64_t rotate90antiClockwise(uint64_t);

board pseudoRotate45clockwise(board);
uint64_t pseudoRotate45clockwise(uint64_t);
board pseudoRotate45antiClockwise(board);
uint64_t pseudoRotate45antiClockwise(uint64_t);

int bit_to_pos(uint64_t);

uint16_t toBase3(uint8_t, uint8_t);

} // namespace bit_manipulations
