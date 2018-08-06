/*
 * connect 4
 *
 * TODO:
 * - bitboards
 */

#include <stdio.h>

#define rowsize 6
#define colsize 7

typedef struct {
    int rows;
    int cols;
    int ply;
    int last_move_col;
    int last_move_row;
    char grid[rowsize][colsize];
} connect4;

/**
 * initializes a connect 4 game
 *
 * @param connect4 *c4 a pointer to the game
 */
void c4init(connect4 *c4) {
    int i, j;
    c4->ply = 0;
    c4->rows = rowsize;
    c4->cols = colsize;

    for (i = 0; i < c4->rows; i++) {
        for (j = 0; j < c4->cols; j++) {
           c4->grid[i][j] = '.';
        }
    }
}

/**
 * copies a connect 4 position
 *
 * @param connect4 *c4 a pointer to the game
 * @return the copied game
 */
connect4 c4cpy(const connect4 *c4) {
    int i, j;
    connect4 cpy;
    cpy.rows = c4->rows;
    cpy.cols = c4->cols;
    cpy.ply = c4->ply;
    cpy.last_move_row = c4->last_move_row;
    cpy.last_move_col = c4->last_move_col;

    for (i = 0; i < cpy.rows; i++) {
        for (j = 0; j < cpy.cols; j++) {
            cpy.grid[i][j] = c4->grid[i][j];
        }
    }

    return cpy;
}

/**
 * prints a connect 4 board
 *
 * @param connect4 *c4 a pointer to the game
 */
void c4print(const connect4 *c4) {
    int i, j;
    puts("");

    for (i = 0; i < c4->rows; i++) {
        for (j = 0; j < c4->cols; j++) {
            printf(" %c ", c4->grid[i][j]);
        }

        puts("");
    }

    for (i = 0; i < c4->cols; i++) {
        printf("---", i);
    }

    puts("");

    for (i = 1; i <= c4->cols; i++) {
        printf(" %d ", i);
    }

    puts("\n");
}

/**
 * drop the current player's piece into a board
 *
 * @param connect4 *c4 a pointer to the game
 * @param int move_col the column to drop a piece into
 * @return int 1 if successful, 0 otherwise
 */
int c4move(connect4 *c4, int move_col) {
    int i;

    for (i = c4->rows - 1; i >= 0; i--) {
        if (c4->grid[i][move_col] == '.') {
            c4->grid[i][move_col] = c4->ply++ & 1 ? 'o' : 'x';
            c4->last_move_col = move_col;
            c4->last_move_row = i;
            return 1;
        }
    }
    
    return 0;
}

/**
 * drop pieces on a board randomly until the game reaches a terminal state
 *
 * @param connect4 *c4 a pointer to the game
 */
void c4rand_game(connect4 *c4) {
    while (!c4just_won(c4) && !c4full(c4)) {
        c4move(c4, rand() % c4->cols);
    }
}

/**
 * runs a simulation from current position to determine the best move
 *
 * @param connect4 *c4 a pointer to the game
 * @param simulations the number of simulations to attempt per move
 * @return the column of the best move or -1 if none found
 */
int c4simulate(connect4 *c4, int simulations) {
    int i, j; 
    double ratio;
    double best_ratio = 0;
    int best_move = -1;

    for (i = 0; i < c4->cols; i++) {
        int wins = 0;
        int losses = 0;

        for (j = 0; j < simulations; j++) {
            connect4 cpy = c4cpy(c4);
            c4move(&cpy, i);
            c4rand_game(&cpy);

            if (c4just_won(&cpy)) {
                (c4->ply & 1) == (cpy.ply & 1) ? losses++ : wins++;
            }
        }

        ratio = losses > 0 ? (double)wins / losses : wins;
            
        if (ratio > best_ratio) {
            best_ratio = ratio;
            best_move = i;
        }
    }

    return best_move;
}

/**
 * get a integer with bits set for available moves
 *
 * @param connect4 *c4 a pointer to the game
 * @return integer bitset of available moves
 */
