#if 1
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <fcntl.h>
#include <sys/ioctl.h>


int getch()
{
	char ch;
	struct termios save, ne;

	//ioctl(0, TCGETS, &save);
	ioctl(0, TCGETA, &ne);

	save=ne;

	ne.c_lflag &= ~(ICANON|ECHO|ISIG);   
	ne.c_cc[VMIN]   =   0     ;         //   MIN   
       ne.c_cc[VTIME]   =   1   ;         //   TIME     
       if(   ioctl(   0,   TCSETAF,   &ne   )   ==   -1)     
                  {   
                                ioctl(0,  TCSETAF, &save);
                               // perror(   "ioctl"   );   
                                return -2;   
                  }   


	//ioctl(0, TCSETS, &ne);

	read(0, &ch, 1);
	ioctl(0,  TCSETAF, &save);

	return ch;
}
#endif
void Mac_Task(void )

{
  char sbuf;
	while(1)  
		{
			CSOS_DelayTaskMs(300);
			sbuf=getch();			
			printf("%x",sbuf);
		}
}
