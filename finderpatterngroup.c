#include <math.h>
#include <stdlib.h>

#include "finderpatterngroup.h"

#define MIN_MODULES_PER_EDGE 9
#define MAX_MODULES_PER_EDGE 180

#define MAX_MODULE_SIZE_DIFF 0.5f

static int compare_module_sizes(struct finder_pattern_list* *a, struct finder_pattern_list* *b) {
    if ((*a)->pattern.module_size < (*b)->pattern.module_size) {
        return -1;
    }
    if ((*a)->pattern.module_size > (*b)->pattern.module_size) {
        return 1;
    }
    return 0;
}

/**
 * Returns an array where the elements are pointers to the original list elements,
 * but sorted by increasing module size.
 */
static struct finder_pattern_list** create_sorted_array(unsigned int n, struct finder_pattern_list* list) {
    struct finder_pattern_list** array = (struct finder_pattern_list**)malloc(n * sizeof(struct finder_pattern_list*));
    if (array == NULL) {
        return NULL;
    }

    for (unsigned int i = 0 ; i < n ; i++, list = list->next) {
        array[i] = list;
    }

    qsort(array, n, sizeof(struct finder_pattern_list*), (int (*)(const void *, const void *))compare_module_sizes);

    return array;
}


/**
 * Returns the distance in pixels between the given points.
 */
static float get_distance(struct finder_pattern* a, struct finder_pattern* b) {
    float x_diff = a->x - b->x;
    float y_diff = a->y - b->y;
    return sqrtf(x_diff * x_diff + y_diff * y_diff);
}


/**
 * Checks if the 3 given points can form a valid finder pattern group.
 * If so, adds the group to the given list.
 */
static void check_points(struct finder_pattern* p1, struct finder_pattern* p2, struct finder_pattern* p3,
                        struct finder_pattern_group_list* *groups) {
    float distance_1_2 = get_distance(p1, p2);
    float distance_1_3 = get_distance(p1, p3);
    float distance_2_3 = get_distance(p2, p3);

    // First, let's sort the points as A B C so that AC is the biggest distance and B
    // is the candidate for being the top left corner
    struct finder_pattern* a;
    struct finder_pattern* b;
    struct finder_pattern* c;

    float distance_AB;
    float distance_BC;
    float distance_AC;

    if (distance_1_3 >= distance_1_2 && distance_1_3 >= distance_2_3) {
        // If p1 -> p3 is the biggest, p2 is B
        a = p1;
        b = p2;
        c = p3;

        distance_AB = distance_1_2;
        distance_BC = distance_2_3;
        distance_AC = distance_1_3;
    }
    else if (distance_2_3 >= distance_1_2 && distance_2_3 >= distance_1_3) {
        // If p2 -> p3 is the biggest, p1 is B
        a = p2;
        b = p1;
        c = p3;

        distance_AB = distance_1_2;
        distance_BC = distance_1_3;
        distance_AC = distance_2_3;
    } else {
        a = p1;
        b = p3;
        c = p2;

        distance_AB = distance_1_3;
        distance_BC = distance_2_3;
        distance_AC = distance_1_2;
    }

    // At this point, we know B, but we can have either:
    //
    // 0,0
    //  +-------------------> x axis
    //  |
    //  |  B-C  or  B-A
    //  |  |        |
    //  |  A        C
    //  |
    //  V
    //  y axis
    //
    // Since this is the first configuration that we want, we may need
    // to swap A and C. To figure this out, we need to calculate the cross
    // product between the vectors A->B and B->C and look at the resulting
    // z component. The formula to calculate the cross product between 2
    // vectors is:
    //
    // -->   -->     Ux     Vx     Uy.Vz - Uz.Vy
    //  U  ^  V   =  Uy  ^  Vy  =  Uz.Vx - Ux.Vz
    //               Uz     Vz     Ux.Vy - Uy.Vx
    //
    // Since we consider our vectors are in the same plane, we can set Uz = Vz = 0 so:
    //
    // -->   -->     0
    //  U  ^  V   =  0
    //               Ux.Vy - Uy.Vx
    //
    //      ->  -->    ->  -->
    // With AB = U and BC = V, that gives us:
    //
    // z = (Xb - Xa).(Yc - Yb) - (Yb - Ya).(Xc - Xb)
    //
    // If z is positive, we are in the configuration we want. If it is negative,
    // we need to swap A and C.

    float z = (b->x - a->x) * (c->y - b->y) - (b->y - a->y) * (c->x - b->x);
    if (z < 0) {
        struct finder_pattern* tmp = a;
        a = c;
        c = tmp;
    }

    // Now that we know A, B and C, let's verify that the distances AB and BC
    // are about the same
    float delta = fabs(distance_AB - distance_BC) / fmin(distance_AB, distance_BC);
    if (delta > 0.1f) {
        return;
    }

    // Now, it ABC is a valid group, the angle ABC must be about 90° so let's
    // check it by verifying it with Pythagoras' theorem
    float pyth_AC = sqrtf(distance_AB * distance_AB + distance_BC * distance_BC);
    float delta_AC = fabs(distance_AC - pyth_AC) / fmin(distance_AC, pyth_AC);
    if (delta_AC > 0.1f) {
        return;
    }

    // Ok ABC is an isosceles rectangle triangle, is it the right size ?
    float estimated_module_count = (distance_AB + distance_BC) / (b->module_size * 2.0f);
    if (estimated_module_count < MIN_MODULES_PER_EDGE || estimated_module_count > MAX_MODULES_PER_EDGE) {
        return;
    }

    // We have a match
    struct finder_pattern_group_list* match = (struct finder_pattern_group_list*)malloc(sizeof(struct finder_pattern_group_list));
    if (match == NULL) {
        return;
    }
    match->bottom_left = (*a);
    match->top_left = (*b);
    match->top_right = (*c);
    match->next = (*groups);
    (*groups) = match;
}


struct finder_pattern_group_list* find_groups(struct finder_pattern_list* list) {
    unsigned int n = get_list_size(list);
    if (n < 3) {
        // We need at list 3 finder patterns to have a match
        return NULL;
    }

    // Let's start by sorting the finder patterns by module sizes
    struct finder_pattern_list** sorted_array = create_sorted_array(n, list);
    if (sorted_array == NULL) {
        return NULL;
    }

    struct finder_pattern_group_list* groups = NULL;

    for (unsigned int i = 0 ; i < n - 2 ; i++) {
        struct finder_pattern* p1 = &(sorted_array[i]->pattern);

        for (unsigned int j = i + 1 ; j < n - 1 ; j++) {
            struct finder_pattern* p2 = &(sorted_array[j]->pattern);

            if (fabs(p1->module_size - p2->module_size) > MAX_MODULE_SIZE_DIFF) {
                // If the module sizes differ too much, we can stop here since the
                // points are sorted by increasing module size
                break;
            }

            for (unsigned int k = j + 1 ; k < n ; k++) {
                struct finder_pattern* p3 = &(sorted_array[k]->pattern);

                if (fabs(p2->module_size - p3->module_size) > MAX_MODULE_SIZE_DIFF) {
                    // Again, if the sizes differ too much, we can stop
                    break;
                }

                check_points(p1, p2, p3, &groups);
            }
        }
    }

    free(sorted_array);
    return groups;
}


void free_finder_pattern_group_list(struct finder_pattern_group_list* list) {
    struct finder_pattern_group_list* tmp;
    while (list != NULL) {
        tmp = list->next;
        free(list);
        list = tmp;
    }
}
