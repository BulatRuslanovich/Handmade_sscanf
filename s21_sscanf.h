#ifndef SRC_S21_SSCANF_H_
#define SRC_S21_SSCANF_H_

typedef enum widthType { NONE_WIDTH,
                       WIDTH_NUMBER,
                       WIDTH_STAR } widthType;

enum errorCode {
	OK = -1,
	ERROR,
};

typedef enum lengthType {
    NONE_LENGTH,
    LENGTH_SHORT,
    LENGTH_LONG,
    LENGTH_LONG_LONG,
    LENGTH_DOUBLE,
    LENGTH_LONG_DOUBLE
} lengthType;

typedef struct token {
    void *address;  
    widthType widthType;
	lengthType lengthType;
    unsigned int width;
    char buffer[512];
    char spec;
} token;

int s21_sscanf(const char *str, const char *format, ...);


#endif  