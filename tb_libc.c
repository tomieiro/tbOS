#include "tb_libc.h"

static int is_space(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' || c == '\v';
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static int is_hex_digit(char c) {
    if (is_digit(c)) {
        return 1;
    }

    if (c >= 'a' && c <= 'f') {
        return 1;
    }

    return c >= 'A' && c <= 'F';
}

static void raw_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t raw_inb(uint16_t port) {
    uint8_t value;

    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void raw_serial_write_char(char c) {
    for (;;) {
        if ((raw_inb(0x3F8 + 5) & 0x20u) != 0) {
            break;
        }
    }

    raw_outb(0x3F8, (uint8_t)c);
}

static void raw_serial_write_text(const char *text) {
    while (*text != '\0') {
        raw_serial_write_char(*text++);
    }
}

static int hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }

    return 10 + (c - 'A');
}

static int write_unsigned_decimal(char *buffer, size_t size, uint32_t value) {
    char digits[16];
    size_t count = 0;
    size_t offset = 0;

    if (size == 0) {
        return 0;
    }

    if (value == 0) {
        if (size > 1) {
            buffer[0] = '0';
            buffer[1] = '\0';
            return 1;
        }

        buffer[0] = '\0';
        return 0;
    }

    while (value > 0 && count < sizeof(digits)) {
        digits[count++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    while (count > 0 && offset + 1 < size) {
        buffer[offset++] = digits[--count];
    }

    buffer[offset] = '\0';
    return (int)offset;
}

static int write_unsigned_hex(char *buffer, size_t size, uintptr_t value) {
    char digits[2 * sizeof(uintptr_t)];
    size_t count = 0;
    size_t offset = 0;

    if (size < 3) {
        if (size > 0) {
            buffer[0] = '\0';
        }
        return 0;
    }

    buffer[offset++] = '0';
    buffer[offset++] = 'x';

    if (value == 0) {
        if (offset + 2 <= size) {
            buffer[offset++] = '0';
            buffer[offset] = '\0';
            return (int)offset;
        }
    }

    while (value > 0 && count < sizeof(digits)) {
        uintptr_t nibble = value & 0xFu;
        digits[count++] = (char)(nibble < 10 ? ('0' + nibble) : ('a' + (nibble - 10)));
        value >>= 4u;
    }

    while (count > 0 && offset + 1 < size) {
        buffer[offset++] = digits[--count];
    }

    buffer[offset] = '\0';
    return (int)offset;
}

void *memcpy(void *destination, const void *source, size_t size) {
    unsigned char *dst = (unsigned char *)destination;
    const unsigned char *src = (const unsigned char *)source;
    size_t index;

    for (index = 0; index < size; ++index) {
        dst[index] = src[index];
    }

    return destination;
}

void *memmove(void *destination, const void *source, size_t size) {
    unsigned char *dst = (unsigned char *)destination;
    const unsigned char *src = (const unsigned char *)source;
    size_t index;

    if (dst == src || size == 0) {
        return destination;
    }

    if (dst < src) {
        for (index = 0; index < size; ++index) {
            dst[index] = src[index];
        }
    } else {
        for (index = size; index > 0; --index) {
            dst[index - 1] = src[index - 1];
        }
    }

    return destination;
}

void *memset(void *destination, int value, size_t size) {
    unsigned char *dst = (unsigned char *)destination;
    size_t index;

    for (index = 0; index < size; ++index) {
        dst[index] = (unsigned char)value;
    }

    return destination;
}

int memcmp(const void *left, const void *right, size_t size) {
    const unsigned char *lhs = (const unsigned char *)left;
    const unsigned char *rhs = (const unsigned char *)right;
    size_t index;

    for (index = 0; index < size; ++index) {
        if (lhs[index] != rhs[index]) {
            return (int)lhs[index] - (int)rhs[index];
        }
    }

    return 0;
}

size_t strlen(const char *text) {
    size_t length = 0;

    while (text[length] != '\0') {
        length++;
    }

    return length;
}

int strcmp(const char *left, const char *right) {
    while (*left != '\0' && *left == *right) {
        left++;
        right++;
    }

    return (unsigned char)*left - (unsigned char)*right;
}

char *strcpy(char *destination, const char *source) {
    char *result = destination;

    while (*source != '\0') {
        *destination++ = *source++;
    }

    *destination = '\0';
    return result;
}

char *strchr(const char *text, int value) {
    char target = (char)value;

    while (*text != '\0') {
        if (*text == target) {
            return (char *)text;
        }
        text++;
    }

    if (target == '\0') {
        return (char *)text;
    }

    return NULL;
}

char *strpbrk(const char *text, const char *accept) {
    while (*text != '\0') {
        const char *cursor = accept;

        while (*cursor != '\0') {
            if (*cursor == *text) {
                return (char *)text;
            }
            cursor++;
        }

        text++;
    }

    return NULL;
}

size_t strspn(const char *text, const char *accept) {
    size_t count = 0;

    while (*text != '\0') {
        const char *cursor = accept;
        int matched = 0;

        while (*cursor != '\0') {
            if (*cursor == *text) {
                matched = 1;
                break;
            }
            cursor++;
        }

        if (!matched) {
            break;
        }

        count++;
        text++;
    }

    return count;
}

int strcoll(const char *left, const char *right) {
    return strcmp(left, right);
}

int abs(int value) {
    return value < 0 ? -value : value;
}

double ldexp(double value, int exponent) {
    while (exponent > 0) {
        value *= 2.0;
        exponent--;
    }

    while (exponent < 0) {
        value *= 0.5;
        exponent++;
    }

    return value;
}

float ldexpf(float value, int exponent) {
    while (exponent > 0) {
        value *= 2.0f;
        exponent--;
    }

    while (exponent < 0) {
        value *= 0.5f;
        exponent++;
    }

    return value;
}

double frexp(double value, int *exponent) {
    double fraction = value;
    int exp = 0;

    if (value == 0.0) {
        *exponent = 0;
        return 0.0;
    }

    if (fraction < 0.0) {
        fraction = -fraction;
    }

    while (fraction >= 1.0) {
        fraction *= 0.5;
        exp++;
    }

    while (fraction < 0.5) {
        fraction *= 2.0;
        exp--;
    }

    *exponent = exp;
    return value < 0.0 ? -fraction : fraction;
}

float frexpf(float value, int *exponent) {
    float fraction = value;
    int exp = 0;

    if (value == 0.0f) {
        *exponent = 0;
        return 0.0f;
    }

    if (fraction < 0.0f) {
        fraction = -fraction;
    }

    while (fraction >= 1.0f) {
        fraction *= 0.5f;
        exp++;
    }

    while (fraction < 0.5f) {
        fraction *= 2.0f;
        exp--;
    }

    *exponent = exp;
    return value < 0.0f ? -fraction : fraction;
}

void abort(void) {
    raw_serial_write_text("abort()\r\n");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

float tb_floorf(float value) {
    int32_t truncated = (int32_t)value;
    float result = (float)truncated;

    if (result > value) {
        result -= 1.0f;
    }

    return result;
}

float tb_fmodf(float left, float right) {
    int32_t quotient;

    if (right == 0.0f) {
        return 0.0f;
    }

    quotient = (int32_t)(left / right);
    return left - ((float)quotient * right);
}

float tb_powf(float base, float exponent) {
    int32_t integer_exponent;
    float result = 1.0f;
    int negative = 0;

    integer_exponent = (int32_t)exponent;
    if ((float)integer_exponent != exponent) {
        return 0.0f;
    }

    if (integer_exponent < 0) {
        negative = 1;
        integer_exponent = -integer_exponent;
    }

    while (integer_exponent > 0) {
        if ((integer_exponent & 1) != 0) {
            result *= base;
        }

        base *= base;
        integer_exponent >>= 1;
    }

    if (negative) {
        if (result == 0.0f) {
            return 0.0f;
        }
        result = 1.0f / result;
    }

    return result;
}

float tb_lua_str2number(const char *text, char **endptr) {
    const char *cursor = text;
    float result = 0.0f;
    float fraction_scale = 0.1f;
    int negative = 0;
    int saw_digit = 0;

    while (is_space(*cursor)) {
        cursor++;
    }

    if (*cursor == '-') {
        negative = 1;
        cursor++;
    } else if (*cursor == '+') {
        cursor++;
    }

    while (is_digit(*cursor)) {
        result = (result * 10.0f) + (float)(*cursor - '0');
        cursor++;
        saw_digit = 1;
    }

    if (*cursor == '.') {
        cursor++;
        while (is_digit(*cursor)) {
            result += (float)(*cursor - '0') * fraction_scale;
            fraction_scale *= 0.1f;
            cursor++;
            saw_digit = 1;
        }
    }

    if (!saw_digit) {
        *endptr = (char *)text;
        return 0.0f;
    }

    if (*cursor == 'e' || *cursor == 'E') {
        int exponent_negative = 0;
        int exponent = 0;
        const char *exp_cursor = cursor + 1;

        if (*exp_cursor == '-') {
            exponent_negative = 1;
            exp_cursor++;
        } else if (*exp_cursor == '+') {
            exp_cursor++;
        }

        if (is_digit(*exp_cursor)) {
            while (is_digit(*exp_cursor)) {
                exponent = (exponent * 10) + (*exp_cursor - '0');
                exp_cursor++;
            }

            while (exponent > 0) {
                result = exponent_negative ? (result / 10.0f) : (result * 10.0f);
                exponent--;
            }

            cursor = exp_cursor;
        }
    }

    if (negative) {
        result = -result;
    }

    *endptr = (char *)cursor;
    return result;
}

float tb_lua_strx2number(const char *text, char **endptr) {
    const char *cursor = text;
    float result = 0.0f;
    int negative = 0;
    int saw_digit = 0;

    while (is_space(*cursor)) {
        cursor++;
    }

    if (*cursor == '-') {
        negative = 1;
        cursor++;
    } else if (*cursor == '+') {
        cursor++;
    }

    if (cursor[0] != '0' || (cursor[1] != 'x' && cursor[1] != 'X')) {
        *endptr = (char *)text;
        return 0.0f;
    }

    cursor += 2;
    while (is_hex_digit(*cursor)) {
        result = (result * 16.0f) + (float)hex_value(*cursor);
        cursor++;
        saw_digit = 1;
    }

    if (!saw_digit) {
        *endptr = (char *)text;
        return 0.0f;
    }

    if (negative) {
        result = -result;
    }

    *endptr = (char *)cursor;
    return result;
}

int tb_lua_number2str(char *buffer, size_t size, float value) {
    uint32_t whole;
    int written = 0;
    int fraction;

    if (size == 0) {
        return 0;
    }

    if (value < 0.0f) {
        if (size < 2) {
            buffer[0] = '\0';
            return 0;
        }

        buffer[written++] = '-';
        value = -value;
    }

    whole = (uint32_t)value;
    written += write_unsigned_decimal(buffer + written, size - (size_t)written, whole);
    if ((size_t)written + 3 >= size) {
        return written;
    }

    buffer[written++] = '.';
    fraction = (int)((value - (float)whole) * 10.0f);
    if (fraction < 0) {
        fraction = -fraction;
    }
    buffer[written++] = (char)('0' + (fraction % 10));
    buffer[written] = '\0';
    return written;
}

int tb_lua_integer2str(char *buffer, size_t size, int32_t value) {
    size_t offset = 0;
    uint32_t unsigned_value;

    if (size == 0) {
        return 0;
    }

    if (value < 0) {
        if (size < 2) {
            buffer[0] = '\0';
            return 0;
        }

        buffer[offset++] = '-';
        unsigned_value = (uint32_t)(-value);
    } else {
        unsigned_value = (uint32_t)value;
    }

    return (int)offset + write_unsigned_decimal(buffer + offset, size - offset, unsigned_value);
}

int tb_lua_pointer2str(char *buffer, size_t size, const void *pointer) {
    return write_unsigned_hex(buffer, size, (uintptr_t)pointer);
}
