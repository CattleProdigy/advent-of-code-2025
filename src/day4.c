#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct stack {
    size_t capacity;
    size_t size;
    size_t* items;
};

void free_stack(struct stack* s)
{
    if (s) {
        free(s->items);
        free(s);
    }
}


struct stack* new_stack(size_t capacity)
{
    struct stack* s = malloc(sizeof(struct stack));
    s->capacity = capacity;
    s->size = 0;
    s->items = malloc(capacity * sizeof(size_t));
    return s;
}

struct stack* copy_stack(const struct stack* const s)
{
    struct stack* new_s = malloc(sizeof(struct stack));
    new_s->capacity = s->capacity;
    new_s->size = s->size;
    new_s->items = malloc(s->capacity * sizeof(size_t));
    memcpy(new_s->items, s->items, s->size * sizeof(size_t));
    return new_s;
}

bool stack_is_empty(const struct stack* const s)
{
    return s->size == 0;
}

void stack_push(struct stack* s, size_t item)
{
    if (s->size >= s->capacity) {
        while (s->capacity <= s->size) {
            s->capacity *= 2;
        }
        s->items = realloc(s->items, s->capacity * sizeof(size_t));
    }
    s->items[s->size] = item;
    s->size++;
}

size_t stack_pop(struct stack* s)
{
    if (s->size == 0) {
        fprintf(stderr, "Pop from empty stack\n");
        abort();
    }
    s->size--;
    return s->items[s->size];
}

void stack_clear(struct stack* s)
{
    s->size = 0;
}

struct roll_grid {
    size_t width;
    size_t height;
    bool* rolls;
};

void free_roll_grid(struct roll_grid* jb)
{
    if (jb) {
        free(jb->rolls);
        free(jb);
    }
}

struct roll_grid* new_roll_grid(size_t width, size_t height)
{
    struct roll_grid* rg = malloc(sizeof(struct roll_grid));
    rg->width = width;
    rg->height = height;
    rg->rolls = malloc(width * height * sizeof(bool));
    memset(rg->rolls, 0, width * height * sizeof(bool));
    return rg;
}

struct roll_grid* copy_roll_grid(const struct roll_grid* const rg)
{
    struct roll_grid* new_rg = malloc(sizeof(struct roll_grid));
    new_rg->width = rg->width;
    new_rg->height = rg->height;
    new_rg->rolls = malloc(rg->width * rg->height * sizeof(bool));
    memcpy(new_rg->rolls, rg->rolls, rg->width * rg->height * sizeof(bool));
    return new_rg;
}

bool get_roll(const struct roll_grid* const rg, size_t x, size_t y)
{
    if (x >= rg->width || y >= rg->height) {
        fprintf(stderr, "Roll coordinates out of range: (%zu, %zu)\n", x, y);
        abort();
    }
    return rg->rolls[y * rg->width + x];
}

void print_roll_grid(const struct roll_grid* const rg)
{
    for (size_t y = 0; y < rg->height; ++y) {
        bool* row = &rg->rolls[y * rg->width];
        for (size_t x = 0; x < rg->width; ++x) {
            printf("%c", row[x] ? '@' : '.');
        }
        printf("\n");
    }
}

struct roll_grid* parse_roll_grid(FILE* stream)
{
    size_t width = 0;
    size_t height = 0;
    size_t line_length = 0;

    char buffer[256] = { 0 };
    bool* rolls = NULL;
    while (fgets(buffer, sizeof(buffer), stream)) {

        line_length = strnlen(buffer, 256);
        if (buffer[line_length - 1] == '\n') {
            --line_length;
            buffer[line_length] = '\0';
        }
        if (width != 0 && line_length != width) {
            fprintf(stderr, "Inconsistent width: %zu vs %zu\n", width, line_length);
            abort();
        } else {
            width = line_length;
        }

        if (height == 0) {
            rolls = malloc((height + 1) * width * sizeof(bool));
        } else {
            rolls = realloc(rolls, (height + 1) * width * sizeof(bool));
        }

        bool* output_row = &rolls[width * height];
        for (size_t i = 0; i < line_length; ++i) {
            if (buffer[i] == '@') {
                output_row[i] = true;
            } else if (buffer[i] == '.') {
                output_row[i] = false;
            } else {
                fprintf(stderr, "Invalid character in roll data: %c\n", buffer[i]);
                abort();
            }
        }
        memset(buffer, 0, sizeof(buffer));

        ++height;
    }

    struct roll_grid* rg = new_roll_grid(width, height);
    memcpy(rg->rolls, rolls, width * height * sizeof(bool));
    free(rolls);

    return rg;
}

static uint8_t bool_num(bool x) { return x ? 1 : 0; }

