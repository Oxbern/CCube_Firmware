#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <reent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>


void _exit(int n) {
label: goto label;       /* plz don't kill me */
}

static char *_heap_start = (char *)0xC0177000;
static char *_heap_end = (char *)(0xC0177000+2048*4);
static char *heap_ptr = NULL;

void * _sbrk_r(struct _reent *_s_r, ptrdiff_t nbytes)
{
	char  *base;		/*  errno should be set to  ENOMEM on error	*/

	if (!heap_ptr) {	/*  Initialize if first time through.		*/
		heap_ptr = _heap_start;
	}
	base = heap_ptr;	/*  Point to end of heap.					*/
	
	if (heap_ptr + nbytes > _heap_end)
	{
			errno = ENOMEM;
			return (caddr_t) -1;
	}
	heap_ptr += nbytes;	/*  Increase heap.							*/
	
	return base;		/*  Return pointer to start of new heap area.	*/
}



void * _sbrk(ptrdiff_t incr)
{
  char  *base;

/* Initialize if first time through. */

  if (!heap_ptr) heap_ptr = _heap_start;

  base = heap_ptr;      /*  Point to end of heap.                       */

	if (heap_ptr + incr > _heap_end)
	{
			errno = ENOMEM;
			return (caddr_t) -1;
	}
  
  heap_ptr += incr;     /*  Increase heap.                              */

  return base;          /*  Return pointer to start of new heap area.   */
}


int _close(int file)
{
	return -1;
}


int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _read(int file, char *ptr, int len)
{
	return 0;
}

int _open(char *path, int flags, ...)
{
	/* Pretend like we always fail */
	return -1;
}

int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int _times(struct tms *buf)
{
	return -1;
}

int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}
