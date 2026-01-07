#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct int2 {
    int64_t x;
    int64_t y;
};

size_t parse_tile_list(FILE* input_stream, struct int2** tiles_out)
{
    size_t capacity = 16;
    size_t count = 0;
    struct int2* tiles = malloc(capacity * sizeof(struct int2));
    if (!tiles) {
        perror("Failed to allocate memory for tiles");
        exit(1);
    }

    while (true) {
        int64_t x, y;
        int result = fscanf(input_stream, "%zu,%zu\n", &x, &y);
        if (result == EOF) {
            break;
        }
        if (result != 2) {
            fprintf(stderr, "Invalid input format\n");
            free(tiles);
            exit(1);
        }

        if (count >= capacity) {
            capacity *= 2;
            struct int2* new_tiles = realloc(tiles, capacity * sizeof(struct int2));
            if (!new_tiles) {
                perror("Failed to reallocate memory for tiles");
                free(tiles);
                exit(1);
            }
            tiles = new_tiles;
        }

        tiles[count].x = x;
        tiles[count].y = y;
        count++;
    }

    *tiles_out = tiles;
    return count;
}

void print_tile_list(struct int2* tiles, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        printf("Tile %zu: (%zu, %zu)\n", i, tiles[i].x, tiles[i].y);
    }
}

int int2_cmp(void const* a, void const* b)
{
    struct int2 const* ia = (struct int2 const*)a;
    struct int2 const* ib = (struct int2 const*)b;
    if (ia->x != ib->x) {
        return (ia->x < ib->x) ? -1 : 1;
    }
    if (ia->y != ib->y) {
        return (ia->y < ib->y) ? -1 : 1;
    }
    return 0;
}

void part1(struct int2* tiles, size_t count)
{
    struct int2* tiles_copy = malloc(count * sizeof(struct int2));
    memcpy(tiles_copy, tiles, count * sizeof(struct int2));
    tiles = tiles_copy;
    qsort(tiles, count, sizeof(struct int2), int2_cmp);

    int64_t max_area = 0;
    for (size_t i = 0; i < count; i++) {
        for (size_t j = count - 1; j > i; j--) {
            int64_t width = labs(tiles[j].x - tiles[i].x) + 1;
            int64_t height = labs(tiles[j].y - tiles[i].y) + 1;
            int64_t area = width * height;
            if (area > max_area) {
                max_area = area;
            }
        }
    }

    printf("Part 1: %zu\n", max_area);
}

struct stack {
    size_t capacity;
    size_t size;
    struct int2* items;
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
    s->items = malloc(capacity * sizeof(struct int2));
    return s;
}

bool stack_is_empty(const struct stack* const s) { return s->size == 0; }

void stack_push(struct stack* s, struct int2 item)
{
    if (s->size >= s->capacity) {
        while (s->capacity <= s->size) {
            s->capacity *= 2;
        }
        s->items = realloc(s->items, s->capacity * sizeof(struct int2));
    }
    s->items[s->size] = item;
    s->size++;
}

struct int2 stack_pop(struct stack* s)
{
    if (s->size == 0) {
        fprintf(stderr, "Pop from empty stack\n");
        abort();
    }
    s->size--;
    return s->items[s->size];
}

struct grid {
    int64_t width;
    int64_t height;
    struct int2 min;
    bool* cells;
};

struct grid* new_grid(int64_t width, int64_t height, struct int2 min)
{
    struct grid* g = malloc(sizeof(struct grid));
    g->width = width;
    g->height = height;
    g->min = min;
    g->cells = calloc((size_t)width * (size_t)height, sizeof(bool));
    return g;
}

void free_grid(struct grid* g)
{
    if (g) {
        free(g->cells);
        free(g);
    }
}

