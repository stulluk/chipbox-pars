
#ifndef __MVAPI_GLOBAL_H__
#define __MVAPI_GLOBAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED_VARIABLE(a) do { \
	a = a; \
} while(0);

#define CHECK_HANDLE_VALID(o, m) do { \
	if ((NULL == o) || (o->obj_type != m) ||(o->dev_fd < 0)) { \
		return MVAPI_FAILED; \
	} \
} while(0)

#define IOCTL(o, cmd, arg, t) do { \
	if (-1 == ioctl(o->dev_fd, cmd, arg)) { \
		o->errno = t##_ERROR_IOCTL_FAILED; \
		return MVAPI_FAILED; \
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
	MVAPI_SUCCEED = 0,
	MVAPI_FAILED = -1
} MVAPI_RESULT;

#ifdef __cplusplus
}
#endif

#endif  // __MVAPI_GLOBAL_H__


