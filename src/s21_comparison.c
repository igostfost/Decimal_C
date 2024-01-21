#include "s21_decimal.h"

int s21_is_equal(s21_decimal val_1, s21_decimal val_2) {
  int flag = 1;
  if (get_bit(val_1, 127) != get_bit(val_2, 127)) {
    flag = 0;
  } else {
    if ((get_scale(val_1) - get_scale(val_2))) {
      normalize(&val_1, &val_2);
    }
    int tmp_flag = s21_comparison_mantissa(val_1, val_2);
    if (tmp_flag == 0) {
      flag = 1;
    } else {
      flag = 0;
    }
  }

  return flag;
}

// !=
int s21_is_not_equal(s21_decimal val_1, s21_decimal val_2) {
  int flag = 0;
  if (get_bit(val_1, 127) != get_bit(val_2, 127)) {
    flag = 1;
  } else {
    if (get_scale(val_1) || get_scale(val_2)) normalize(&val_1, &val_2);

    int tmp_flag = s21_comparison_mantissa(val_1, val_2);
    if (tmp_flag == 0) {
      flag = 0;
    } else {
      flag = 1;
    }
  }
  return flag;
}

int s21_comparison_mantissa(s21_decimal d1, s21_decimal d2) {
  int bit1 = 0;
  int bit2 = 0;
  int flag = 0;
  for (int i = 95; i >= 0 && !flag; --i) {
    bit1 = get_bit(d1, i);
    bit2 = get_bit(d2, i);
    if (bit1 > bit2) {
      flag = 1;
    } else if (bit1 < bit2) {
      flag = 2;
    }
  }
  return flag;
}

int s21_is_greater(s21_decimal d1, s21_decimal d2) {
  int flag = 0;
  int tmp_flag = 0;
  if (get_bit(d1, 127) < get_bit(d2, 127)) {
    if (is_zero(d1) && is_zero(d2)) {
      flag = 0;
    } else
      flag = 1;
  } else if ((get_bit(d1, 127) == 0) && (get_bit(d2, 127) == 0)) {
    normalize(&d1, &d2);
    tmp_flag = s21_comparison_mantissa(d1, d2);
    if (tmp_flag == 1) {
      flag = 1;
    } else {
      flag = 0;
    }
  } else if ((get_bit(d1, 127) == 1) && (get_bit(d2, 127) == 1)) {
    normalize(&d1, &d2);
    tmp_flag = s21_comparison_mantissa(d1, d2);
    if (tmp_flag == 2) {
      flag = 1;
    } else {
      flag = 0;
    }
  }

  return flag;
}

int s21_is_greater_or_equal(s21_decimal d1, s21_decimal d2) {
  int flag = s21_is_greater(d1, d2);
  if (flag == 0) {
    flag = s21_is_equal(d1, d2);
  }
  return flag;
}

int s21_is_less(s21_decimal d1, s21_decimal d2) {
  int flag = 0;
  flag = s21_is_greater(d1, d2);
  flag = !flag;
  if (flag == 1) {
    if (s21_is_equal(d1, d2) == 1) {
      flag = 0;
    }
  }
  return flag;
}

int s21_is_less_or_equal(s21_decimal d1, s21_decimal d2) {
  int flag = s21_is_greater(d1, d2);
  flag = !flag;

  return flag;
}