bool grid_get(const struct grid* g, int64_t x, int64_t y)
{
    int64_t gx = x - g->min.x;
    int64_t gy = y - g->min.y;
    if (gx < 0 || gx >= g->width || gy < 0 || gy >= g->height) {
        fprintf(stderr, "Grid get out of bounds: (%zu, %zu)\n", x, y);
        exit(1);
    }
    return g->cells[gy * g->width + gx];
}

void grid_set(struct grid* g, int64_t x, int64_t y, bool value)
{
    int64_t gx = x - g->min.x;
    int64_t gy = y - g->min.y;
    if (gx < 0 || gx >= g->width || gy < 0 || gy >= g->height) {
        fprintf(stderr, "Grid get out of bounds: (%zu, %zu)\n", x, y);
        exit(1);
        return;
    }
    g->cells[gy * g->width + gx] = value;
}

bool line_is_horiz(struct int2 start, struct int2 end) { return start.y == end.y; }

void print_grid(const struct grid* g)
{
    for (int64_t y = 0; y < g->height; y++) {
        for (int64_t x = 0; x < g->width; x++) {
            bool cell = g->cells[y * g->width + x];
            putchar(cell ? '#' : '.');
        }
        putchar('\n');
    }
}

enum direction { DIR_RIGHT, DIR_UP, DIR_LEFT, DIR_DOWN };
const char* direction_to_string(enum direction dir)
{
    switch (dir) {
    case DIR_RIGHT:
        return "RIGHT";
    case DIR_UP:
        return "UP";
    case DIR_LEFT:
        return "LEFT";
    case DIR_DOWN:
        return "DOWN";
    default:
        return "UNKNOWN";
    }
}

bool is_clockwise_turn(enum direction from, enum direction to)
{
    if (from == DIR_RIGHT && to == DIR_DOWN) {
        return true;
    }
    if (from == DIR_DOWN && to == DIR_LEFT) {
        return true;
    }
    if (from == DIR_LEFT && to == DIR_UP) {
        return true;
    }
    if (from == DIR_UP && to == DIR_RIGHT) {
        return true;
    }
    return false;
}

enum direction dir(struct int2 start, struct int2 end)
{
    enum direction dir;
    if (line_is_horiz(start, end)) {
        if (start.x < end.x) {
            dir = DIR_RIGHT;
        } else {
            dir = DIR_LEFT;
        }
    } else {
        if (start.y < end.y) {
            dir = DIR_DOWN;
        } else {
            dir = DIR_UP;
        }
    }
    return dir;
}

bool ray_line_intersection(struct int2 line_start, struct int2 line_end, struct int2 ray_origin, enum direction ray_dir)
{
    if (line_is_horiz(line_start, line_end)) {
        int64_t line_y = line_start.y;
        assert(line_start.x <= line_end.x);

        if (ray_dir == DIR_UP || ray_dir == DIR_DOWN) {
            // Horizontal, perpendicular case
            if (ray_origin.x >= line_start.x && ray_origin.x <= line_end.x) {
                // Within x bounds
                return (ray_dir == DIR_UP && ray_origin.y >= line_y) //
                    || (ray_dir == DIR_DOWN && ray_origin.y <= line_y);
            }
        } else {
            // Horizontal, parallel case
            return ray_origin.y == line_y
                && ((ray_dir == DIR_RIGHT && ray_origin.x <= line_end.x)
                    || (ray_dir == DIR_LEFT && ray_origin.x >= line_start.x));
        }
    } else {
        int64_t line_x = line_start.x;
        assert(line_start.y <= line_end.y);
        if (ray_dir == DIR_LEFT || ray_dir == DIR_RIGHT) {
            // vertical, perpendicular case
            if (ray_origin.y >= line_start.y && ray_origin.y <= line_end.y) {
                // Within y bounds
                return (ray_dir == DIR_LEFT && ray_origin.x >= line_x)
                    || (ray_dir == DIR_RIGHT && ray_origin.x <= line_x);
            }
        } else {
            // vertical, parallel case
            return ray_origin.x == line_x
                && ((ray_dir == DIR_UP && ray_origin.y >= line_start.y)
                    || (ray_dir == DIR_DOWN && ray_origin.y <= line_end.y));
        }
    }
    return false;
}

