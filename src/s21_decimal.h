#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBITS 0xffffffff
#define MINUS 0x80000000  // в двоичной 10000000 00000000 00000000 00000000

typedef struct {
  unsigned int bits[4];
} s21_decimal;

typedef struct {
  uint64_t bits[7];
  uint16_t scale;
  uint8_t sign;
} big_decimal;

typedef union {  // разделяют одну и ту же область памяти
  int int_bits;
  float float_bits;
} floatbits;

// 0 - OK
// 1 - число слишком велико или равно бесконечности
// 2 - число слишком мало или равно отрицательной бесконечности
// 3 - деление на 0
int s21_add(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // +
int s21_sub(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // -
int s21_mul(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // *
int s21_div(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // /

// 0 - FALSE; 1 - TRUE
int s21_is_less(s21_decimal, s21_decimal);              // <
int s21_is_less_or_equal(s21_decimal, s21_decimal);     // <=
int s21_is_greater(s21_decimal, s21_decimal);           // >
int s21_is_greater_or_equal(s21_decimal, s21_decimal);  //>=
int s21_is_equal(s21_decimal, s21_decimal);             // ==
int s21_is_not_equal(s21_decimal, s21_decimal);         //!=

// 0 - OK; 1 - ошибка конвертации
int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

// 0 - OK; 1 - ошибка вычисления
int s21_floor(s21_decimal value,
              s21_decimal *result);  // Округление до ближайшего целого в
                                     // сторону отрицательной бесконечности
int s21_round(s21_decimal value,
              s21_decimal *result);  // Округление до ближайшего целого
int s21_truncate(s21_decimal value,
                 s21_decimal *result);  // Возвращает целые цифры
int s21_negate(s21_decimal value,
               s21_decimal *result);  // Decimal умноженное на -1.

// supporting functions
s21_decimal create_decimal(int bit0, int bit1, int bit2, int scale, int minus);
int get_bit(s21_decimal number, int index);
void set_bit(s21_decimal *number, int index, int value);
void set_minus(s21_decimal *number);
// void print_binary(int x);
// void print_decimal(s21_decimal number);
int get_scale(s21_decimal number);
void set_scale(s21_decimal *number, int scale);
void clear_decimal(s21_decimal *num);
void normalize(s21_decimal *a, s21_decimal *b);
s21_decimal copy_decimal(s21_decimal number);
void simple_add(s21_decimal *a, s21_decimal b);
int left_shift(s21_decimal *number, int step);
s21_decimal multiply_mantissa(s21_decimal num, int value);
int simple_add_with_check(s21_decimal a, s21_decimal b, s21_decimal *sum);
s21_decimal invert(s21_decimal num);
int s21_comparison_mantissa(s21_decimal d1, s21_decimal d2);
int mantissa_bigger(s21_decimal d1, s21_decimal d2);
int is_neg(s21_decimal num);
int decimal_ok(s21_decimal number);
int is_zero(s21_decimal num);
int s21_from_decimal_to_double(s21_decimal src, double *dst);

s21_decimal create_decimal(int bit0, int bit1, int bit2, int minus, int scale);
void normalize(s21_decimal *a, s21_decimal *b);
big_decimal dec_to_big(s21_decimal dec);
s21_decimal big_to_dec(big_decimal big);
// void print_big_binary(uint64_t x);
// void print_big(big_decimal num);
int get_overflow(big_decimal *num);
int is_big_ok(big_decimal num);
int scale_down(big_decimal *num, uint64_t limit);
uint64_t right_shift(big_decimal *num);
int is_big_zero(big_decimal num);

#endif  // SRC_S21_DECIMAL_H_
