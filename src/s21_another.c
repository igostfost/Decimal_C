#include "s21_decimal.h"

// Возвращает результат умножения указанного Decimal на -1.
int s21_negate(s21_decimal value, s21_decimal *result) {
  int sign = get_bit(value, 127);
  *result = value;
  if (sign == 1) {
    set_bit(result, 127, 0);

  } else {
    set_bit(result, 127, 1);
  }
  return 0;
}

// Возвращает целые цифры указанного Decimal числа; любые дробные цифры
// отбрасываются, включая конечные нули.
int s21_truncate(s21_decimal value, s21_decimal *result) {
  // s21_decimal result = create_decimal(0,0,0,0,0);
  result->bits[0] = result->bits[1] = result->bits[2] = result->bits[3] = 0;
  int flag = 0;
  float diff = 0;
  if (get_scale(value)) {
    double doub_from_dec = 0.0;
    s21_from_decimal_to_double(value, &doub_from_dec);
    // отбрасываем дробную часть
    doub_from_dec = trunc(doub_from_dec);
    if (doub_from_dec < 0.0) {
      set_bit(result, 127, 1);

      doub_from_dec = doub_from_dec * (-1);
    }

    for (int i = 0; doub_from_dec >= 1 && i < 96; i++) {
      // floor - округляет до наибольшего целого, которое меньше или равно
      // нашему числу.
      doub_from_dec = floor(doub_from_dec) / 2;
      diff = doub_from_dec - floor(doub_from_dec);
      if (diff > 0.0) {
        set_bit(result, i, 1);
      } else {
        set_bit(result, i, 0);
      }
    }

    set_scale(result, 0);
  } else {
    for (int i = 0; i < 4; ++i) {
      result->bits[i] = value.bits[i];
    }
  }

  return flag;
}

// Округляет Decimal до ближайшего целого числа.
int s21_round(s21_decimal value, s21_decimal *result) {
  result->bits[0] = result->bits[1] = result->bits[2] = result->bits[3] = 0;
  double doub_from_dec = 0.0;
  double diff = 0;
  s21_from_decimal_to_double(value, &doub_from_dec);
  doub_from_dec = round(doub_from_dec);
  if (doub_from_dec < 0.0) {
    set_bit(result, 127, 1);

    doub_from_dec *= -1;
  }
  for (int i = 0; doub_from_dec >= 1 && i < 96; i++) {
    doub_from_dec = floor(doub_from_dec) / 2;
    diff = doub_from_dec - floor(doub_from_dec);
    if (diff > 0.0) {
      set_bit(result, i, 1);
    } else {
      set_bit(result, i, 0);
    }
  }
  set_scale(result, 0);
  return 0;
}

int s21_floor(s21_decimal value, s21_decimal *result) {
  result->bits[0] = result->bits[1] = result->bits[2] = result->bits[3] = 0;
  s21_decimal one = {{1, 0, 0, 0}};
  s21_truncate(value, result);
  if (get_bit(value, 127) && !s21_is_equal(*result, value)) {
    s21_sub(*result, one, result);
  }
  return 0;
}