bool point_in_polygon(struct int2 point, struct int2* polygon, size_t polygon_count)
{
    size_t intersection_count = 0;

    enum direction ray_dir = DIR_RIGHT;
    for (size_t i = 0; i < polygon_count; i++) {
        struct int2 line_start = polygon[i];
        struct int2 line_end = polygon[(i + 1) % polygon_count];

        // Trim end point to avoid double counting
        if (line_is_horiz(line_start, line_end)) {
            if (line_start.x < line_end.x) {
                line_end.x -= 1;
            } else {
                line_end.x += 1;
            }
        } else {
            if (line_start.y < line_end.y) {
                line_end.y -= 1;
            } else {
                line_end.y += 1;
            }
        }

        struct int2 effective_line_start = (struct int2) {
            .x = line_start.x < line_end.x ? line_start.x : line_end.x,
            .y = line_start.y < line_end.y ? line_start.y : line_end.y,
        };
        struct int2 effective_line_end = (struct int2) {
            .x = line_start.x < line_end.x ? line_end.x : line_start.x,
            .y = line_start.y < line_end.y ? line_end.y : line_start.y,
        };

        if (line_is_horiz(effective_line_start, effective_line_end) && point.y == effective_line_start.y
            && point.x >= effective_line_start.x && point.x <= effective_line_end.x) {
            // Point is on horizontal line
            return false;
        } else if (!line_is_horiz(effective_line_start, effective_line_end) && point.x == effective_line_start.x
            && point.y >= effective_line_start.y && point.y <= effective_line_end.y) {
            // Point is on vertical line
            return false;
        }

        if (ray_line_intersection(effective_line_start, effective_line_end, point, ray_dir)) {
            intersection_count++;
        }
    }

    return (intersection_count % 2) == 1;
}

int int64_t_cmp_void(const void* a, const void* b)
{
    int64_t ia = *(const int64_t*)a;
    int64_t ib = *(const int64_t*)b;
    if (ia < ib) {
        return -1;
    } else if (ia > ib) {
        return 1;
    } else {
        return 0;
    }
}


