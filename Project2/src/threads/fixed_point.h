#ifndef FIXED_POINT_ARITHMETIC
#define FIXED_POINT_ARITHMETIC

/** Definition: **/

// Typename, used for the fixed point arithmetic
typedef int fixed_point;

// Number of bits after "dot"
#define FRACTION_BITS 14





/** Conversions: **/

// Conversion from int to fixed_point
#define int_to_fixed(value) ((value) << FRACTION_BITS)

// Conversion from fixed_point to int
#define fixed_to_int(value) ((value) >> FRACTION_BITS)

// Conversion from fixed_point to int (rounded to closest integer)
#define fixed_round_to_closest_int(value) (((value) + (((value > 0) ? 1 : (-1)) << (FRACTION_BITS - 1))) >> FRACTION_BITS)





/** Operators: **/


/* Sum: */

// Sum of fixed_point variables
#define fixed_sum(a, b) ((a) + (b))

// Sum of a fixed_point and an integer (first parameter is fixed_point)
#define fixed_int_sum(f, i) ((f) + int_to_fixed(i))

// Sum of an integer and a fixed_point (first parameter is int)
#define int_fixed_sum(i, f) fixed_int_sum(f, i)


/* Subtraction: */

// Subtraction of fixed_point variables
#define fixed_sub(a, b) ((a) - (b))

// fixed_point subtracted by an integer
#define fixed_int_sub(f, i) ((f) - int_to_fixed(i))

// integer subtracted by a fixed_point
#define int_fixed_sub(i, f) (int_to_fixed(i) - (f))


/* Multiplication: */

// Product of two fixed_point variables
#define fixed_mul(a, b) ((fixed_point)((((int64_t)(a)) * ((int64_t)(b))) >> FRACTION_BITS))

// Product of a fixed_point and an integer
#define fixed_int_mul(f, i) ((f) * (i))

// Product of an integer and a fixed point
#define int_fixed_mul(i, f) ((i) * (f))


/* Division: */

// Division of one fixed_point by another
#define fixed_div(a, b) ((fixed_point)((((int64_t)(a)) << FRACTION_BITS) / ((int64_t)(b))))

// Division of a fixed_point by an integer
#define fixed_int_div(f, i) ((f) / (i))

// Division of an integer by a fixed_point (produces an integer)
#define int_fixed_div(i, f) fixed_div(i, f)





#endif
