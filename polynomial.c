#include <stdio.h>
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
        unsigned int n = degree_a;
        degree_a = degree_b;
        degree_b = n;
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
        uint8_t v_a = get_coefficient(a, degree_a - i);
        for (unsigned int j = 0 ; j <= degree_b ; j++) {
            u_int8_t value = res->coefficients[i + j];
            uint8_t v_b = get_coefficient(b, degree_b - j);
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


int divide_polynomials(struct gf_polynomial* a, struct gf_polynomial* b,
                        struct gf_polynomial* *quotient, struct gf_polynomial* *remainder) {
    unsigned int degree_a = get_degree(a);
    unsigned int degree_b = get_degree(b);
    if (0 == get_degree(b) && 0 == get_coefficient(b, 0)) {
        // We cannot divide by 0
        return -1;
    }
    if (degree_a < degree_b) {
        return 0;
    }

    // This implementation uses the extended synthetic division.
    // It works in place in an array the same size as a
    // which will eventually contain the coefficiients of both the
    // quotient and remainder in the following order:
    //
    // q(n) q(n-1) ... q(0) r(m) ... r(0)
    //
    // where q(i) is the quotient coefficient for degree i.
    u_int8_t* tmp = (u_int8_t*)malloc((degree_a + 1) * sizeof(u_int8_t));
    if (tmp == NULL) {
        return -2;
    }

    // Let's initialize the array by copying a into it
    for (unsigned int i = 0 ; i <= degree_a ; i++) {
        tmp[i] = get_coefficient(a, degree_a - i);
    }

    for (unsigned int i = 0 ; i < (degree_a - degree_b + 1) ; i++) {
        uint8_t coeff = tmp[i];
        if (coeff != 0) {
            for (unsigned int j = 1 ; j <= degree_b ; j++) {
                u_int8_t divisor_b = get_coefficient(b, j);
                if (divisor_b != 0) {
                    tmp[i + j] = gf_add_or_subtract(tmp[i + j], gf_multiply(divisor_b, coeff));
                }
            }
        }
    }

    // When we are done, we construct the quotient and remainder from the array
    *quotient = new_gf_polynomial(degree_a, tmp);
    if ((*quotient) == NULL) {
        free(tmp);
        return -2;
    }
    *remainder = new_gf_polynomial(degree_b, tmp + degree_a);
    if ((*remainder) == NULL) {
        free(*quotient);
        free(tmp);
        return -2;
    }

    free(tmp);
    return 1;
}


void poly_print(char* name, struct gf_polynomial* p) {
    printf("%s (d=%d):", name, get_degree(p));
    if (is_zero_polynomial(p)) {
        printf("0\n");
        return;
    }
    int first = 1;
    for (int i = get_degree(p) ; i >= 0 ; i--) {
        u_int8_t v = get_coefficient(p, i);
        if (v != 0) {
            if (first) {
                first = 0;
            } else {
                printf(" +");
            }
            if (v != 1) {
                printf(" a^%d", gf_log(v));
                if (i > 0) {
                    printf(".");
                }
            } else if (i == 0) {
                printf(" 1");
            }
            if (i == 1) {
                printf("X");
            } else if (i > 1) {
                printf("X^%d", i);
            }
        }
    }
    printf("\n");
}


int is_zero_polynomial(struct gf_polynomial* p) {
    for (unsigned int i = 0 ; i < p->n_coefficients ; i++) {
        if (p->coefficients[i]) {
            return 0;
        }
    }
    return 1;
}

struct gf_polynomial* get_monomial(unsigned int degree, u_int8_t coefficient) {
    struct gf_polynomial* p = new_gf_polynomial(degree + 1, NULL);
    if (p == NULL) {
        return NULL;
    }
    set_coefficient(p, degree, coefficient);
    return p;
}


void multiply_by_scalar(struct gf_polynomial* p, u_int8_t scalar) {
    for (unsigned int i = 0 ; i < p->n_coefficients ; i++) {
        p->coefficients[i] = gf_multiply(p->coefficients[i], scalar);
    }
}
