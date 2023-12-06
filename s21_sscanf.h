#ifndef SRC_S21_SSCANF_H_
#define SRC_S21_SSCANF_H_
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  // потом свою подключем
#define BUFF_SIZE 512

typedef enum widthType { NONE_WIDTH, WIDTH_NUMBER, WIDTH_STAR } widthType;

enum errorCode {
  ERROR = -1,
  OK,
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
int isSpace(char c);
int isLetter(char c);
int isDigit(char c);
int checkEOFString(const char *str);
token parseToken(char **formatPtr, va_list *va);
void skipSpaces(char **str);
void parseWidth(char **formatPtr, token *result);
int parseNumber(char **formatPtr);
void parseLength(char **formatPtr, token *result);
void parseSpecifier(char **formatPtr, token *result);
int writeTokensToMemory(char **strPtr, token *tokens, int tokenLen);
int writeCharToMem(char **str, token *tok);
int writeStringToMem(char **str, token *tok);

void writeAcceptToBuffer(char **str, const char *accept, char *buffer, int width, int startIndex);
int writeIntToMem(char **str, token *tok);

void intConverter(token *tok, long long int result);
#endif