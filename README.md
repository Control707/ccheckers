# Checkers Game in C

Welcome to the Checkers Game project! This README provides an in-depth overview of the implementation, design decisions, and technical details of the Checkers game developed in C. Whether you're a developer looking to understand the codebase or a enthusiast interested in how bitwise operations can optimize game logic, this document has you covered.

## Table of Contents

- [Overview](#overview)
- [Design](#design)
    - [Data Structures](#data-structures)
        - [Board Structure](#board-structure)
        - [Move Structure](#move-structure)
    - [Bitboard Representation](#bitboard-representation)
- [Implementation Details](#implementation-details)
    - [Initialization](#initialization)
        - [`init_board`](#init_board)
    - [Displaying the Board](#displaying-the-board)
        - [`print_board`](#print_board)
    - [Utility Functions](#utility-functions)
        - [`count_bits`](#count_bits)
        - [`is_game_over`](#is_game_over)
        - [`get_winner`](#get_winner)
    - [Move Validation](#move-validation)
        - [`is_valid_single_move`](#is_valid_single_move)
        - [`is_valid_capture`](#is_valid_capture)
    - [Executing Moves](#executing-moves)
        - [`make_move`](#make_move)
        - [`has_captures`](#has_captures)
        - [`get_all_moves`](#get_all_moves)
        - [`make_multi_jump`](#make_multi_jump)
    - [Main Game Loop](#main-game-loop)
        - [`main`](#main)
- [Challenges](#challenges)
- [Bitwise Operations and Binary Arithmetic](#bitwise-operations-and-binary-arithmetic)
- [Diagrams](#diagrams)
- [Variables and Data Types](#variables-and-data-types)
- [References](#references)

---

## Overview

This project implements a command-line version of the classic Checkers game using the C programming language. The game utilizes efficient bitwise operations and binary arithmetic to manage and manipulate the game state, ensuring optimal performance and memory usage. Players can engage in a turn-based match, making moves and captures until a win condition is met.

---

## Design

### Data Structures

#### Board Structure

The game state is encapsulated within a `Board` structure, which utilizes three `uint64_t` variables to represent the positions of white pieces, black pieces, and kings on an 8x8 Checkers board.

```c
typedef struct {
    uint64_t white;
    uint64_t black;
    uint64_t kings;
} Board;
```

- **white**: Bitmask representing the positions of white regular pieces.
- **black**: Bitmask representing the positions of black regular pieces.
- **kings**: Bitmask representing the positions of kings (both white and black).

#### Move Structure

Each move in the game is represented by a `Move` structure, indicating the starting and ending positions of a piece.

```c
typedef struct {
    int from;
    int to;
} Move;
```

- **from**: The starting position index (0-63).
- **to**: The ending position index (0-63).

### Bitboard Representation

The game employs a **bitboard** approach, where the 8x8 Checkers board is represented as a 64-bit integer. Each bit corresponds to a square on the board:

```
Bit Index Mapping:

  0  1  2  3  4  5  6  7
  8  9 10 11 12 13 14 15
 16 17 18 19 20 21 22 23
 24 25 26 27 28 29 30 31
 32 33 34 35 36 37 38 39
 40 41 42 43 44 45 46 47
 48 49 50 51 52 53 54 55
 56 57 58 59 60 61 62 63
```

Using bitboards allows for efficient storage and manipulation of the game state through bitwise operations.

---

## Implementation Details

### Initialization

#### `init_board`

Initializes the game board with the starting positions of white and black pieces.

```c
void init_board(Board *board) {
    board->white = 0b0000000000000000000000000000000000000000010101011010101001010101ULL;
    board->black = 0b1010101001010101101010100000000000000000000000000000000000000000ULL;
    board->kings = 0ULL;  // No kings initially
}
```

- **white**: Binary representation sets bits for white pieces in the first three rows.
- **black**: Binary representation sets bits for black pieces in the last three rows.
- **kings**: Initially set to zero as there are no kings at the start.

### Displaying the Board

#### `print_board`

Displays the current state of the board in a human-readable format.

```c
void print_board(const Board *board) {
    printf("  0 1 2 3 4 5 6 7\n");
    for (int row = 7; row >= 0; row--) {
        printf("%d ", row);
        for (int col = 0; col < 8; col++) {
            uint64_t mask = 1ULL << (row * 8 + col);
            if (board->white & mask) {
                printf(board->kings & mask ? "W " : "w ");
            } else if (board->black & mask) {
                printf(board->kings & mask ? "B " : "b ");
            } else {
                printf(". ");
            }
        }
        printf("\n");
    }
    printf("\n");
}
```

- Iterates through each row and column, using bit masking to determine the presence and type of piece.
- **Output Symbols**:
    - `w`: White regular piece
    - `W`: White king
    - `b`: Black regular piece
    - `B`: Black king
    - `.`: Empty square

### Utility Functions

#### `count_bits`

Counts the number of set bits (1s) in a `uint64_t` number.

```c
int count_bits(uint64_t n) {
    int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}
```

- **Purpose**: Useful for determining the number of pieces a player has left.

#### `is_game_over`

Checks if the game has reached a terminal state.

```c
int is_game_over(const Board *board) {
    // Game is over if either player has no pieces or no legal moves
    return (board->white == 0 || board->black == 0);
}
```

- **Terminal Conditions**:
    - One player has no remaining pieces.
    - (Note: The implementation currently checks only for no pieces; it can be extended to check for no legal moves.)

#### `get_winner`

Determines the winner of the game.

```c
int get_winner(const Board *board) {
    if (board->white == 0) return BLACK;
    if (board->black == 0) return WHITE;
    return -1; // No winner yet
}
```

- **Returns**:
    - `WHITE` if black has no pieces.
    - `BLACK` if white has no pieces.
    - `-1` if the game is still ongoing.

### Move Validation

#### `is_valid_single_move`

Validates a non-capturing move.

```c
int is_valid_single_move(const Board *board, int from, int to, int player) {
    uint64_t from_mask = 1ULL << from;
    uint64_t to_mask = 1ULL << to;
    uint64_t player_pieces = (player == WHITE) ? board->white : board->black;
    uint64_t all_pieces = board->white | board->black;

    // Check if 'from' position has a player's piece and 'to' position is empty
    if (!(player_pieces & from_mask) || (all_pieces & to_mask)) {
        return 0;
    }

    int from_row = from / 8, from_col = from % 8;
    int to_row = to / 8, to_col = to % 8;
    int row_diff = to_row - from_row;
    int col_diff = to_col - from_col;

    // Regular move
    if (abs(row_diff) == 1 && abs(col_diff) == 1) {
        if (player == WHITE && row_diff > 0) return 1;
        if (player == BLACK && row_diff < 0) return 1;
        if (board->kings & from_mask) return 1;
    }

    return 0;
}
```

- **Checks**:
    - The `from` position contains a piece belonging to the player.
    - The `to` position is empty.
    - The move direction is forward for regular pieces unless it's a king.
    - The move is diagonal by one square.

#### `is_valid_capture`

Validates a capturing move.

```c
int is_valid_capture(const Board *board, int from, int to, int player) {
    uint64_t from_mask = 1ULL << from;
    uint64_t to_mask = 1ULL << to;
    uint64_t player_pieces = (player == WHITE) ? board->white : board->black;
    uint64_t opponent_pieces = (player == WHITE) ? board->black : board->white;
    uint64_t all_pieces = board->white | board->black;

    // Check if 'from' position has a player's piece and 'to' position is empty
    if (!(player_pieces & from_mask) || (all_pieces & to_mask)) {
        return 0;
    }

    int from_row = from / 8, from_col = from % 8;
    int to_row = to / 8, to_col = to % 8;
    int row_diff = to_row - from_row;
    int col_diff = to_col - from_col;

    // Capture move
    if (abs(row_diff) == 2 && abs(col_diff) == 2) {
        int captured_row = (from_row + to_row) / 2;
        int captured_col = (from_col + to_col) / 2;
        uint64_t captured_mask = 1ULL << (captured_row * 8 + captured_col);
        if (opponent_pieces & captured_mask) {
            if (player == WHITE && row_diff > 0) return 1;
            if (player == BLACK && row_diff < 0) return 1;
            if (board->kings & from_mask) return 1;
        }
    }

    return 0;
}
```

- **Checks**:
    - The `from` position contains a piece belonging to the player.
    - The `to` position is empty.
    - The move is diagonal by two squares.
    - An opponent's piece exists in the square being jumped over.
    - The move direction is valid (forward for regular pieces unless it's a king).

### Executing Moves

#### `make_move`

Updates the board state by executing a move.

```c
void make_move(Board *board, int from, int to, int player) {
    uint64_t from_mask = 1ULL << from;
    uint64_t to_mask = 1ULL << to;
    uint64_t move_mask = from_mask | to_mask;

    // Update piece positions
    if (player == WHITE) {
        board->white ^= move_mask;
    } else {
        board->black ^= move_mask;
    }

    // Handle capture
    if (abs((to / 8) - (from / 8)) == 2) {
        int captured_pos = (from + to) / 2;
        uint64_t captured_mask = 1ULL << captured_pos;
        board->white &= ~captured_mask;
        board->black &= ~captured_mask;
        board->kings &= ~captured_mask;
    }

    // Update kings
    board->kings &= ~from_mask;
    if ((player == WHITE && to / 8 == 7) || (player == BLACK && to / 8 == 0) || (board->kings & from_mask)) {
        board->kings |= to_mask;
    }
}
```

- **Steps**:
    - Toggles the bits for the `from` and `to` positions.
    - Handles captures by removing the opponent's piece in the captured square.
    - Promotes pieces to kings if they reach the opponent's baseline.

#### `has_captures`

Checks if the player has any available capture moves.

```c
int has_captures(const Board *board, int player) {
    uint64_t player_pieces = (player == WHITE) ? board->white : board->black;

    for (int from = 0; from < 64; from++) {
        if (player_pieces & (1ULL << from)) {
            for (int to = 0; to < 64; to++) {
                if (is_valid_capture(board, from, to, player)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
```

- **Purpose**: Enforces mandatory captures as per Checkers rules.

#### `get_all_moves`

Generates all possible legal moves for the current player.

```c
void get_all_moves(const Board *board, int player, Move *moves, int *num_moves) {
    *num_moves = 0;
    uint64_t player_pieces = (player == WHITE) ? board->white : board->black;
    int has_capture = has_captures(board, player);

    for (int from = 0; from < 64; from++) {
        if (player_pieces & (1ULL << from)) {
            for (int to = 0; to < 64; to++) {
                if (has_capture) {
                    if (is_valid_capture(board, from, to, player)) {
                        moves[*num_moves].from = from;
                        moves[*num_moves].to = to;
                        (*num_moves)++;
                    }
                } else {
                    if (is_valid_single_move(board, from, to, player)) {
                        moves[*num_moves].from = from;
                        moves[*num_moves].to = to;
                        (*num_moves)++;
                    }
                }
            }
        }
    }
}
```

- **Process**:
    - Iterates through all pieces of the player.
    - Checks for available capture moves.
    - If captures are available, only capture moves are considered.
    - If no captures, all single moves are considered.

#### `make_multi_jump`

Handles multiple consecutive captures by the same piece.

```c
void make_multi_jump(Board *board, int player) {
    int from, to;
    Move moves[64];
    int num_moves;

    do {
        get_all_moves(board, player, moves, &num_moves);
        print_board(board);
        printf("Enter next jump (from to), or -1 -1 to end turn: ");
        scanf("%d %d", &from, &to);

        if (from == -1 && to == -1) break;

        int valid_move = 0;
        for (int i = 0; i < num_moves; i++) {
            if (moves[i].from == from && moves[i].to == to) {
                valid_move = 1;
                break;
            }
        }

        if (valid_move && is_valid_capture(board, from, to, player)) {
            make_move(board, from, to, player);
        } else {
            printf("Invalid move. Try again.\n");
        }
    } while (has_captures(board, player));
}
```

- **Functionality**:
    - Continues to prompt the player for additional jumps as long as capture moves are available.
    - Ensures that all mandatory jumps are executed in a single turn.

### Main Game Loop

#### `main`

Handles the overall flow of the game.

```c
int main() {
    Board board;
    init_board(&board);
    int current_player = WHITE;
    Move moves[64];
    int num_moves;

    while (!is_game_over(&board)) {
        print_board(&board);
        printf("%s's turn\n", current_player == WHITE ? "White" : "Black");

        get_all_moves(&board, current_player, moves, &num_moves);

        if (num_moves == 0) {
            printf("%s has no legal moves. Game over!\n", current_player == WHITE ? "White" : "Black");
            break;
        }

        printf("Legal moves:\n");
        for (int i = 0; i < num_moves; i++) {
            printf("%d: %d -> %d\n", i, moves[i].from, moves[i].to);
        }

        int move_index;
        printf("Enter the index of your move: ");
        scanf("%d", &move_index);

        if (move_index >= 0 && move_index < num_moves) {
            make_move(&board, moves[move_index].from, moves[move_index].to, current_player);

            // Check for multi-jump
            if (is_valid_capture(&board, moves[move_index].from, moves[move_index].to, current_player)) {
                make_multi_jump(&board, current_player);
            }

            current_player = !current_player;  // Switch player
        } else {
            printf("Invalid move index. Try again.\n");
        }
    }

    int winner = get_winner(&board);
    if (winner == WHITE) {
        printf("White wins!\n");
    } else if (winner == BLACK) {
        printf("Black wins!\n");
    } else {
        printf("It's a draw!\n");
    }

    return 0;
}
```

- **Flow**:
    1. Initializes the board.
    2. Alternates turns between white and black players.
    3. Displays the board and prompts the current player for a move.
    4. Validates and executes the move.
    5. Checks for multi-jump opportunities.
    6. Determines and announces the winner when the game ends.

---

## Challenges

1. **Bitboard Manipulation**: Ensuring accurate representation and manipulation of game pieces using bitwise operations required careful planning to prevent bugs.
2. **Move Validation**: Implementing comprehensive move validation, especially for multi-jump scenarios, was complex due to the need to handle various game states.
3. **User Input Handling**: Designing a user-friendly command-line interface that accurately interprets and responds to user inputs without crashes was essential.
4. **King Promotion**: Correctly promoting pieces to kings and ensuring that their movement abilities are accurately represented added an extra layer of complexity.

---

## Bitwise Operations and Binary Arithmetic

The application heavily relies on bitwise operations to manage the game state efficiently. Here's how:

- **Bitmasking**: Each piece's position is represented by a specific bit in a `uint64_t` variable. Bitmasking allows for quick querying and updating of piece positions.

  ```c
  uint64_t mask = 1ULL << position;
  if (board->white & mask) { /* White piece present */ }
  ```

- **Setting Bits**: To place a piece on the board, a bit is set using the OR operation.

  ```c
  board->white |= mask;
  ```

- **Clearing Bits**: To remove a piece, a bit is cleared using the AND operation with the complement.

  ```c
  board->white &= ~mask;
  ```

- **Toggling Bits**: Moving a piece involves toggling bits using the XOR operation.

  ```c
  board->white ^= (from_mask | to_mask);
  ```

- **Counting Bits**: The `count_bits` function iterates through bits to count the number of set bits, indicating the number of pieces a player has.

  ```c
  int count_bits(uint64_t n) {
      int count = 0;
      while (n) {
          count += n & 1;
          n >>= 1;
      }
      return count;
  }
  ```

- **Shifting**: Calculations for move directions (e.g., determining captured pieces) use arithmetic based on row and column indices derived from bit positions.

---

## Diagrams

### Bitboard Layout

```
Bit 0  Bit 1  Bit 2  Bit 3  Bit 4  Bit 5  Bit 6  Bit 7
Bit 8  Bit 9  Bit10 Bit11 Bit12 Bit13 Bit14 Bit15
Bit16 Bit17 Bit18 Bit19 Bit20 Bit21 Bit22 Bit23
Bit24 Bit25 Bit26 Bit27 Bit28 Bit29 Bit30 Bit31
Bit32 Bit33 Bit34 Bit35 Bit36 Bit37 Bit38 Bit39
Bit40 Bit41 Bit42 Bit43 Bit44 Bit45 Bit46 Bit47
Bit48 Bit49 Bit50 Bit51 Bit52 Bit53 Bit54 Bit55
Bit56 Bit57 Bit58 Bit59 Bit60 Bit61 Bit62 Bit63
```

- **Rows**: Numbered from 0 (bottom) to 7 (top).
- **Columns**: Numbered from 0 (left) to 7 (right).

### Move Validation Flowchart

```
Start Move Validation
        |
   Is 'from' occupied by player?
        |
       Yes
        |
    Is 'to' empty?
        |
       Yes
        |
   Is move diagonal by 1?
  /          \
Yes            No
 |               |
Check direction   Invalid
 |        
Is player moving forward or is it a king?
  /                  \
Yes                   No
 |                       |
Valid Move            Invalid
```

*Note: This is a simplified representation. The actual implementation includes additional checks for captures and multi-jumps.*

---

## Variables and Data Types

- **`uint64_t`**: Used extensively for bitboards (`white`, `black`, `kings`) to represent the state of the board with high efficiency.
- **`int`**: Utilized for indices, counters, and simple flags.
- **`Board` Structure**: Encapsulates the entire game state, making it easy to pass around and manipulate.
- **`Move` Structure**: Represents individual moves, enhancing code readability and maintainability.

---

## References

- **Checkers Game Rules**: Understanding the standard rules of Checkers was essential for implementing game logic accurately.
- **Bitboard Techniques**: Inspired by chess programming, bitboards offer an efficient way to handle game states.
    - [Bitboards Explained](https://www.chessprogramming.org/Bitboards)
- **C Programming Documentation**: Standard C libraries (`stdio.h`, `stdlib.h`, etc.) were used for input/output operations and memory management.
    - [C Programming Language](https://www.cprogramming.com/)
- **Bitwise Operations in C**: Leveraged for efficient game state manipulation.
    - [Bitwise Operators in C](https://www.geeksforgeeks.org/bitwise-operators-in-c-cpp/)

---
