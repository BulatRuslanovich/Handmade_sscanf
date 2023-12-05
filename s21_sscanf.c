#include "s21_sscanf.h"


int s21_sscanf(const char *str, const char *format, ...) {
  int errCode = checkEOFString(str);
  int result = 0;

  if (errCode == OK) {
    va_list va;
    va_start(va, format);
    int tokenLen = 0;

    char *formatPtr = (char *)format;
    int lenFormatPtr = (int)strlen(formatPtr);
    char *strPtr = (char *)str;

    token tokens[BUFF_SIZE];

    while (*formatPtr && formatPtr < format + lenFormatPtr &&
           tokenLen < BUFF_SIZE) {
      tokens[tokenLen] = parseToken(&formatPtr, &va);
      tokenLen++;
    }

    result = writeTokensToMemory(&strPtr, tokens, tokenLen);
    va_end(va);
  }

  return (errCode == OK) ? errCode : result;
}

int isSpace(char c) { return (c == ' ' || c == '\n' || c == '\t'); }

int isLetter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isDigit(char c) { return c >= '0' && c <= '9'; }

int checkEOFString(const char *str) {
  int result = ERROR;

  for (size_t i = 0; str[i]; i++) {
    if (!isSpace(str[i]) && str[i] != '\0') {
      result = OK;
      break;
    }
  }

  return result;
}

token parseToken(char **formatPtr, va_list *va) {
  token result = {.address = NULL,
                  .lengthType = NONE_LENGTH,
                  .widthType = NONE_WIDTH,
                  .width = 0,
                  .spec = 0};

  if (isSpace(**formatPtr)) {
    result.spec = 'z';
    skipSpaces(formatPtr);
  }

  if (**formatPtr == '%' && result.spec == 0) {
    (*formatPtr)++;

    if (**formatPtr == '%') {
      memset(result.buffer, '\0', BUFF_SIZE - 1);  // очистка буфера
      result.buffer[0] = '%';
      (*formatPtr)++;
      result.spec = 'b';
    } else {
      parseWidth(formatPtr, &result);
      parseLength(formatPtr, &result);
      parseSpecifier(formatPtr, &result);

      if (result.widthType != WIDTH_STAR) {
        result.address = va_arg(*va, void *);
      }
    }

    if (result.spec == 'p') {  // p == pointer == адрес (за ассоциацию не за
                               // что)
      result.lengthType = NONE_LENGTH;
    }
  }

  if (isLetter(**formatPtr) && result.spec != 0) {
    memset(result.buffer, '\0', BUFF_SIZE - 1);  // очистка буфера

    for (size_t i = 0;
         **formatPtr && !isSpace(**formatPtr) && **formatPtr != '%';
         i++, (*formatPtr)++) {
      result.buffer[i] = **formatPtr;
    }

    result.spec = 'b';
  }

  return result;
}

void skipSpaces(char **str) {
  while (**str && isSpace(**str)) {
    (*str)++;
  }
}

void parseWidth(char **formatPtr, token *result) {
  if (**formatPtr == '*') {
    (*formatPtr)++;
    result->widthType = WIDTH_STAR;
  } else {
    int width = parseNumber(formatPtr);

    if (width != 0) {
      result->widthType = WIDTH_NUMBER;
      result->width = width;
    }
  }
}

int parseNumber(char **formatPtr) {
  char temp[BUFF_SIZE] = {'\0'};

  for (size_t i = 0; isDigit(**formatPtr); i++, (*formatPtr)++) {
    temp[i] = **formatPtr;
  }

  return atoi(temp);
}

void parseLength(char **formatPtr, token *result) {
  switch (**formatPtr) {
    case 'h':
      result->lengthType = LENGTH_SHORT;
      (*formatPtr)++;
      break;
    case 'l':
      result->lengthType = LENGTH_LONG;
      (*formatPtr)++;

      if (**formatPtr == 'l') {
        result->lengthType = LENGTH_LONG_LONG;
        (*formatPtr)++;
      }

      break;
    case 'L':
      result->lengthType = LENGTH_LONG_DOUBLE;
      (*formatPtr)++;
      break;
  }
}

void parseSpecifier(char **formatPtr, token *result) {
  result->spec = (**formatPtr);
  (*formatPtr)++;
}

int writeTokensToMemory(char **strPtr, token *tokens, int tokenLen) {
  int errCode = 0;
  char *start = *strPtr;

  for (size_t i = 0; i < tokenLen; i++) {
    char spec = tokens[i].spec;

    if (spec == 'c') {
		errCode = writeCharToMem(strPtr, &tokens[i]);
    }

    if (spec == 'd') {
    }

    if (spec == 'i' || spec == 'p') {
    }

    if (spec == 'g', spec == 'G', spec == 'f') {
    }

    if (spec == 's') {
		errCode = writeStringToMem(strPtr, &tokens[i]);
    }

    if (spec == 'x' || spec == 'X') {
    }

    if (spec == 'o') {
    }

    if (spec == 'z') {
      skipSpaces(strPtr);
    }

    if (spec == 'u') {
    }

    if (spec == 'b') {
    }

    if (spec == 'n') {
    }
  }
}

int writeCharToMem(char **str, token *tok) {
	int codeErr = ERROR;

	if (**str) {
		codeErr = OK;

		if (tok->width == WIDTH_STAR) {
			(*str)++;
		} else {
			*(char*)tok->address = **str;
			(*str)++;
		}
	}

	return codeErr;
}

int writeStringToMem(char **str, token *tok) {
	int codeErr = ERROR;
	size_t i = 0;
	char buffer[BUFF_SIZE] = {0};
	int isStr = 0;
	int isEndOfString = 0;

	while (**str && !isStr) //пока строка не закончилась и он не нашел начало слова
	{
		if (!isSpace(**str)) {
			isStr = 1;

			while (**str && !isEndOfString)
			{
				buffer[i] = **str;
				i++;

				if (tok->widthType == WIDTH_NUMBER && i >= tok->width) {
					isEndOfString = 1;
				}

				(*str)++;

				if (isSpace(**str)) {
					(*str)--;
					isEndOfString = 1;
				}

			}
			
			
		}

		(*str)++;
	}


	if (tok->widthType != WIDTH_STAR && isStr) {
        strcpy((char *)tok->address, buffer);
    }

	return codeErr;
}


int main() {
	char* test_char;
	s21_sscanf("lopata", "%s", test_char);

	printf("%s\n", test_char);
}