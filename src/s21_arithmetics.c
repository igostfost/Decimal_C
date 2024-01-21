#include "s21_decimal.h"

// returns 1 is dec is negative
int is_neg(s21_decimal num) { return get_bit(num, 127); }

s21_decimal invert(s21_decimal num) {
  s21_decimal res = {0};
  for (int i = 0; i < 3; i++) {
    res.bits[i] = ~num.bits[i];
  }
  res.bits[0] += 1;
  return res;
}

// returns 1 if addition is not possible (overflow)
int simple_add_with_check(s21_decimal a, s21_decimal b, s21_decimal *sum) {
  int extra, x;
  extra = x = 0;
  for (int i = 0; i < 96; i++) {
    x = get_bit(a, i) + get_bit(b, i) + extra;
    if (x < 2) {
      extra = 0;
    } else if (x == 2) {
      extra = 1;
      x = 0;
    } else if (x == 3) {
      extra = 1;
      x = 1;
    }
    set_bit(sum, i, x);
  }
  return !(decimal_ok(*sum));
}

// returns 1 if d1 mantissa > d2 mantissa else 0
int mantissa_bigger(s21_decimal d1, s21_decimal d2) {
  int res = 1;
  for (int i = 2; i >= 0 && res; i--) {
    if (d1.bits[i] < d2.bits[i]) {
      res = 0;
    }
  }
  return res;
}

int is_zero(s21_decimal num) {
  int res = 1;
  for (int i = 0; i < 3 && res; i++) {
    if (num.bits[i]) res = 0;
  }
  return res;
}

/*
0 - OK
1 - the number is too large or equal to infinity
2 - the number is too small or equal to negative infinity
*/
int s21_sub(s21_decimal dec_1, s21_decimal dec_2, s21_decimal *result) {
  int flag = 0;
  clear_decimal(result);
  s21_decimal a = copy_decimal(dec_1);
  s21_decimal b = copy_decimal(dec_2);
  normalize(&a, &b);
  int sign_a = is_neg(a);
  int sign_b = is_neg(b);
  s21_decimal inverted = {0};
  int a_bigger = mantissa_bigger(a, b);

  if ((is_zero(a) && is_zero(b)) || is_zero(b)) {
    *result = copy_decimal(a);
  } else if (is_zero(a)) {
    s21_negate(b, result);
  } else if (!sign_a && !sign_b && a_bigger) {  // +a, +b, a > b
    inverted = invert(b);
    flag = simple_add_with_check(a, inverted, result);
  } else if (!sign_a && !sign_b && !a_bigger) {  // +a, +b, a < b
    inverted = invert(a);
    flag = simple_add_with_check(b, inverted, result);
    set_bit(result, 127, 1);
  } else if (!sign_a && sign_b) {  // +a, -b
    flag = simple_add_with_check(a, b, result);
  } else if (sign_a && !sign_b) {  // -a, +b
    flag = simple_add_with_check(a, b, result);
    set_bit(result, 127, 1);
  } else if (sign_a && sign_b && !a_bigger) {  // -a, -b, a < b
    inverted = invert(a);
    flag = simple_add_with_check(b, inverted, result);
  } else if (sign_a && sign_b && a_bigger) {  // -a, -b, a > b
    inverted = invert(b);
    flag = simple_add_with_check(a, inverted, result);
    set_bit(result, 127, 1);
  }
  int res_scale = is_zero(a) ? get_scale(b) : get_scale(a);
  set_scale(result, res_scale);
  return flag;
}

big_decimal dec_to_big(s21_decimal dec) {
  big_decimal res = {0};
  for (int i = 0; i < 3; i++) {
    res.bits[i] = dec.bits[i] & MAXBITS;
  }
  res.scale = get_scale(dec);
  res.sign = is_neg(dec);
  return res;
}

s21_decimal big_to_dec(big_decimal big) {
  s21_decimal res = {0};
  for (int i = 0; i < 3; i++) {
    res.bits[i] = big.bits[i] & MAXBITS;
  }
  set_scale(&res, big.scale);
  set_bit(&res, 127, big.sign);
  return res;
}

// void print_big_binary(uint64_t x) {
//   for (int i = 63; i >= 0; i--) {  // iterate over each bit
//     if ((x >> i) & 1) {            // check if the bit is set
//       printf("1");
//     } else {
//       printf("0");
//     }
//     if (!(i % 4)) printf(" ");
//     if (!(i % 32)) printf(" ");
//   }
//   printf("\n");
// }

// void print_big(big_decimal num) {
//   char sign = num.sign ? '-' : '+';
//   printf("sign: %c\nscale: %u\n", sign, num.scale);
//   for (int i = 6; i >= 0; i--) {
//     printf("bit[%d]: ", i);
//     print_big_binary(num.bits[i]);
//   }
// }

