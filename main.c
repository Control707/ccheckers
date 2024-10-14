#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define WHITE 0
#define BLACK 1

typedef struct {
    uint64_t white;
    uint64_t black;
    uint64_t kings;
} Board;

typedef struct {
    int from;
    int to;
} Move;

void init_board(Board *board) {
    board->white = 0b0000000000000000000000000000000000000000010101011010101001010101ULL;

    board->black = 0b1010101001010101101010100000000000000000000000000000000000000000ULL;

    board->kings = 0ULL;  // No kings initially
}

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


int count_bits(uint64_t n) {
    int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

int is_game_over(const Board *board) {
    // Game is over if either player has no pieces or no legal moves
    return (board->white == 0 || board->black == 0);
}

int get_winner(const Board *board) {
    if (board->white == 0) return BLACK;
    if (board->black == 0) return WHITE;
    return -1; // No winner yet
}

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
