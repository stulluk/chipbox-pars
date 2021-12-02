
#ifndef __CSAPI_GLOBAL_H__
#define __CSAPI_GLOBAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED_VARIABLE(a) do { \
	a = a; \
} while(0);

#define CHECK_HANDLE_VALID(o, m) do { \
	if ((NULL == o) || (o->obj_type != m) ||(o->dev_fd < 0)) { \
		return CSAPI_FAILED; \
	} \
} while(0)

#define IOCTL(o, cmd, arg, t) do { \
	if (-1 == ioctl(o->dev_fd, cmd, arg)) { \
		o->errno = t##_ERROR_IOCTL_FAILED; \
		return CSAPI_FAILED; \
	}; \
} while(0)
 
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

typedef enum
{
	CSAPI_SUCCEED = 0,
	CSAPI_FAILED = -1
} CSAPI_RESULT;

#ifdef __cplusplus
}
#endif

#endif  // __CSAPI_GLOBAL_H__


