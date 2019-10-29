#include "galoisfield.h"
#include "polynomial.h"

u_int8_t evaluate_polynomial(u_int8_t* coefficients, unsigned int n, u_int8_t x) {
    u_int8_t result = coefficients[n - 1];
    u_int8_t current = x;
    for (unsigned int i = 1 ; i < n ; i++) {
        uint8_t r = coefficients[n - 1 - i];
        u_int8_t tmp = gf_multiply(r, current);
        result = gf_add_or_subtract(result, tmp);
        current = gf_multiply(current, x);
    }
    return result;
}
