#include "board.hpp"

namespace bit_manipulations {

void init();

board flipVertical(board);
board mirrorHorizontal(board);
board flipDiagA1H8(board);
board flipDiagA8H1(board);

board rotate180(board);
board rotate90clockwise(board);
board rotate90antiClockwise(board);

board pseudoRotate45clockwise(board);
board pseudoRotate45antiClockwise(board);

int bit_to_pos(uint64_t);

uint16_t toBase3(uint8_t, uint8_t);

} // namespace bit_manipulations
