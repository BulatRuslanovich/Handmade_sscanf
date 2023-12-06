#include "s21_sscanf.h"

int s21_sscanf(const char *str, const char *format, ...) {
  int errCode = checkEOFString(str);

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

    errCode = writeTokensToMemory(&strPtr, tokens, tokenLen);
    va_end(va);
  }

  return errCode;
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
  } else if (result.spec == 0) {
    memset(result.buffer, '\0', BUFF_SIZE - 1);  // очистка буфера

	int i = 0;

	while (**formatPtr && !isSpace(**formatPtr) && **formatPtr != '%')
	{
		result.buffer[i++] = **formatPtr;
		(*formatPtr)++;
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
      errCode = writeIntToMem(strPtr, &tokens[i]);
    }

    if (spec == 'g', spec == 'G', spec == 'f') {
		errCode = writeFloatToMem(strPtr, &tokens[i]);
    }

    if (spec == 's') {
      errCode = writeStringToMem(strPtr, &tokens[i]);
    }

    if (spec == 'z') {
      skipSpaces(strPtr);
    }

    if (spec == 'u') {
		errCode = writeUnsignedToMem(strPtr, &tokens[i]);
    }

    if (spec == 'b') {
		skipCharsInBuffer(strPtr, &tokens[i]);
    }

    if (spec == 'n') {
      *((int *)tokens[i].address) = (*strPtr) - start;
    }

	if (spec == 'i' || spec == 'p') {
		errCode = writeUnspecToMem(strPtr, &tokens[i]);
    }

	if (spec == 'x' || spec == 'X') {
		errCode = writeOctHexToMem(strPtr, &tokens[i], 16);
    }

    if (spec == 'o') {
		errCode = writeOctHexToMem(strPtr, &tokens[i], 8);
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
      *(char *)tok->address = **str;
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

  while (**str &&
         !isStr)  // пока строка не закончилась и он не нашел начало слова
  {
    if (!isSpace(**str)) {
      isStr = 1;

      while (**str && !isEndOfString) {
        buffer[i] = **str;
        i++;

        if (tok->widthType == WIDTH_NUMBER && i >= tok->width) {
          isEndOfString = 1;
        } else {
          (*str)++;

          if (isSpace(**str)) {
            isEndOfString = 1;
          }
        }
      }

      isEndOfString = 1;
    }

    if (isEndOfString == 0) {
      (*str)++;
    }
  }

  if (tok->widthType != WIDTH_STAR && isStr) {
    strcpy((char *)tok->address, buffer);
  }

  return codeErr;
}

int writeIntToMem(char **str, token *tok) {
  long long int result = 0;
  int codeErr = ERROR;
  char buffer[BUFF_SIZE] = {0};

  if (strspn(*str, "0987654321+-")) {
    int signCount = strspn(*str, "+-");

    if (!(signCount > 1 ||
          (signCount && (tok->widthType && tok->width <= 1)))) {
      buffer[0] = **str;
      (*str)++;
      writeAcceptToBuffer(str, "0987654321", buffer, tok->width, 1);
      codeErr = OK;
    }
  }

  	result = atoi(buffer);

	if (tok->spec == 'p' && tok->width != WIDTH_STAR && codeErr == OK) {
		unsigned long long int result = strntollu(buffer, NULL, 16, strlen(buffer));
		*(int *)tok->address = (int)result;
	} else {
		if (tok->widthType != WIDTH_STAR && codeErr == OK) {
			intConverter(tok, result);
		}
	}



  if (tok->widthType != WIDTH_NUMBER)
    writeAcceptToBuffer(str, "0123456789", NULL, 0, 0);  

  return codeErr;
}

void writeAcceptToBuffer(char **str, const char *accept, char *buffer,
                         int width, int startIndex) {
  int isEnd = 0;
  while (**str && strspn(*str, accept) != 0 && !isEnd) {
    if ((width && startIndex >= width) || (isSpace(**str))) {
      isEnd = 1;
    } else {
		if (buffer) {
			buffer[startIndex] = **str;
		}

		(*str)++;
		startIndex++;
    }
  }
}

void intConverter(token *tok, long long int result) {
  if (tok->spec != 'p') {  
    if (tok->lengthType == NONE_LENGTH) {
      *(int *)tok->address = (int)result;
    } else if (tok->lengthType == LENGTH_SHORT) {
      *(short int *)tok->address = (short int)result;
    } else if (tok->lengthType == LENGTH_LONG) {
      *(long int *)tok->address = (long int)result;
    } else if (tok->lengthType == LENGTH_LONG_LONG || tok->lengthType == LENGTH_LONG_DOUBLE) {
      *(long long int *)tok->address = (long long int)result;
    }
  }
}

int skipCharsInBuffer(char **str, token *tok) {
	int errCode = OK;

    int test = strspn(*str, tok->buffer);
    int len = strlen(tok->buffer);

    if (len == 1 && **str == tok->buffer[0]) {
        (*str) = (*str) + len;
    } else {
		if (test != len) {
			errCode = ERROR;
		} else {
			(*str) = (*str) + len;
		}
	}

	return errCode;
}


int writeFloatToMem(char **str, token *tok) {
    int acceptCheck = 0;

    if (tok->spec == 'f') {
        acceptCheck = strspn(*str, "0123456789+-");
	}
    else {
        acceptCheck = strspn(*str, "0123456789eE+-NnaAifIF");
	}

    if (acceptCheck) {
        int signCount = strspn(*str, "+-");

        if (!(signCount > 1 || (signCount && (tok->width <= 1 && tok->widthType)))) {
            char buffer[BUFF_SIZE] = {'\0'};
            int startIndex = 0;

            if (signCount) {
                buffer[0] = **str;
                startIndex = 1;
                (*str)++;
            }

            if (tok->spec == 'f') {
                writeAcceptToBuffer(str, ".0123456789+-", buffer, tok->width,
                                    startIndex);
			}
            else {
                writeAcceptToBuffer(str, ".0123456789eE+-NaAifIFn", buffer, tok->width,
                                    startIndex);
			}

            if (tok->widthType != WIDTH_STAR) {
                long double result = strtold(buffer, NULL);
                floatConverter(tok, result);
            }
        }
    }

    if (tok->widthType != WIDTH_NUMBER) {
        if (tok->spec == 'f') {
            writeAcceptToBuffer(str, ".0123456789", NULL, 0, 0);
		} else {
            writeAcceptToBuffer(str, ".0123456789eE+-NaAifIFn", NULL, 0, 0);
		}
    }
}

void floatConverter(token *tok, long double result) {
    if (tok->lengthType == NONE_LENGTH) {
        *(float *)tok->address = (float)result;
	} else if (tok->lengthType == LENGTH_LONG) {
        *(double *)tok->address = (double)result;
	} else if (tok->lengthType == LENGTH_LONG_DOUBLE) {
        *(long double *)tok->address = (long double)result;
	}
}

int writeUnsignedToMem(char **str, token *tok) {
	int errCode = ERROR;

	skipSpaces(str);

	char buffer[BUFF_SIZE] = {0};

	if (strspn(*str, "0987654321+-")) {
		int signCount = strspn(*str, "+-");
		 if (!(signCount > 1 || (signCount && (tok->width <= 1 && tok->widthType)))) {
            buffer[0] = **str;
			(*str)++;

            writeAcceptToBuffer(str, "0987654321", buffer, tok->width, 1);
			errCode = OK;
		 }
	}

	unsigned long long int result = atoi(buffer);

	if (tok->widthType != WIDTH_STAR && errCode != ERROR) {
		unsignedConverter(tok, result);
	}
        

    if (tok->widthType != WIDTH_NUMBER) {
        writeAcceptToBuffer(str, "0123456789", NULL, 0, 0);
	}

	return ERROR;
}

void unsignedConverter(token *tok, unsigned long long int result) {
    if (tok->lengthType == NONE_LENGTH) {
        *(unsigned int *)tok->address = (unsigned int)result;
    } else if (tok->lengthType == LENGTH_SHORT) {
        *(unsigned short int *)tok->address = (unsigned short int)result;
    } else if (tok->lengthType == LENGTH_LONG) {
        *(unsigned long int *)tok->address = (unsigned long int)result;
    } else if (tok->lengthType == LENGTH_LONG_LONG) {
        *(unsigned long long int *)tok->address = (unsigned long long int)result;
    }
}

int writeOctHexToMem(char **str, token *tok, int base) {
	int errCode = OK;
	int sign = 1;
	char *ptr = NULL;

	if (tok->spec == 'p') {
		base = 16;
	}

	skipSpaces(str);

	if (**str == '-') {
		tok->width--;
		sign = -1;
		(*str)++;
	}

	if (base == 16 && **str == '0' && (*(*str + 1) == 'x' || *(*str + 1) == 'X')) {
        tok->width -= 2;
	}

	if (strspn(*str, "0123456789abcdefABCDEF") > 0 ||
        strspn(*str, "xX0123456789abcdefABCDEF") >= 2) {
        unsigned long long int result = strntollu(*str, &ptr, base, tok->widthType ? tok->width : strlen(*str));

        if (tok->width != WIDTH_STAR) {
            if (tok->spec == 'p') {
                *(int *)tok->address = (int)result;
			} else {
                unsignedConverter(tok, result * sign);
			}
        }
    } else {
        errCode = ERROR;
    }

    unsigned int acceptCount = (unsigned int)strspn(*str, "xX0123456789abcdefABCDEF");

    if (tok->widthType != WIDTH_NUMBER) {
        *str += acceptCount;
	} else {
        *str += acceptCount < tok->width ? acceptCount : tok->width;
	}

	return errCode;
}

unsigned long long int strntollu(const char *str, char **endptr,
                                     int basis, int n_byte) {
    unsigned long long result = 0;
    short sign = 1;
    if (endptr) {
        *endptr = NULL;
	}

    char dict[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (*str == '-') {
        sign = -1;
        str++;
    }
    if (basis == 16 &&
        (!strncmp(str, "0x", 2) || !strncmp(str, "0X", 2)))
        str += 2;
	
    long long val;
    short exit = 0;

    while (*str && n_byte && !exit) {
        char *tmp2;
        char current_sim =
            (*str >= 'a' && *str <= 'z') ? *str - 'a' + 'A' : *str;

        tmp2 = strchr(dict, (int)current_sim);

        if (!tmp2) {
            exit = 1;
        } else {
            val = (tmp2 - dict) / sizeof(char);

            if (val >= basis) {
                exit = 1;
            } else {
                result = result * basis + val;
                str++;
                n_byte--;
            }
        }

        if (exit)
            *endptr = (char *)str;
    }

    return result * sign;
}

int writeUnspecToMem(char **str, token *tok) {
	int errCode = ERROR;
    skipSpaces(str);

    if (strspn(*str, "0x") == 2) {
        errCode = OK;
        writeOctHexToMem(str, tok, 16);
    } else if (strspn(*str, "0") == 1) {
        errCode = OK;
        writeOctHexToMem(str, tok, 8);
    } else if (strspn(*str, "+-0123456789")) {
        errCode = OK;
        writeIntToMem(str, tok);
    }

	return errCode;
}


int main() {
	int a;
	char c;

	s21_sscanf("0xffff", "%p", &a);

  return 0;
}


