#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sorted_array {
    size_t capacity;
    size_t size;
    int64_t* items;
};

void sorted_array_free(struct sorted_array* sa)
{
    if (sa) {
        free(sa->items);
        free(sa);
    }
}

struct sorted_array* sorted_array_new(size_t capacity)
{
    struct sorted_array* sa = malloc(sizeof(struct sorted_array));
    sa->capacity = capacity;
    sa->size = 0;
    sa->items = malloc(capacity * sizeof(size_t));
    return sa;
}

struct found {
    bool found;
    size_t index;
};

struct found insert_point(const void* key, const void* base, size_t nmemb, size_t size, __compar_fn_t compar)
{
    size_t l = 0;
    size_t u = nmemb;

    if (nmemb == 0) {
        return (struct found) { .found = false, .index = 0 };
    }

    while (l < u) {
        size_t idx = (l + u) / 2;
        const void* cur = (const void*)(((const char*)base) + (idx * size));
        int comparison = (*compar)(cur, key);
        if (comparison < 0) {
            l = idx + 1;
        } else if (comparison > 0) {
            u = idx;
        } else {
            return (struct found) { .found = true, .index = idx };
        }
    }

    return (struct found) { .found = false, .index = u };
}

int int64_cmp(const void* a, const void* b)
{
    const int64_t* ra = (const int64_t*)a;
    const int64_t* rb = (const int64_t*)b;

    if (*ra < *rb) {
        return -1;
    } else if (*ra > *rb) {
        return 1;
    } else {
        return 0;
    }
}

void sorted_array_insert(struct sorted_array* sa, int64_t item)
{
    if (sa->size >= sa->capacity) {
        while (sa->capacity <= sa->size) {
            sa->capacity *= 2;
        }
        sa->items = realloc(sa->items, sa->capacity * sizeof(int64_t));
    }

    struct found insert_result = insert_point(&item, sa->items, sa->size, sizeof(int64_t), int64_cmp);
    if (insert_result.found) {
        return;
    }
    size_t i = insert_result.index;

    memmove(&sa->items[i + 1], &sa->items[i], (sa->size - i) * sizeof(int64_t));
    sa->items[i] = item;
    sa->size++;
}

void sorted_array_remove(struct sorted_array* sa, int64_t item)
{
    struct found search_result = insert_point(&item, sa->items, sa->size, sizeof(int64_t), int64_cmp);
    if (!search_result.found) {
        return;
    }

    size_t i = search_result.index;
    memmove(&sa->items[i], &sa->items[i + 1], (sa->size - i - 1) * sizeof(int64_t));
    sa->size--;
}

bool sorted_array_contains(const struct sorted_array* sa, int64_t item)
{
    void* search_ret = bsearch(&item, sa->items, sa->size, sizeof(int64_t), int64_cmp);
    return search_ret != NULL;
}

struct int2 {
    int64_t x;
    int64_t y;
};

struct tachyon_manifold {
    struct int2 start;
    struct int2* splitters;
    size_t num_splitters;
    size_t width;
    size_t height;
};

void free_tachyon_manifold(struct tachyon_manifold* tm)
{
    free(tm->splitters);
    free(tm);
}

struct tachyon_manifold* parse_tachyon_manifold(FILE* input_stream)
{
    char line_buffer[256];
    size_t height = 0;
    size_t width = SIZE_MAX;
    struct int2 start = { LLONG_MAX, LLONG_MAX };
    struct int2* splitters = NULL;
    size_t num_splitters = 0;

    while (fgets(line_buffer, sizeof(line_buffer), input_stream)) {
        if (line_buffer[0] == '\n' || line_buffer[0] == '\0') {
            break;
        }
        size_t len = strnlen(line_buffer, sizeof(line_buffer));
        if (line_buffer[len - 1] == '\n') {
            --len;
            line_buffer[len] = '\0';
        }
        if (width == SIZE_MAX) {
            width = len;
        } else if (len != width) {
            fprintf(stderr, "Inconsistent line lengths in input\n");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < len; ++i) {
            if (line_buffer[i] == '^') {
                splitters = realloc(splitters, (num_splitters + 1) * sizeof(struct int2));
                if (!splitters) {
                    perror("Failed to allocate memory for splitters");
                    exit(EXIT_FAILURE);
                }
                splitters[num_splitters].x = (int64_t)i;
                splitters[num_splitters].y = (int64_t)height;
                ++num_splitters;
            } else if (line_buffer[i] == 'S') {
                if (start.x != LLONG_MAX || start.y != LLONG_MAX) {
                    fprintf(stderr, "Multiple start positions in input\n");
                    exit(EXIT_FAILURE);
                }
                start.x = (int64_t)i;
                start.y = (int64_t)height;
            }
        }

        ++height;
    }
    if (start.x == LLONG_MAX || start.y == LLONG_MAX) {
        fprintf(stderr, "No start position found in input\n");
        exit(EXIT_FAILURE);
    }

    struct tachyon_manifold* tm = malloc(sizeof(struct tachyon_manifold));
    *tm = (struct tachyon_manifold) {
        .start = start, .splitters = splitters, .num_splitters = num_splitters, .width = width, .height = height
    };

    return tm;
}

