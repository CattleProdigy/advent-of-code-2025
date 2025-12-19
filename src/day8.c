#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct int3 {
    int64_t x;
    int64_t y;
    int64_t z;
};

size_t parse_box_locations(FILE* input_stream, struct int3** out_end)
{
    struct int3* locations = NULL;
    size_t count = 0;
    size_t capacity = 0;

    char line[256];
    while (fgets(line, sizeof(line), input_stream)) {
        if (count >= capacity) {
            capacity = capacity ? capacity * 2 : 16;
            locations = realloc(locations, capacity * sizeof(struct int3));
        }
        sscanf(line, "%lld,%lld,%lld", (long long*)&locations[count].x, (long long*)&locations[count].y,
            (long long*)&locations[count].z);
        count++;
    }

    *out_end = locations;

    return count;
}

struct box_pair_distance {
    size_t box1;
    size_t box2;
    int64_t distance;
};

int bpd_cmp(const void* a, const void* b)
{
    const struct box_pair_distance* bd_a = (const struct box_pair_distance*)a;
    const struct box_pair_distance* bd_b = (const struct box_pair_distance*)b;

    if (bd_a->distance < bd_b->distance) {
        return -1;
    } else if (bd_a->distance > bd_b->distance) {
        return 1;
    } else {
        return 0;
    }
}

int sizet_cmp(const void* a, const void* b)
{
    const size_t* sa = (const size_t*)a;
    const size_t* sb = (const size_t*)b;

    if (*sa > *sb) {
        return -1;
    } else if (*sa < *sb) {
        return 1;
    } else {
        return 0;
    }
}

void part1(struct int3* locs, size_t count, size_t connection_count)
{
    size_t pairings = count * (count - 1) / 2;
    struct box_pair_distance* distances = malloc(pairings * sizeof(struct box_pair_distance));

    size_t pair_idx = 0;
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ++j, ++pair_idx) {
            int64_t dx = locs[i].x - locs[j].x;
            int64_t dy = locs[i].y - locs[j].y;
            int64_t dz = locs[i].z - locs[j].z;
            distances[pair_idx]
                = (struct box_pair_distance) { .box1 = i, .box2 = j, .distance = dx * dx + dy * dy + dz * dz };
        }
    }

    qsort(distances, pairings, sizeof(struct box_pair_distance), bpd_cmp);

    uint64_t* circuit_assignments = malloc(count * sizeof(uint64_t));
    for (size_t i = 0; i < count; ++i) {
        circuit_assignments[i] = ULLONG_MAX;
    }

    if (connection_count > pairings) {
        fprintf(stderr, "Requested connections exceed possible pairings\n");
        exit(EXIT_FAILURE);
    }

    size_t cur_circuit = 0;
    size_t connections_made = 0;

    for (size_t i = 0; i < pairings; ++i) {
        size_t box1 = distances[i].box1;
        size_t box2 = distances[i].box2;

        uint64_t* circuit1 = &circuit_assignments[box1];
        uint64_t* circuit2 = &circuit_assignments[box2];

        if (*circuit1 == ULLONG_MAX && *circuit2 == ULLONG_MAX) {
            uint64_t new_circuit = cur_circuit;
            *circuit1 = new_circuit;
            *circuit2 = new_circuit;
            cur_circuit++;
            connections_made++;
        } else if (*circuit1 != ULLONG_MAX && *circuit2 == ULLONG_MAX) {
            *circuit2 = *circuit1;
            connections_made++;
        } else if (*circuit1 == ULLONG_MAX && *circuit2 != ULLONG_MAX) {
            *circuit1 = *circuit2;
            connections_made++;
        } else if (*circuit1 != *circuit2) {
            assert(*circuit1 != ULLONG_MAX && *circuit2 != ULLONG_MAX);
            uint64_t old_circuit;
            uint64_t new_circuit;
            if (*circuit1 < *circuit2) {
                old_circuit = *circuit2;
                new_circuit = *circuit1;
            } else {
                old_circuit = *circuit1;
                new_circuit = *circuit2;
            }
            for (size_t j = 0; j < count; ++j) {
                if (circuit_assignments[j] == old_circuit) {
                    circuit_assignments[j] = new_circuit;
                }
            }
            connections_made++;
        } else {
            connections_made++;
        }

        if (connections_made >= connection_count) {
            break;
        }
    }

    size_t* circuit_counts = calloc(count, sizeof(size_t));

    for (size_t i = 0; i < count; ++i) {
        if (circuit_assignments[i] != ULLONG_MAX) {
            circuit_counts[circuit_assignments[i]]++;
        } else {
            circuit_counts[cur_circuit]++;
            cur_circuit++;
        }
    }
    qsort(circuit_counts, cur_circuit, sizeof(size_t), sizet_cmp);

    size_t product = 1;
    for (size_t i = 0; i < 3; ++i) {
        product *= circuit_counts[i];
    }

    printf("Product of sizes of three largest circuits: %zu\n", product);

    free(circuit_counts);
    free(circuit_assignments);
    free(distances);
}

