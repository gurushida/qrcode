#ifndef _FINDERPATTERNGROUP_H
#define _FINDERPATTERNGROUP_H

#include "errors.h"
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
 *
 * @param list The finder patterns
 * @param groups Where to store the groups
 * @return SUCCESS if some groups are found
 *         DECODING_ERROR if no group is found
 *         MEMORY_ERROR in case of memory allocation error
 */
int find_groups(struct finder_pattern_list* list, struct finder_pattern_group_list* *groups);


/**
 * Frees all the memory associated to the given list.
 */
void free_finder_pattern_group_list(struct finder_pattern_group_list* list);


#endif