void part2(struct int2* tiles_input, size_t count)
{
    // Grab values
    int64_t* x_values = calloc(count, sizeof(int64_t));
    int64_t* y_values = calloc(count, sizeof(int64_t));
    for (size_t i = 0; i < count; i++) {
        x_values[i] = tiles_input[i].x;
        y_values[i] = tiles_input[i].y;
    }

    // Sort
    qsort(x_values, count, sizeof(int64_t), int64_t_cmp_void);
    qsort(y_values, count, sizeof(int64_t), int64_t_cmp_void);

    // Dedupe
    int64_t* x_values_dedup = calloc(count, sizeof(int64_t));
    int64_t* y_values_dedup = calloc(count, sizeof(int64_t));
    size_t x_uniq_count = 1;
    size_t y_uniq_count = 1;
    {
        if (count == 0) {
            return;
        }
        int64_t last_x = x_values[0];
        int64_t last_y = y_values[0];
        x_values_dedup[0] = last_x;
        y_values_dedup[0] = last_y;

        for (size_t i = 1; i < count; i++) {
            if (x_values[i] != last_x) {
                last_x = x_values[i];
                x_values_dedup[x_uniq_count] = last_x;
                x_uniq_count++;
            }
            if (y_values[i] != last_y) {
                last_y = y_values[i];
                y_values_dedup[y_uniq_count] = last_y;
                y_uniq_count++;
            }
        }
    }

    free(x_values);
    free(y_values);

    // Coordinate compress tiles
    struct int2* tiles_compressed = malloc(count * sizeof(struct int2));
    for (size_t i = 0; i < count; i++) {
        struct int2 input_tile = tiles_input[i];
        void* found_x = bsearch(&input_tile.x, x_values_dedup, x_uniq_count, sizeof(int64_t), int64_t_cmp_void);
        void* found_y = bsearch(&input_tile.y, y_values_dedup, y_uniq_count, sizeof(int64_t), int64_t_cmp_void);
        if (!found_x || !found_y) {
            fprintf(stderr, "Failed to find tile value in compressed list\n");
            exit(1);
        }
        size_t compressed_x = (size_t)((int64_t*)found_x - x_values_dedup);
        size_t compressed_y = (size_t)((int64_t*)found_y - y_values_dedup);
        tiles_compressed[i] = (struct int2) { .x = (int64_t)compressed_x, .y = (int64_t)compressed_y };
    }

    struct int2* tiles = tiles_compressed;

    struct int2 min = { INT64_MAX, INT64_MAX };
    struct int2 max = { INT64_MIN, INT64_MIN };
    for (size_t i = 0; i < count; i++) {
        if (tiles[i].x < min.x) {
            min.x = tiles[i].x;
        }
        if (tiles[i].x > max.x) {
            max.x = tiles[i].x;
        }
        if (tiles[i].y < min.y) {
            min.y = tiles[i].y;
        }
        if (tiles[i].y > max.y) {
            max.y = tiles[i].y;
        }
    }
    size_t grid_width = (size_t)(max.x - min.x + 1);
    size_t grid_height = (size_t)(max.y - min.y + 1);

    printf("Grid size: %zu x %zu\n", grid_width, grid_height);

    struct grid* grid = new_grid((int64_t)grid_width, (int64_t)grid_height, min);
    struct grid* visited_grid = new_grid((int64_t)grid_width, (int64_t)grid_height, min);

    struct stack* flood_fill_stack = new_stack(16);

    // Find first insertion point
    {
        struct int2 first_tile = tiles[0];
        struct int2 test_tiles[8] = {
            { first_tile.x - 1, first_tile.y - 1 },
            { first_tile.x - 1, first_tile.y },
            { first_tile.x - 1, first_tile.y + 1 },
            { first_tile.x, first_tile.y - 1 },
            { first_tile.x, first_tile.y + 1 },
            { first_tile.x + 1, first_tile.y - 1 },
            { first_tile.x + 1, first_tile.y },
            { first_tile.x + 1, first_tile.y + 1 },
        };

        for (size_t i = 0; i < 8; i++) {
            struct int2 test_tile = test_tiles[i];
            if (point_in_polygon(test_tile, tiles, count)) {
                printf("Insertion tile: (%zu, %zu)\n", test_tile.x, test_tile.y);
                stack_push(flood_fill_stack, test_tile);
                break;
            }
        }
        if (stack_is_empty(flood_fill_stack)) {
            fprintf(stderr, "Failed to find insertion tile\n");
            exit(1);
        }
    }

    // Premark perimeter
    {
        for (size_t i = 0; i < count; i++) {
            struct int2 tile_line_start = tiles[i];
            struct int2 tile_line_end = tiles[(i + 1) % count];
            if (line_is_horiz(tile_line_start, tile_line_end)) {
                int64_t effective_start = tile_line_start.x < tile_line_end.x ? tile_line_start.x : tile_line_end.x;
                int64_t effective_end = tile_line_start.x > tile_line_end.x ? tile_line_start.x : tile_line_end.x;
                for (int64_t x = effective_start; x <= effective_end; x++) {
                    if (x != tile_line_start.x && grid_get(grid, x, tile_line_start.y)) {
                        printf("Self-intersecting perimeter detected\n");
                    }
                    grid_set(grid, x, tile_line_start.y, true);
                }
            } else {
                int64_t effective_start = tile_line_start.y < tile_line_end.y ? tile_line_start.y : tile_line_end.y;
                int64_t effective_end = tile_line_start.y > tile_line_end.y ? tile_line_start.y : tile_line_end.y;
                for (int64_t y = effective_start; y <= effective_end; y++) {
                    if (y != tile_line_start.y && grid_get(grid, tile_line_start.x, y)) {
                        printf("Self-intersecting perimeter detected\n");
                    }
                    grid_set(grid, tile_line_start.x, y, true);
                }
            }
        }
    }

    printf("Starting flood fill\n");

    while (!stack_is_empty(flood_fill_stack)) {
        struct int2 pos = stack_pop(flood_fill_stack);

        if (grid_get(grid, pos.x, pos.y)) {
            continue;
        }
        grid_set(grid, pos.x, pos.y, true);

        if (pos.x < min.x || pos.x > max.x || pos.y < min.y || pos.y > max.y) {
            fprintf(stderr, "Flood fill reached out of bounds at (%zu, %zu)\n", pos.x, pos.y);
            exit(1);
        }

        for (int64_t dy = -1; dy <= 1; dy++) {
            for (int64_t dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                struct int2 neighbor = { pos.x + dx, pos.y + dy };
                if (grid_get(grid, neighbor.x, neighbor.y)) {
                    continue;
                }
                if (grid_get(visited_grid, neighbor.x, neighbor.y)) {
                    continue;
                }
                grid_set(visited_grid, neighbor.x, neighbor.y, true);
                stack_push(flood_fill_stack, neighbor);
            }
        }
    }
    printf("Flood fill complete\n");

    int64_t max_area = 0;
    for (size_t i = 0; i < count; i++) {
        for (size_t j = count - 1; j > i; j--) {

            int64_t min_x = tiles[i].x < tiles[j].x ? tiles[i].x : tiles[j].x;
            int64_t max_x = tiles[i].x > tiles[j].x ? tiles[i].x : tiles[j].x;
            int64_t min_y = tiles[i].y < tiles[j].y ? tiles[i].y : tiles[j].y;
            int64_t max_y = tiles[i].y > tiles[j].y ? tiles[i].y : tiles[j].y;

            bool enclosed = true;
            for (int64_t y = min_y; y <= max_y; y++) {
                for (int64_t x = min_x; x <= max_x; x++) {
                    if (!grid_get(grid, x, y)) {
                        enclosed = false;
                        break;
                    }
                }
                if (!enclosed) {
                    break;
                }
            }
            if (!enclosed) {
                continue;
            }

            struct int2 corner1 = { x_values_dedup[(size_t)tiles[i].x], y_values_dedup[(size_t)tiles[i].y] };
            struct int2 corner2 = { x_values_dedup[(size_t)tiles[j].x], y_values_dedup[(size_t)tiles[j].y] };
            int64_t width = labs(corner1.x - corner2.x) + 1;
            int64_t height = labs(corner1.y - corner2.y) + 1;
            int64_t area = width * height;
            if (area > max_area) {
                max_area = area;
            }
        }
    }
    printf("Part 2: %zu\n", max_area);

    // print_grid(grid);

    free(x_values_dedup);
    free(y_values_dedup);
    free_grid(grid);
    free_grid(visited_grid);
    free_stack(flood_fill_stack);
}

void parse_and_run(FILE* input_stream)
{
    struct int2* tiles;
    size_t count = parse_tile_list(input_stream, &tiles);
    // print_tile_list(tiles, count);
    part1(tiles, count);
    part2(tiles, count);

    free(tiles);
}

int main()
{
    printf("Test Input:\n");

    const char* test_input = "7,1\n"
                             "7,3\n"
                             "2,3\n"
                             "2,5\n"
                             "9,5\n"
                             "9,7\n"
                             "11,7\n"
                             "11,1";

    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream);
    fclose(test_stream);

    printf("Real Input:\n");

    FILE* real_input_stream = fopen("../inputs/day9", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }
    parse_and_run(real_input_stream);
    fclose(real_input_stream);

    return 0;
}