int is_big_zero(big_decimal num) {
  int res = 1;
  for (int i = 0; i < 6 && res; i++) {
    if (num.bits[i]) res = 0;
  }
  return res;
}

int get_overflow(big_decimal *num) {
  uint64_t overflow = 0;
  int flag = 0;
  for (int i = 0; i < 7; i++) {
    num->bits[i] += overflow;
    overflow = num->bits[i] >> 32;
    num->bits[i] &= MAXBITS;
  }
  if (!is_big_ok(*num)) flag = 1;

  return flag;
}

int is_big_ok(big_decimal num) {
  int res = 1;
  for (int i = 6; i >= 3 && res; i--) {
    if (num.bits[i]) res = 0;
  }
  return res;
}

uint64_t right_shift(big_decimal *num) {
  uint64_t remainder = 0;
  for (int i = 6; i >= 0; i--) {
    num->bits[i] += remainder << 32;
    remainder = num->bits[i] % 10;
    num->bits[i] /= 10;
  }
  num->scale--;
  return remainder;
}

int scale_down(big_decimal *num, uint64_t limit) {
  int too_small = 0;
  uint64_t last_rem = 0, was_rem = 0;
  while (num->scale > limit && !too_small) {
    last_rem = right_shift(num);
    if (last_rem) was_rem++;
    if (is_big_zero(*num)) too_small = 1;
  }

  // rounding
  if (last_rem > 5 || (last_rem == 5 && (was_rem > 1 || (num->bits[0] % 2)))) {
    num->bits[0]++;
  }
  return too_small;
}

/* returns
0 - OK
1 - the number is too large or equal to infinity
2 - the number is too small or equal to negative infinity
*/
int s21_mul(s21_decimal a, s21_decimal b, s21_decimal *result) {
  int flag = 0;
  clear_decimal(result);
  if (!is_zero(a) && !is_zero(b)) {
    int res_sign = (is_neg(a) == is_neg(b)) ? 0 : 1;
    big_decimal a_big = dec_to_big(a);
    big_decimal b_big = dec_to_big(b);
    big_decimal res_big = {0};

    res_big.bits[0] = a_big.bits[0] * b_big.bits[0];
    res_big.bits[1] =
        a_big.bits[1] * b_big.bits[0] + a_big.bits[0] * b_big.bits[1];
    res_big.bits[2] = a_big.bits[0] * b_big.bits[2] +
                      a_big.bits[1] * b_big.bits[1] +
                      a_big.bits[2] * b_big.bits[0];
    res_big.bits[3] =
        a_big.bits[1] * b_big.bits[2] + a_big.bits[2] * b_big.bits[1];
    res_big.bits[4] = a_big.bits[2] * b_big.bits[2];

    res_big.scale = get_scale(a) + get_scale(b);
    int too_big = get_overflow(&res_big);
    int too_small = scale_down(&res_big, 28);
    if (too_big) {
      scale_down(&res_big, 0);
      if (!is_big_ok(res_big)) flag = res_sign ? 2 : 1;
    } else if (too_small) {
      flag = 2;
    }
    if (!flag) {
      *result = big_to_dec(res_big);
      set_bit(result, 127, res_sign);
    }
  }
  return flag;
}

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int res = 0;

  int sign_value_1 = get_bit(value_1, 127);
  int sign_value_2 = get_bit(value_2, 127);

  if ((get_scale(value_1) || get_scale(value_2))) {
    normalize(&value_1, &value_2);
  }
  if (sign_value_1 == sign_value_2) {
    simple_add_with_check(value_1, value_2, result);
  }
  set_scale(result, get_scale(value_1));
  if (sign_value_1 && sign_value_2 && !res) {
    set_minus(result);
  }
  if (sign_value_1 == 0 && sign_value_2 == 1 && !res) {
    set_bit(&value_2, 127, 0);
    s21_sub(value_1, value_2, result);
  } else if (sign_value_1 == 1 && sign_value_2 == 0 && !res) {
    set_bit(&value_1, 127, 0);
    s21_sub(value_2, value_1, result);
  }

  if (is_zero(value_1) && sign_value_2 == 0 && !res) {
    result->bits[3] = value_2.bits[3];
  } else if (is_zero(value_2) && sign_value_1 == 0 && !res) {
    result->bits[3] = value_1.bits[3];
  }
  return res;
}

int s21_div(s21_decimal a, s21_decimal b, s21_decimal *result) {
  int flag = 0;
  clear_decimal(result);
  if (is_zero(b))
    flag = 3;
  else if (!is_zero(a)) {
    int res_sign = (is_neg(a) == is_neg(b)) ? 0 : 1;
    int res_scale = get_scale(a) - get_scale(b);

    set_bit(result, 127, res_sign);
    set_scale(result, res_scale);

    // divide
  }
  return flag;
}
