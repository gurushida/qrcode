#ifndef _FINDERPATTERNGROUP_H
#define _FINDERPATTERNGROUP_H

#include "finderpattern.h"

/**
 * This structure is used to represent a list of potential QR codes
 * where each candidate is represented by a group of 3 finder patterns
 * that may be the corners of a QR code.
 */
struct finder_pattern_group_list {
    struct finder_pattern bottom_left;
    struct finder_pattern top_left;
    struct finder_pattern top_right;

    struct finder_pattern_group_list* next;
};


/**
 * Given a list a finder patterns, this function tries
 * to identifies triplets of finder patterns that could
 * be the corners of QA code candidates. A finder pattern
 * may appear in zero, one or multiple groups. We will let
 * the QR code decoding process eliminate the false positives.
 */
struct finder_pattern_group_list* find_groups(struct finder_pattern_list* list);


/**
 * Frees all the memory associated to the given list.
 */
void free_finder_pattern_group_list(struct finder_pattern_group_list* list);


#endif