int c4moves(const connect4 *c4) {
    int i;
    int moves = 0;

    for (i = 0; i < c4->cols; i++) {
        if (c4->grid[0][i] == '.') {
            moves |= 1 << i;
        }
    }

    return moves;
}

/**
 * negamax with alpha beta pruning--too slow for c4 as-is
 *
 * @param connect4 *c4 a pointer to the game
 * @return the column of the best move
 */
int c4negamax(const connect4 *c4) {
    int best_move;
    _c4nega(c4, 0, -2, 2, &best_move);
    return best_move;
}

// recursive negamax algorithm
int _c4nega(const connect4 *c4, int depth, int a, int b, int *best_move) {
    if (c4just_won(&c4)) { return -1; }
    if (c4full(&c4)) { return 0; }

    int i;
    int best_val = -2;
    int moves = c4moves(c4);

    for (i = 0; i < c4->cols; i++) {
        if (moves >> i & 1) {
            connect4 cpy = c4cpy(c4);
            c4move(&cpy, i);
            int child_val = -_c4nega(&cpy, depth + 1, -b, -a, best_move);

            if (child_val > a) { a = child_val; }

            if (best_val < child_val) {
                best_val = child_val;

                if (depth == 0) { *best_move = i; }
            }
        }
        
        if (a >= b) { break; }
    }

    return best_val;
}

/**
 * determine if last move won the game 
 *
 * @param connect4 *c4 a pointer to the game
 * @return 1 if won 0 if not
 */
int c4just_won(const connect4 *c4) {
    return c4won_at(c4, 1, 0) ||
           c4won_at(c4, 1, 1) ||
           c4won_at(c4, 0, 1) ||
           c4won_at(c4, -1, 1);
}

/**
 * check for 4-in-a-row from the last move in a single direction
 *
 * @param connect4 *c4 a pointer to the game
 * @param int dx the increment on the x-axis in the range -1 to 1
 * @param int dx the increment on the y-axis in the range -1 to 1
 * @return 1 if won 0 if not
 */
int c4won_at(const connect4 *c4, int dx, int dy) {
    char side = c4->ply & 1 ? 'x' : 'o';
    int x = c4->last_move_col - dx;
    int y = c4->last_move_row - dy;
    int count = 0;
    
    // move backward from last move
    while (y >= 0 && x >= 0 && y < c4->rows && 
           x < c4->cols && c4->grid[y][x] == side) {
        count++;
        x -= dx;
        y -= dy;
    }

    x = c4->last_move_col;
    y = c4->last_move_row;

    // move forward from last move
    while (y >= 0 && x >= 0 && y < c4->rows && 
           x < c4->cols && c4->grid[y][x] == side) {
        count++;
        x += dx;
        y += dy;
    }
    
    return count >= 4;
}

/** 
 * checks for draws assuming prior win check; 
 * if board is not won and is full, it's drawn
 *
 * @param connect4 *c4 a pointer to the game
 * @return 1 if full, 0 if not
 */
int c4full(const connect4 *c4) {
    return c4->ply >= c4->rows * c4->cols;
}

int main() {
    char side;
    int move, j;
    int i = 0;
    const int simulations = 50; // difficulty setting
    srand(time(NULL));
    connect4 c4;
    c4init(&c4);

    printf("Choose 'x' or 'o': ");
    scanf("%c", &side);

    if (side == 'x') { i++; }

    for (;; i++) {
        if (i & 1) { // human turn
            c4print(&c4);
            printf("Enter column number ('q' to quit): ");

            if (!scanf("%d", &move)) { break; }
            else { c4move(&c4, move - 1); }
        }
        else if (!c4move(&c4, c4simulate(&c4, simulations))) { // ai turn
            while (!c4move(&c4, rand() % c4.cols));
        }

        if (c4just_won(&c4)) {
            c4print(&c4);
            printf("%c wins!\n", c4.ply & 1 ? 'x' : 'o');
            break;
        }
        else if (c4full(&c4)) {
            c4print(&c4);
            printf("draw!\n");
            break;
        }
    }
    
    return 0;
}
