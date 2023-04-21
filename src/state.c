#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

static void find_tail(game_state_t* state, unsigned snum);
static bool match_head(game_state_t* state, unsigned row, unsigned col, unsigned snum);

/* Task 1 */
game_state_t* create_default_state() {
    
    game_state_t *default_state = malloc(sizeof(game_state_t));
    
    /* Set row number */
    default_state->num_rows = 18;

    /* Init the board */
    default_state->board = malloc(sizeof(char *) * default_state->num_rows);
    for (int i = 0; i < default_state->num_rows; i++) {
        *(default_state->board + i) = malloc(sizeof(char) * 21); // an extra bit for '\0'
        default_state->board[i][20] = '\0';
    }

    /* Set up game board */
    for (int i = 0; i < 20; i++) {
        default_state->board[0][i] = '#';
        default_state->board[17][i] = '#';
    }
    
    for (int i = 0; i < 20; i++) {
        if (i == 0 || i == 19) {
            default_state->board[1][i] = '#';
            continue;
        }
        default_state->board[1][i] = ' ';
    }

    for (int i = 2; i < default_state->num_rows - 1; i++) {
        strcpy(default_state->board[i], default_state->board[1]);
    }

    /* Hardcode snack and friut */
    default_state->board[2][2] = 'd';
    default_state->board[2][3] = '>';
    default_state->board[2][4] = 'D';

    default_state->board[2][9] = '*';

    /* Set up snake */
    default_state->num_snakes = 1;
    default_state->snakes = malloc(sizeof(snake_t) * default_state->num_snakes);
    default_state->snakes->tail_row = 2;
    default_state->snakes->tail_col = 2;
    default_state->snakes->head_row = 2;
    default_state->snakes->head_col = 4;
    default_state->snakes->live = 1;

    return default_state;
}

/* Task 2 */
void free_state(game_state_t* state) {
    free(state->snakes);
    for (int i = 0; i < state->num_rows; i++) {
        free(state->board[i]);
    }
    free(state->board);
    free(state);
    return;
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
    for (int i = 0; i < state->num_rows; i++) {
        fprintf(fp, "%s\n", state->board[i]);
    }
    return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
    return (c == 'w' || c == 's' || c == 'a' || c == 'd');
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
    return c == 'W' || c == 'S' || c == 'A' || c == 'D' || c == 'x';
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
    return (c == 'W' || c == 'S' || c == 'A' || c == 'D'
             || c == 'x' || c == '^' || c == '>' || c == '<'
             || c == 'v' || c == 'w' || c == 'a' || c == 's' || c == 'd');
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
    switch (c) {
        case '^':
            c = 'w';
            break;
        case '>':
            c = 'd';
            break;
        case '<':
            c = 'a';
            break;
        case 'v':
            c = 's';
            break;
    }
    return c;
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
    switch (c) {
        case 'W':
            c = '^';
            break;
        case 'D':
            c = '>';
            break;
        case 'A':
            c = '<';
            break;
        case 'S':
            c = 'v';
            break;
    }
    return c;
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
    switch (c) {
        case 'v':
        case 's':
        case 'S':
            return cur_row + 1; // move down one row
        case '^':
        case 'w':
        case 'W':
            return cur_row - 1; // move up one row
        default:
            return cur_row; // do not move row
     }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
    switch (c) {
        case '>':
        case 'd':
        case 'D':
            return cur_col + 1; // move right one column
        case '<':
        case 'a':
        case 'A':
            return cur_col - 1; // move left one column
        default:
            return cur_col; // do not move column
     }
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
    unsigned int head_row = state->snakes[snum].head_row;
    unsigned int head_col = state->snakes[snum].head_col;
    char head_state = get_board_at(state, head_row, head_col);

    unsigned int next_row = get_next_row(head_row, head_state);
    unsigned int next_col = get_next_col(head_col, head_state);

    return get_board_at(state, next_row, next_col);
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
    unsigned int head_row = state->snakes[snum].head_row;
    unsigned int head_col = state->snakes[snum].head_col;
    char head_state = get_board_at(state, head_row, head_col);

    unsigned int next_row = get_next_row(head_row, head_state);
    unsigned int next_col = get_next_col(head_col, head_state);

    /* Update the snake's head */
    state->snakes[snum].head_row = next_row;
    state->snakes[snum].head_col = next_col;

    /* Update the board */
    set_board_at(state, head_row, head_col, head_to_body(head_state));
    set_board_at(state, next_row, next_col, head_state);

    return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
    unsigned int tail_row = state->snakes[snum].tail_row;
    unsigned int tail_col = state->snakes[snum].tail_col;
    char tail_state = get_board_at(state, tail_row, tail_col);

    unsigned int next_row = get_next_row(tail_row, tail_state);
    unsigned int next_col = get_next_col(tail_col, tail_state);
    char next_tail_state = get_board_at(state, next_row, next_col);

    /* Update the snake's tail */
    state->snakes[snum].tail_row = next_row;
    state->snakes[snum].tail_col = next_col;

    /* Update the board */
    set_board_at(state, next_row, next_col, body_to_tail(next_tail_state));
    set_board_at(state, tail_row, tail_col, ' ');

    return;
}

