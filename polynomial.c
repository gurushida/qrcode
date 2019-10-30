#include <stdlib.h>
#include <string.h>
#include "galoisfield.h"
#include "polynomial.h"

u_int8_t evaluate_polynomial(struct gf_polynomial* p, u_int8_t x) {
    unsigned int n = p->n_coefficients;
    u_int8_t result = p->coefficients[n - 1];
    u_int8_t current = x;
    for (unsigned int i = 1 ; i < n ; i++) {
        uint8_t r = p->coefficients[n - 1 - i];
        u_int8_t tmp = gf_multiply(r, current);
        result = gf_add_or_subtract(result, tmp);
        current = gf_multiply(current, x);
    }
    return result;
}


struct gf_polynomial* new_gf_polynomial(unsigned int n_coefficients, u_int8_t* coefficients) {
    struct gf_polynomial* p = (struct gf_polynomial*)malloc(sizeof(struct gf_polynomial));
    if (p == NULL) {
        return NULL;
    }
    p->coefficients = (u_int8_t*)calloc(n_coefficients, sizeof(u_int8_t));
    if (p->coefficients == NULL) {
        free(p);
        return NULL;
    }
    if (coefficients != NULL) {
        memmove(p->coefficients, coefficients, n_coefficients * sizeof(u_int8_t));
    }
    p->n_coefficients = n_coefficients;
    return p;
}


void free_gf_polynomial(struct gf_polynomial* p) {
    free(p->coefficients);
    free(p);
}
