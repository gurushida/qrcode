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


struct gf_polynomial* add_polynomials(struct gf_polynomial* a, struct gf_polynomial* b) {
    unsigned int degree_a = get_degree(a);
    unsigned int degree_b = get_degree(b);

    if (degree_a < degree_b) {
        // Let's make sure that a is the one with the higher degree
        struct gf_polynomial* tmp = a;
        a = b;
        b = tmp;
    }

    struct gf_polynomial* res = new_gf_polynomial(degree_a + 1, NULL);
    if (res == NULL) {
        return NULL;
    }

    for (unsigned int i = 0 ; i <= degree_a ; i++) {
        uint8_t value = gf_add_or_subtract(get_coefficient(a, i), get_coefficient(b, i));
        set_coefficient(res, i, value);
    }

    return res;
}


struct gf_polynomial* multiply_polynomials(struct gf_polynomial* a, struct gf_polynomial* b) {
    unsigned int degree_a = get_degree(a);
    unsigned int degree_b = get_degree(b);

    struct gf_polynomial* res = new_gf_polynomial(degree_a + degree_b + 1, NULL);
    if (res == NULL) {
        return NULL;
    }

    for (unsigned int i = 0 ; i <= degree_a ; i++) {
        uint8_t v_a = a->coefficients[i];
        for (unsigned int j = 0 ; j <= degree_b ; j++) {
            u_int8_t value = res->coefficients[i + j];
            uint8_t v_b = b->coefficients[j];
            res->coefficients[i + j] = gf_add_or_subtract(value, gf_multiply(v_a, v_b));
        }
    }

    return res;
}


unsigned int get_degree(struct gf_polynomial* a) {
    unsigned int i = 0;
    while (i < a->n_coefficients && a->coefficients[i] == 0) {
        i++;
    }
    if (i == a->n_coefficients) {
        return 0;
    }
    return a->n_coefficients - 1 - i;
}


u_int8_t get_coefficient(struct gf_polynomial* a, unsigned int degree) {
    if (degree >= a->n_coefficients) {
        return 0;
    }
    return a->coefficients[a->n_coefficients - 1 - degree];
}


int set_coefficient(struct gf_polynomial* a, unsigned int degree, u_int8_t coefficient) {
    if (degree >= a->n_coefficients) {
        return 0;
    }
    a->coefficients[a->n_coefficients - 1 - degree] = coefficient;
    return 1;
}


int equal_polynomials(struct gf_polynomial* a, struct gf_polynomial* b) {
    unsigned int degree_a = get_degree(a);
    unsigned int degree_b = get_degree(b);
    if (degree_a != degree_b) {
        return 0;
    }

    for (unsigned int i = 0 ; i <= degree_a ; i++) {
        if (get_coefficient(a, i) != get_coefficient(b, i)) {
            return 0;
        }
    }

    return 1;
}
