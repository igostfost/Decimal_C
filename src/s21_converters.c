#include "s21_decimal.h"

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  clear_decimal(dst);
  int flag = 0;
  if ((src <= INT_MAX && src >= INT_MIN) || src == 0) {
    if (src < 0) {
      set_minus(dst);
      src *= -1;
    }
    dst->bits[0] = src;
  } else
    flag = 1;
  return flag;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  int error = 0;
  int scale = get_scale(src);
  if (src.bits[0] > UINT_MAX) {
    error = 1;
  } else {
    if (src.bits[1] || src.bits[2]) {
      error = 1;
    } else {
      *dst = src.bits[0];
      if (scale > 0 && scale <= 28) {
        *dst = *dst / pow(10, scale);
      }
    }
    if (get_bit(src, 127)) *dst = *dst * (-1);
  }
  return error;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  double tmp = (double)*dst;
  int error = 0;
  if (get_scale(src) > 28) {
    error = 1;
    tmp = 0;
  }
  // add "else if" with check value src over big or == inf;
  else if (src.bits[0] > UINT_MAX || src.bits[1] > UINT_MAX ||
           src.bits[2] > UINT_MAX) {
    error = 1;
    tmp = 0;
  } else {
    for (int i = 0; i <= 95; i++) {
      tmp = tmp + get_bit(src, i) * pow(2, i);
    }
    tmp = tmp * pow(10, -get_scale(src));
    if (get_bit(src, 127) == 1) tmp = tmp * (-1);
  }
  *dst = tmp;

  return error;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  clear_decimal(dst);
  int return_value = 0;

  if (isinf(src) || isnan(src)) {
    return_value = 1;  // Проверка на бесконечность или NaN
  } else {
    if (src != 0) {
      int exp = ((*(int *)&src & ~0x80000000) >> 23) -
                127;  // Получение экспоненты числа и сохранение в exp
      double temp = (double)fabs(
          src);  // Получение абсолютного значения числа и сохранение в temp
      int i = 0;  // Смещение для масштаба числа
      // Цикл для определения смещения off в зависимости от значения temp
      for (; i < 28 && (int)temp / (int)pow(2, 21) == 0; temp *= 10, i++) {
      }
      temp = round(temp);  // Округление temp до ближайшего целого

      if (i <= 28 &&
          (exp > -94 && exp < 96)) {  // Проверка условий для преобразования
                                      // числа в десятичное представление
        floatbits mant = {0};  // Структура для извлечения мантиссы числа
        temp = (float)temp;  // Приведение temp к типу float
        // Цикл для определения смещения i в зависимости от значения temp
        for (; fmod(temp, 10) == 0 && i > 0; i--, temp /= 10) {
        }
        mant.float_bits = temp;  // Присваивание мантиссы числа структуре mant
        exp = ((*(int *)&mant.float_bits & ~0x80000000) >> 23) -
              127;  // Получение экспоненты из мантиссы
        dst->bits[exp / 32] |=
            1 << exp % 32;  // Установка соответствующего бита в структуре dst
        for (int i = exp - 1, j = 22; j >= 0; i--, j--) {
          if ((mant.int_bits & (1 << j)) != 0)
            dst->bits[i / 32] |=
                1 << i % 32;  // Установка соответствующего бита в структуре dst
        }
        dst->bits[3] = (i << 16);  // Установка смещения в структуре dst
      }

      if (src < 0.0) {
        set_minus(dst);  // Установка знака минус в структуре dst
      }
    }
  }
  return return_value;  // Возвращение результ
}

int s21_from_decimal_to_double(s21_decimal src, double *dst) {
  long double tmp = (double)*dst;
  for (int i = 0; i < 96; i++) {
    tmp = tmp + get_bit(src, i) * pow(2, i);
  }
  tmp = tmp * pow(10, -get_scale(src));
  if (get_bit(src, 127)) tmp = tmp * (-1);
  *dst = tmp;
  return 0;
}