void print_tachyon_manifold(struct tachyon_manifold* tm)
{
    printf("Tachyon Manifold:\n");
    printf(" Start Position: (%lld, %lld)\n", (long long)tm->start.x, (long long)tm->start.y);
    printf(" Dimensions: %zu x %zu\n", tm->width, tm->height);
    printf(" Splitters:\n");
    for (size_t i = 0; i < tm->num_splitters; ++i) {
        printf("  (%lld, %lld)\n", (long long)tm->splitters[i].x, (long long)tm->splitters[i].y);
    }
}

void part1(struct tachyon_manifold* tm)
{
    int64_t start_x = tm->start.x;
    struct sorted_array* beams = sorted_array_new(tm->num_splitters);
    sorted_array_insert(beams, start_x);

    size_t split_count = 0;

    for (size_t i = 0; i < tm->num_splitters; ++i) {
        int64_t splitter_x = tm->splitters[i].x;
        if (sorted_array_contains(beams, splitter_x)) {
            sorted_array_remove(beams, splitter_x);
            ++split_count;
            if (splitter_x > 0) {
                sorted_array_insert(beams, splitter_x + 1);
            }
            if (splitter_x + 1 < (int64_t)tm->width) {
                sorted_array_insert(beams, splitter_x + 1);
            }
            sorted_array_insert(beams, splitter_x - 1);
        }
    }

    printf("Part 1: Number of splits through splitters: %zu\n", split_count);

    sorted_array_free(beams);
}

size_t paths_count2(struct tachyon_manifold* tm, size_t splitter_index, size_t** memo)
{
    if (memo[splitter_index]) {
        return *memo[splitter_index];
    }

    size_t total_paths = 0;
    int64_t beam_x = tm->splitters[splitter_index].x;
    bool left = beam_x > 0;
    bool right = beam_x + 1 < (int64_t)tm->width;
    bool found_left = false;
    bool found_right = false;

    for (size_t i = splitter_index + 1; i < tm->num_splitters; ++i) {
        int64_t next_splitter_x = tm->splitters[i].x;
        if (left && !found_left && next_splitter_x == beam_x - 1) {
            total_paths += paths_count2(tm, i, memo);
            found_left = true;
        } else if (right && !found_right && next_splitter_x == beam_x + 1) {
            total_paths += paths_count2(tm, i, memo);
            found_right = true;
        }

        if (found_left && found_right) {
            break;
        }
    }

    if (left && !found_left) {
        total_paths += 1;
    }
    if (right && !found_right) {
        total_paths += 1;
    }

    memo[splitter_index] = malloc(sizeof(size_t));
    *memo[splitter_index] = total_paths;

    return total_paths;
}

size_t paths_count(struct tachyon_manifold* tm, int64_t beam_x, size_t splitter_index_search)
{
    bool found_splitter = false;
    size_t total_paths = 0;
    for (size_t i = splitter_index_search; i < tm->num_splitters; ++i) {
        int64_t next_splitter_x = tm->splitters[i].x;
        if (next_splitter_x == beam_x) {
            if (beam_x > 0) {
                total_paths += paths_count(tm, beam_x - 1, i + 1);
            }
            if (beam_x + 1 < (int64_t)tm->width) {
                total_paths += paths_count(tm, beam_x + 1, i + 1);
            }
            found_splitter = true;
            break;
        }
    }

    if (!found_splitter) {
        return 1;
    }

    return total_paths;
}

void part2(struct tachyon_manifold* tm)
{
    size_t** memo = calloc(tm->num_splitters, sizeof(size_t*));

    int64_t start_x = tm->start.x;
    size_t first_splitter_idx = SIZE_MAX;
    for (size_t i = 0; i < tm->num_splitters; ++i) {
        if (tm->splitters[i].x == start_x) {
            first_splitter_idx = i;
            break;
        }
    }

    if (first_splitter_idx == SIZE_MAX) {
        fprintf(stderr, "Start position does not align with any splitter for part 2\n");
        exit(EXIT_FAILURE);
    }

    size_t total_paths = paths_count2(tm, first_splitter_idx, memo);

    for (size_t i = 0; i < tm->num_splitters; ++i) {
        if (memo[i]) {
            free(memo[i]);
        }
    }
    free(memo);

    printf("Part 2: Total distinct paths through the manifold: %zu\n", total_paths);
}

void parse_and_run(FILE* input_stream)
{
    struct tachyon_manifold* mh = parse_tachyon_manifold(input_stream);
    part1(mh);
    part2(mh);

    free_tachyon_manifold(mh);
}

int main()
{
    printf("Test Input:\n");

    const char* test_input = ".......S.......\n"
                             "...............\n"
                             ".......^.......\n"
                             "...............\n"
                             "......^.^......\n"
                             "...............\n"
                             ".....^.^.^.....\n"
                             "...............\n"
                             "....^.^...^....\n"
                             "...............\n"
                             "...^.^...^.^...\n"
                             "...............\n"
                             "..^...^.....^..\n"
                             "...............\n"
                             ".^.^.^.^.^...^.\n"
                             "...............";

    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream);
    fclose(test_stream);

    printf("Real Input:\n");

    FILE* real_input_stream = fopen("../inputs/day7", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }
    parse_and_run(real_input_stream);
    fclose(real_input_stream);

    return 0;
}
