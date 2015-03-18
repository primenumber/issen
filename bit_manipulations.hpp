#include "board.hpp"

namespace bit_manipulations {

void init();

board flipVertical(const board);
board mirrorHorizontal(board);
board flipDiagA1H8(const board);
board flipDiagA8H1(const board);

board rotate180(const board);
board rotate90clockwise(const board);
board rotate90antiClockwise(const board);

board pseudoRotate45clockwise(const board);
board pseudoRotate45antiClockwise(const board);

} // namespace bit_manipulations