void part2(struct int3* locs, size_t count)
{
    size_t pairings = count * (count - 1) / 2;
    struct box_pair_distance* distances = malloc(pairings * sizeof(struct box_pair_distance));

    size_t pair_idx = 0;
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ++j, ++pair_idx) {
            int64_t dx = locs[i].x - locs[j].x;
            int64_t dy = locs[i].y - locs[j].y;
            int64_t dz = locs[i].z - locs[j].z;
            distances[pair_idx]
                = (struct box_pair_distance) { .box1 = i, .box2 = j, .distance = dx * dx + dy * dy + dz * dz };
        }
    }

    qsort(distances, pairings, sizeof(struct box_pair_distance), bpd_cmp);

    uint64_t* circuit_assignments = malloc(count * sizeof(uint64_t));
    for (size_t i = 0; i < count; ++i) {
        circuit_assignments[i] = ULLONG_MAX;
    }

    size_t cur_circuit = 0;

    size_t last_pairing_a = SIZE_MAX;
    size_t last_pairing_b = SIZE_MAX;
    for (size_t i = 0; i < pairings; ++i) {
        size_t box1 = distances[i].box1;
        size_t box2 = distances[i].box2;

        uint64_t* circuit1 = &circuit_assignments[box1];
        uint64_t* circuit2 = &circuit_assignments[box2];

        if (*circuit1 == ULLONG_MAX && *circuit2 == ULLONG_MAX) {
            uint64_t new_circuit = cur_circuit;
            *circuit1 = new_circuit;
            *circuit2 = new_circuit;
            cur_circuit++;
        } else if (*circuit1 != ULLONG_MAX && *circuit2 == ULLONG_MAX) {
            *circuit2 = *circuit1;
            last_pairing_a = box1;
            last_pairing_b = box2;
        } else if (*circuit1 == ULLONG_MAX && *circuit2 != ULLONG_MAX) {
            *circuit1 = *circuit2;
            last_pairing_a = box1;
            last_pairing_b = box2;
        } else if (*circuit1 != *circuit2) {
            assert(*circuit1 != ULLONG_MAX && *circuit2 != ULLONG_MAX);
            uint64_t old_circuit;
            uint64_t new_circuit;
            if (*circuit1 < *circuit2) {
                old_circuit = *circuit2;
                new_circuit = *circuit1;
            } else {
                old_circuit = *circuit1;
                new_circuit = *circuit2;
            }
            last_pairing_a = box1;
            last_pairing_b = box2;
            for (size_t j = 0; j < count; ++j) {
                if (circuit_assignments[j] == old_circuit) {
                    circuit_assignments[j] = new_circuit;
                }
            }
        }
    }

    printf("Last pairing that connected two circuits: Box %zu and Box %zu\n", last_pairing_a, last_pairing_b);

    int64_t pt_prod = locs[last_pairing_a].x * locs[last_pairing_b].x;
    printf("Product of their X coordinates: %lld\n", (long long)pt_prod);

    free(circuit_assignments);
    free(distances);
}

void print_box_locations(struct int3* locs, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        printf("Box Location %zu: (%lld, %lld, %lld)\n", i, (long long)locs[i].x, (long long)locs[i].y,
            (long long)locs[i].z);
    }
}

void parse_and_run(FILE* input_stream, size_t connections)
{
    struct int3* locations;
    size_t count = parse_box_locations(input_stream, &locations);
    // print_box_locations(locations, count);
    part1(locations, count, connections);
    part2(locations, count);

    free(locations);
}

int main()
{
    printf("Test Input:\n");

    const char* test_input = "162,817,812\n"
                             "57,618,57\n"
                             "906,360,560\n"
                             "592,479,940\n"
                             "352,342,300\n"
                             "466,668,158\n"
                             "542,29,236\n"
                             "431,825,988\n"
                             "739,650,466\n"
                             "52,470,668\n"
                             "216,146,977\n"
                             "819,987,18\n"
                             "117,168,530\n"
                             "805,96,715\n"
                             "346,949,466\n"
                             "970,615,88\n"
                             "941,993,340\n"
                             "862,61,35\n"
                             "984,92,344\n"
                             "425,690,689";

    FILE* test_stream = fmemopen((void*)test_input, strlen(test_input), "r");
    parse_and_run(test_stream, 10);
    fclose(test_stream);

    printf("Real Input:\n");

    FILE* real_input_stream = fopen("../inputs/day8", "r");
    if (!real_input_stream) {
        perror("Failed to open input file");
        return 1;
    }
    parse_and_run(real_input_stream, 1000);
    fclose(real_input_stream);

    return 0;
}