/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
    for (unsigned int i = 0; i < state->num_snakes; i++) {
        /* If the snake died, stay still */
        if (state->snakes[i].live == false) {
            continue;
        }
        unsigned int head_row = state->snakes[i].head_row;
        unsigned int head_col = state->snakes[i].head_col;

        /* Check if the snake kills itself in the next step */
        char ns = next_square(state, i);
        if (ns == '#' || is_snake(ns)) {
            state->snakes[i].live = false;
            set_board_at(state, head_row, head_col, 'x');
        } else if (ns == '*') {   /* Check if the snake eats the fruit */
            update_head(state, i);
            add_food(state);
        } else {    /* Nothing happened, the snake goes on */
            update_head(state, i);
            update_tail(state, i);
        }
    }
    return;
}

/* Task 5 */
game_state_t* load_board(char* filename) {

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("\nError, file \"%s\" cannot be opened!\n",filename);
        return NULL;
    }

    unsigned int count_row = 0;
    char the_board;
    while ((the_board = fgetc(fp)) != EOF) {
        if (the_board == '\n') {
            count_row++;
        }
    }
    game_state_t* state = malloc(sizeof(game_state_t));
    state->board = malloc(sizeof(char*) * count_row);
    state->num_rows = count_row;

    /* Reset file pointer to beginning of file */
    fseek(fp, 0, SEEK_SET);

    /* allocate sizes of each line */
    unsigned int str_len = 0;
    int row = 0;
    while ((the_board = fgetc(fp)) != EOF) {
        if (the_board == '\n') {
            state->board[row] = malloc(sizeof(char) * (str_len + 1));
            str_len = 0;
            row++;
        } else {
            str_len++;
        }
    }

    /* Reset file pointer to beginning of file */
    fseek(fp, 0, SEEK_SET);
    
    for (int i = 0; i < count_row; i++) {
        for (int j = 0; ; j++) {
            if ((the_board = fgetc(fp)) == '\n') {
                state->board[i][j] = '\0';
                break;
            }
            state->board[i][j] = the_board;
        }
    }

//  A better version but test unfriendly:

//    char buffer[2000];
//    unsigned int count_row = 0;

//    while (fgets(buffer, 2000, fp)) {
//        count_row++;
        // printf("\nBuffer is: %s.\n", buffer);
        // printf("Strlen is: %d.\n", strlen(buffer));
//    }

    /* Reset file pointer to beginning of file */
//    fseek(fp, 0, SEEK_SET);

//   game_state_t* state = malloc(sizeof(game_state_t));
//    state->board = malloc(sizeof(char*) * count_row);
//    state->num_rows = count_row;
//    for (int i = 0; i < count_row; i++) {
//        fgets(buffer, 2000, fp);
//        state->board[i] = malloc(sizeof(char) * strlen(buffer));
//        strcpy(state->board[i], buffer);
        /* Substitute the '\n' to '\0' at the end of string */
//        state->board[i][strlen(buffer) - 1] = '\0';
//    }

    // printf("\nRow number is: %u.\n", state->num_rows);

    state->snakes = NULL;
    state->num_snakes = 0;

    fclose(fp);
    return state;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
    unsigned int which_head = 0;
    /* Regardless the first and the last row */
    for (unsigned i = 1; i < state->num_rows - 1; i++) {
        for (unsigned j = 1; state->board[i][j] != '\0'; j++) {
            if (is_head(state->board[i][j])) {
                if (which_head == snum) {
                   state->snakes[snum].head_row = i;
                   state->snakes[snum].head_col = j;
                   return;
                }
                which_head++;
            }
        }
    }
    return;
}

/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
    unsigned int total_snake = 0;
    /* Regardless the first and the last row */
    for (int i = 1; i < state->num_rows - 1; i++) {
        for (int j = 1; state->board[i][j] != '\0'; j++) {
            if (is_head(state->board[i][j])) {
                total_snake++;
            }
        }
    }
    state->num_snakes = total_snake;
    state->snakes = malloc(sizeof(snake_t) * total_snake);
    for (unsigned i = 0; i < total_snake; i++) {
        find_head(state, i);
        state->snakes[i].live = true;
        find_tail(state, i);
    }
    return state;
}

/*
  Find and set the corresponding tail of given head
*/
static void find_tail(game_state_t* state, unsigned snum) {
    /* Regardless the first and the last row */
    for (unsigned i = 1; i < state->num_rows - 1; i++) {
        for (unsigned j = 1; state->board[i][j] != '\0'; j++) {
            /* If find a tail */
            if (is_tail(state->board[i][j])) {
                /* Check if the tail matches the given head */
                if (match_head(state, i, j, snum)) {
                    state->snakes[snum].tail_row = i;
                    state->snakes[snum].tail_col = j;
                    return;
                }
            }
        }
    }
}

/* Find the head of given tail */
static bool match_head(game_state_t* state, unsigned row, unsigned col, unsigned snum) {
    char head_board = state->board[row][col];
    while (!is_head(head_board)) {
        switch (head_board) {
            case 'a':
            case '<':
            case 'd':
            case '>':
                col = get_next_col(col, head_board);
                break;
            case 's':
            case 'v':
            case 'w':
            case '^':
                row = get_next_row(row, head_board);
                break;
        }

        head_board = get_board_at(state, row, col);
    }
    /* If it is the desired head */
    return state->snakes[snum].head_row == row && state->snakes[snum].head_col == col;
}

