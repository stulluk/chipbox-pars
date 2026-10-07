/*
 * glibc_compat.c - keep glibc 2.3.6 behaviour that mvapp relies on, for the EABI build.
 *
 * mvapp calls pthread_cancel() on thread handles that may never have been created
 * (static pthread_t still 0). glibc 2.3.6 returned ESRCH; glibc 2.36 treats the handle
 * as a pointer and crashes (SIGSEGV at address 0x68). The EABI link uses
 * -Wl,--wrap=pthread_cancel so every call from mvapp goes through this function.
 *
 * LinuxThreads (glibc 2.3.6) sem_wait() never failed with EINTR; NPTL does whenever a
 * signal handler without SA_RESTART runs (MiniGUI's 10 ms SIGALRM timer). mvapp treats
 * any sem_wait() failure as "lock not taken" and then posts the semaphore anyway, which
 * breaks mutual exclusion. -Wl,--wrap=sem_wait,--wrap=sem_timedwait retry on EINTR.
 */
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

int __real_pthread_cancel(pthread_t thread);

/* pthread_cancel() that rejects the 0 handle like the old glibc did. */
int __wrap_pthread_cancel(pthread_t thread)
{
  if (thread == (pthread_t)0)
    return ESRCH;
  return __real_pthread_cancel(thread);
}

int __real_sem_wait(sem_t *sem);
int __real_sem_timedwait(sem_t *sem, const struct timespec *abstime);

/* sem_wait() that restarts after a signal handler instead of failing with EINTR. */
int __wrap_sem_wait(sem_t *sem)
{
  int rc;

  do {
    rc = __real_sem_wait(sem);
  } while (rc != 0 && errno == EINTR);
  return rc;
}

/* sem_timedwait() that restarts after a signal handler; the deadline is absolute. */
int __wrap_sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
  int rc;

  do {
    rc = __real_sem_timedwait(sem, abstime);
  } while (rc != 0 && errno == EINTR);
  return rc;
}