uint8_t* count_adjacent(const struct roll_grid* const rg)
{
    uint8_t* conv_res = malloc(rg->width * rg->height * sizeof(uint8_t));
    memset(conv_res, 0, rg->width * rg->height * sizeof(uint8_t));

    size_t width = rg->width;
    size_t height = rg->height;

    // y = 0
    {
        bool* row = &rg->rolls[0 * width];
        bool* row_below = &rg->rolls[1 * width];
        uint8_t* output_row = &conv_res[0 * width];
        output_row[0] = bool_num(row[1]);
        output_row[0] += bool_num(row_below[0]) + bool_num(row_below[1]);
        for (size_t x = 1; x < width - 1; ++x) {
            output_row[x] = bool_num(row[x - 1]) + bool_num(row[x + 1]);
            output_row[x] += bool_num(row_below[x - 1]) + bool_num(row_below[x]) + bool_num(row_below[x + 1]);
        }
        output_row[width - 1] = bool_num(row[width - 2]);
        output_row[width - 1] += bool_num(row_below[width - 2]) + bool_num(row_below[width - 1]);
    }

    for (size_t y = 1; y < height - 1; ++y) {
        bool* row = &rg->rolls[y * width];
        bool* row_above = &rg->rolls[(y - 1) * width];
        bool* row_below = &rg->rolls[(y + 1) * width];
        uint8_t* output_row = &conv_res[y * width];
        output_row[0] = bool_num(row[1]);
        output_row[0] += bool_num(row_below[0]) + bool_num(row_below[1]);
        output_row[0] += bool_num(row_above[0]) + bool_num(row_above[1]);

        for (size_t x = 1; x < width - 1; ++x) {
            output_row[x] = bool_num(row[x - 1]) + bool_num(row[x + 1]);
            output_row[x] += bool_num(row_above[x - 1]) + bool_num(row_above[x]) + bool_num(row_above[x + 1]);
            output_row[x] += bool_num(row_below[x - 1]) + bool_num(row_below[x]) + bool_num(row_below[x + 1]);
        }
        output_row[width - 1] = bool_num(row[width - 2]);
        output_row[width - 1] += bool_num(row_below[width - 2]) + bool_num(row_below[width - 1]);
        output_row[width - 1] += bool_num(row_above[width - 2]) + bool_num(row_above[width - 1]);
    }

    // y = height-1
    {
        bool* row = &rg->rolls[(height - 1) * width];
        bool* row_above = &rg->rolls[(height - 2) * width];
        uint8_t* output_row = &conv_res[(height - 1) * width];
        output_row[0] = bool_num(row[1]);
        output_row[0] += bool_num(row_above[0]) + bool_num(row_above[1]);
        for (size_t x = 1; x < width - 1; ++x) {
            output_row[x] = bool_num(row[x - 1]) + bool_num(row[x + 1]);
            output_row[x] += bool_num(row_above[x - 1]) + bool_num(row_above[x]) + bool_num(row_above[x + 1]);
        }
        output_row[width - 1] = bool_num(row[width - 2]);
        output_row[width - 1] += bool_num(row_above[width - 2]) + bool_num(row_above[width - 1]);
    }

    return conv_res;
}

struct stack* get_removables(const struct roll_grid* const rg, struct stack* removeables)
{
    stack_clear(removeables);

    uint8_t* conv_res = count_adjacent(rg);
    size_t width = rg->width;
    size_t height = rg->height;

    for (size_t y = 0; y < height; ++y) {
        uint8_t* conv_row = &conv_res[y * width];
        bool* roll_row = &rg->rolls[y * width];
        for (size_t x = 0; x < width; ++x) {
            if (roll_row[x] && conv_row[x] < 4) {
                stack_push(removeables, y * width + x);
            }
        }
    }

    free(conv_res);
    return removeables;
}

size_t part2_greedy(const struct roll_grid* const rg)
{
    struct stack* removeables = new_stack(128);
    removeables = get_removables(rg, removeables);
    if (stack_is_empty(removeables)) {
        free_stack(removeables);
        return 0;
    }

    struct roll_grid* rg_copy = copy_roll_grid(rg);
    size_t count = 0;
    while (!stack_is_empty(removeables)) {
        size_t index = stack_pop(removeables);
        rg_copy->rolls[index] = false;
        ++count;
    }
    count += part2_greedy(rg_copy);

    free_stack(removeables);
    free_roll_grid(rg_copy);
    return count;
}


void part2(const struct roll_grid* const rg)
{
    size_t p2_count = part2_greedy(rg);
    printf("Part 2: %zu\n", p2_count);
}

void part1(const struct roll_grid* const rg)
{
    uint8_t* conv_res = count_adjacent(rg);
    size_t width = rg->width;
    size_t height = rg->height;

    size_t p1_count = 0;
    for (size_t y = 0; y < height; ++y) {
        uint8_t* conv_row = &conv_res[y * width];
        bool* roll_row = &rg->rolls[y * width];
        for (size_t x = 0; x < width; ++x) {
            if (roll_row[x] && conv_row[x] < 4) {
                ++p1_count;
                conv_row[x] = 10;
            }
        }
    }

    printf("Part 1: %zu\n", p1_count);

    free(conv_res);
}

void parse_and_run(FILE* input_stream)
{
    struct roll_grid* rg = parse_roll_grid(input_stream);
    part1(rg);
    part2(rg);

    free_roll_grid(rg);
}

int main()
{
    printf("Test Input:\n");
    const char* test_input = "..@@.@@@@.\n"
                             "@@@.@.@.@@\n"
                             "@@@@@.@.@@\n"
                             "@.@@@@..@.\n"
                             "@@.@@@@.@@\n"
                             ".@@@@@@@.@\n"
                             ".@.@.@.@@@\n"
                             "@.@@@.@@@@\n"
                             ".@@@@@@@@.\n"
                             "@.@.@@@.@.";

    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream);
    fclose(test_stream);

     printf("Real Input:\n");

     FILE* real_input_stream = fopen("../inputs/day4", "r");
     if (!real_input_stream) {
         perror("Failed to open input file");
         return 1;
     }
     parse_and_run(real_input_stream);
     fclose(real_input_stream);

    return 0;
}
