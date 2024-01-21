#include "s21_decimal.h"

s21_decimal create_decimal(int bit0, int bit1, int bit2, int scale, int minus) {
  s21_decimal number = {0};
  number.bits[0] = bit0;
  number.bits[1] = bit1;
  number.bits[2] = bit2;
  if (minus) set_minus(&number);
  if (scale) set_scale(&number, scale);
  return number;
}

void set_scale(s21_decimal *number, int scale) {
  int minus = get_bit(*number, 127);
  number->bits[3] = scale << 16;
  if (minus) set_minus(number);
}

int get_scale(s21_decimal number) {
  int result = 0;
  int pow = 1;  // Начальное значение степени 2

  for (int i = 16; i < 24; i++) {
    int bit = get_bit(number, i + 3 * 32);
    result = result + pow * bit;
    pow = pow * 2;  // Умножаем текущую степень 2 на 2
  }
  return result;
}

void set_minus(s21_decimal *number) { set_bit(number, 127, 1); }

int get_bit(s21_decimal number, int index) {
  int mask = 1;
  mask = mask << index % 32;
  int res = mask & number.bits[index / 32];
  return res ? 1 : 0;
}

void set_bit(s21_decimal *number, int index, int value) {
  unsigned int mask = 1;
  mask = mask << index % 32;
  if (value) {
    number->bits[index / 32] = number->bits[index / 32] | mask;
  } else {
    mask = ~mask;
    number->bits[index / 32] = number->bits[index / 32] & mask;
  }
}

// void print_decimal(s21_decimal number) {
//   for (int i = 3; i >= 0; i--) {
//     // printf("bit[%d]: ", i);  // fix
//     print_binary(number.bits[i]);
//     printf("\n");
//   }
// }

s21_decimal copy_decimal(s21_decimal number) {
  s21_decimal result;
  for (int i = 0; i < 4; i++) result.bits[i] = number.bits[i];
  return result;
}

int decimal_ok(s21_decimal number) {
  int res = 1;

  for (int i = 96; i < 112 && res; i++) {
    if (get_bit(number, i)) res = 0;
  }
  for (int i = 117; i < 127 && res; i++) {
    if (get_bit(number, i)) res = 0;
  }
  if (get_scale(number) > 28) res = 0;

  return res;
}

void normalize(s21_decimal *a, s21_decimal *b) {
  int scale_a = get_scale(*a);
  int scale_b = get_scale(*b);
  int difference = scale_a - scale_b;
  s21_decimal res = {0};

  if (!is_zero(*a) && !is_zero(*b)) {
    if (difference > 0) {  // a > b
      res = copy_decimal(*b);
      while (difference && scale_b < 28) {
        res = multiply_mantissa(res, 10);
        scale_b++;
        difference--;
      }
      set_scale(&res, scale_b);
      *b = copy_decimal(res);
    } else if (difference < 0 && scale_a < 28) {  // a < b
      res = copy_decimal(*a);
      while (difference) {
        res = multiply_mantissa(res, 10);
        scale_a++;
        difference++;
      }
      set_scale(&res, scale_a);
      *a = copy_decimal(res);
    }
  }
}

// 1 =  a equal or less than b
// int mantissa_less_or_eq(s21_decimal a, s21_decimal b) {
//   int res = 1;
//   for (int i = 2; i >= 0 && res; i--) {
//     if (a.bits[i] > b.bits[i]) res = 0;
//   }
//   return res;
// }

void clear_decimal(s21_decimal *num) {
  for (int i = 0; i < 4; i++) {
    num->bits[i] = 0;
  }
}

s21_decimal multiply_mantissa(s21_decimal num, int value) {
  s21_decimal res = {0};
  s21_decimal x = {0};
  for (int i = 0; i < 96; i++) {
    x.bits[0] = value;
    if (get_bit(num, i)) {
      left_shift(&x, i);
      simple_add(&res, x);
    }
    clear_decimal(&x);
  }
  return res;
}

void simple_add(s21_decimal *a, s21_decimal b) {
  int extra = 0;
  int x = 0;
  for (int i = 0; i < 96; i++) {
    x = get_bit(*a, i) + get_bit(b, i) + extra;
    if (x < 2) {
      extra = 0;
    } else if (x == 2) {
      extra = 1;
      x = 0;
    } else if (x == 3) {
      extra = 1;
      x = 1;
    }
    set_bit(a, i, x);
  }
}

// if couldnt shift return 1 else 0
int left_shift(s21_decimal *number, int step) {
  int check = 0;
  for (int i = 95; i > 95 - step && !check; i--) {
    if (get_bit(*number, i)) {
      check = 1;
    }
  }
  if (!check) {
    for (int i = 95 - step; i >= 0; i--) {
      int value = get_bit(*number, i);
      set_bit(number, i + step, value);
    }
    for (int i = 0; i < step; i++) {
      set_bit(number, i, 0);
    }
  }
  return check;
}
