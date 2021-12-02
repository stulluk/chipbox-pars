
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = 0, value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0' : (islower(*cp)
								     ? toupper(*cp) : *cp) - 'A' + 10) < base) {
		result = result * base + value;
		cp++;
	}
	if (endp)
		*endp = (char *) cp;
	return result;
}

// void * memcpy(void * dest,const void *src,int count)
// {
//         char *tmp = (char *) dest, *s = (char *) src;
// 
//         while (count--)
//                 *tmp++ = *s++;
// 
//         return dest;
// }
// 
// void * memset(void * s,int c,int count)
// {
//         char *xs = (char *) s;
// 
//         while (count--)
//                 *xs++ = c;
// 
//         return s;
// }
// 
// int strlen(const char * s)
// {
//         const char *sc;
// 
//         for (sc = s; *sc != '\0'; ++sc)
//                 /* nothing */;
//         return sc - s;
// }
// 
// char * strcpy(char * dest,const char *src)
// {
//         char *tmp = dest;
// 
//         while ((*dest++ = *src++) != '\0')
//                 /* nothing */;
//         return tmp;
// }
// 
// char * strncpy(char * dest,const char *src,int count)
// {
//         char *tmp = dest;
// 
//         while (count) {
//                 if ((*tmp = *src) != 0) src++;
//                 tmp++;
//                 count--;
//         }
//         return dest;
// }
// 
// int strcmp(const char * cs,const char * ct)
// {
//         register signed char __res;
// 
//         while (1) {
//                 if ((__res = *cs - *ct++) != 0 || !*cs++)
//                         break;
//         }
// 
//         return __res;
// }
// 
// int strncmp(const char * cs,const char * ct,int count)
// {
//         register signed char __res = 0;
// 
//         while (count) {
//                 if ((__res = *cs - *ct++) != 0 || !*cs++)
//                         break;
//                 count--;
//         }
// 
//         return __res;
// }

