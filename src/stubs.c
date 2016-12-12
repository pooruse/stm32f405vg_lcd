#include <stdio.h>

#include "stubs.h"
#include "st7920.h"
#include "SEGGER_RTT.h"

#ifdef _NEWLIB_VERSION

#include "stm32f4xx.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

#undef errno

extern int errno;

static int mode = 0;

int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
caddr_t _sbrk(int incr);
int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);

void stubs_set_channel(int ch)
{
    mode = ch;
}


int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat *st) {
    return 0;
}

int _isatty(int file) {
    switch (file){
    case STDOUT_FILENO:
    case STDERR_FILENO:
    case STDIN_FILENO:
	return 1;
    default:
	errno = EBADF;
	return 0;
    }
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

int _read(int file, char *ptr, int len) {
    return 0;
}

int _write(int file, char *ptr, int len) {
    int n;
    switch (file) {
    case STDERR_FILENO: /* stderr */	
    case STDOUT_FILENO: /*stdout*/
	if(mode == PUTC_LCD) {
	    for(n = 0; n < len; n++){
	    	lcd_putc(ptr[n]);
	    }
	} else if(mode == PUTC_RTT) {
	    SEGGER_RTT_Write(0, ptr, (unsigned)len);
	}
	break;

    default:
	errno = EBADF;
	return -1;
    }
    return len;
}

caddr_t _sbrk(int incr) {
    extern char _ebss; // Defined by the linker
    static char *heap_end;
    char *prev_heap_end;
    char * stack;
    if (heap_end == 0) {
        heap_end = &_ebss;
    }
    prev_heap_end = heap_end;

    
    stack = (char*) __get_MSP();
    if (heap_end + incr >  stack)
    {
	_write (STDERR_FILENO, "Heap and stack collision\n", 25);
	errno = ENOMEM;
	return  (caddr_t) -1;
    }

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}


#else

/* In stdio.h file is everything related to output stream */
#include "stdio.h"

/* We need to implement own __FILE struct */
/* FILE struct is used from __FILE */
struct __FILE {
    int dummy;
};

/* You need this if you want use printf */
/* Struct FILE is implemented in stdio.h */
FILE __stdout;

int fputc(int ch, FILE *f) {
    /* Do your stuff here */
    /* Send your custom byte */
    /* Send byte to USART */
    lcd_putc((char) ch);

    /* If everything is OK, you have to return character written */
    return ch;
    /* If character is not correct, you can return EOF (-1) to stop writing */
    //return -1;
}
 
#endif
