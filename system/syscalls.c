#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <reent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "GUI.h"
#include "console.h"


void _exit(int n) {
label: goto label;       /* plz don't kill me */
}

#define HEAP_START 0xC0200000

static caddr_t _heap_start = (caddr_t)HEAP_START;
static caddr_t _heap_end = (caddr_t)(HEAP_START+0x500000); /* 5 Mo */
static caddr_t heap_ptr = (caddr_t)NULL;

void * _sbrk_r(struct _reent *_s_r, ptrdiff_t nbytes)
{
	caddr_t base;		/*  errno should be set to  ENOMEM on error	*/

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
	
	return (caddr_t)base;		/*  Return pointer to start of new heap area.	*/
}

_ssize_t  _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt)
{
	/*
	for (uint32_t i = 0; i < cnt; i++)
	{
		char c[2];
		c[0] = *((char*)buf+i);
		c[1] = '\0';
		GUI_DispString(c);
	}
	*/
	console_write((char*)buf, cnt);
	//console_disp();
	return cnt;
}


/*
void * _sbrk(ptrdiff_t incr)
{
  void  *base;

  if (!heap_ptr) heap_ptr = _heap_start;

  base = heap_ptr;      

  if (heap_ptr + incr > _heap_end)
  {
		errno = ENOMEM;
		return (caddr_t) -1;
  }
  
  heap_ptr += incr;     

  return base;
}
*/

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

int _kill(int pid, int sig)
{
	return -1;
}

int _getpid()
{
	return -1;
}
