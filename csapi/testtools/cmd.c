
#include "stdhdr.h"
#include "util.h"
#define CMD_L
#include "cmd.h"

#define MAXCMDHISTORY	20
#define CMD_BUF_SIZE 256
static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen);
static char erase_seq[] = "\b \b";      /* erase sequence   */
static char   tab_seq[] = "        ";       /* used to expand TABs  */

static ssize_t safe_read(int fd, void *buf, size_t count)
{
    ssize_t n;

    do {
        n = read(fd, buf, count);
        fflush(stdout);
    } while (n < 0 && errno == EINTR);

    return n;
}
void parse(int *parc, char *parv[], char *src, int maxpar)
{
    int i;
	bool willbeword;

	*parc = 0;
	willbeword = true;
	i = 0;

	if (src == NULL)
		return;

	while (src[i] != 0 && *parc < maxpar) {
		if (src[i] == ' ') {
			willbeword = true;
			src[i] = 0;
		}
		else {
			if (willbeword) {
				parv[*parc] = &src[i];
				(*parc)++;
				willbeword = false;
			}
		}
		i++;
	}
}

void parseex(int *parc, char *parv[], char *src, int maxpar, char separator)
{
	int i;
	bool willbeword;

	*parc = 0;
	parv[*parc] = NULL;
	willbeword = true;
	i = 0;

	if (src == NULL)
		return;

	while (*parc < maxpar) {
		if (src[i] == ' ') {
			src[i] = 0;
		}
		else if (src[i] == separator) {
			src[i] = 0;
			willbeword = true;
			(*parc)++;
			parv[*parc] = NULL;
		}
		else if ((src[i] == 0) && (willbeword == false)) {
			(*parc)++;
			break;
		}
		else {
			if (willbeword) {
				parv[*parc] = &src[i];
				willbeword = false;
			}
		}
		i++;
	}
}

int checkcmdtbl(struct cmd_t *generallist, cmd_module *moduletbl)
{
	int i, j, ncmd;

	i = 0;
    j = 0;
	while (!((moduletbl[i].csapi_cmd)[0].cmd == NULL || (moduletbl[i].csapi_cmd)[0].func == NULL || moduletbl[i].cmd_num + j >= MAXCMD)){
        memcpy(generallist+j, moduletbl[i].csapi_cmd, (moduletbl[i].cmd_num)*sizeof(struct cmd_t));
        j+=moduletbl[i].cmd_num;
        i++;
    }
	ncmd = j;
    printf(" cmd length=%d \n",j);
	if (ncmd == 0)
		return 0;

	for (i = 0; i < ncmd; i++){
        //    printf("%d cmd=%s\n",i,generallist[i].cmd);
		if (strlen(generallist[i].cmd) > 128) {
			printf("***error*** one of user commands to long to over 128\n");
			return -1;
		}
    }
	for (i = 0; i < ncmd; i++)
		for (j = 0; j < ncmd; j++) {
			if (i != j) {

				if (!strcmp(generallist[i].cmd, generallist[j].cmd)) {
					printf("***error*** Two commands conflicts as: %s\n", generallist[i].cmd);
					return -1;
				}

				if (generallist[i].alias && generallist[j].alias && !strcmp(generallist[i].alias, generallist[j].alias)) {
					printf("***error*** Two alias conflics as: %s\n", generallist[i].alias);
					return -1;
				}
			}

			if (generallist[i].alias && !strcmp(generallist[i].alias, generallist[j].cmd)) {
				printf("***error*** Alias conflics to command as: %s\n", generallist[i].alias);
				return -1;
			}
		}

	for (i = 0; i < ncmd; i++) {
		if (!strcmp(generallist[i].cmd, "?")) {
			printf("user command conficts with remain word '?'\n");
			return -1;
		}
		if (!strcmp(generallist[i].cmd, "quit")) {
			printf("user command conficts with remain word 'quit'\n");
			return -1;
		}
		if (!strcmp(generallist[i].cmd, "help")) {
			printf("user command conficts with remain word 'help'\n");
			return -1;
		}

	}
	return ncmd;
}

static char * delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
    char *s;

    if (*np == 0) {
        return (p);
    }

    if (*(--p) == '\t') {           /* will retype the whole line   */
        while (*colp > plen) {
            printf (erase_seq);
            (*colp)--;
        }
        for (s=buffer; s<p; ++s) {
            if (*s == '\t') {
                printf (tab_seq+((*colp) & 07));
                *colp += 8 - ((*colp) & 07);
            } else {
                ++(*colp);
                putc (*s,stdout);
            }
        }
    } else {
        printf (erase_seq);
        (*colp)--;
    }
    (*np)--;
    return (p);
}
  

int readline_into_buffer (const char *const prompt, char * buffer)
{
    char *p = buffer;
    char * p_buf = p;
    int     n = 0;                          /* buffer index         */
    int     plen = 0;                       /* prompt length        */
    int     col;                            /* output column cnt    */
    char    c;

    /* print prompt */
    if (prompt) {
        plen = strlen (prompt);
        printf (prompt);
    }
    col = plen;

    for (;;) {
        fflush(stdout);
        c = fgetc(stdin);
        //        if(safe_read(STDIN_FILENO, &c, 1) <1)
        //    return 0;
        /*
         * Special character handling
         */
       
        switch (c) {
        case '\r':                              /* Enter                */
        case '\n':
            *p = '\0';
            puts ("\r\n");
            return (p - p_buf);

        case '\0':                              /* nul                  */
            continue;

        case 0x03:                              /* ^C - break           */
            p_buf[0] = '\0';        /* discard input */
            return (-1);

        case 0x15:                              /* ^U - erase line      */
            while (col > plen) {
                puts (erase_seq);
                --col;
            }
            p = p_buf;
            n = 0;
            continue;

        case 0x17:                              /* ^W - erase word      */
            p=delete_char(p_buf, p, &col, &n, plen);
            while ((n > 0) && (*p != ' ')) {
                p=delete_char(p_buf, p, &col, &n, plen);
            }
            continue;

        case 0x08:                              /* ^H  - backspace      */
        case 0x7F:                              /* DEL - backspace      */
            p=delete_char(p_buf, p, &col, &n, plen);
            continue;

        default:
            /*
             * Must be a normal character then
             */
            if (n < CMD_BUF_SIZE-2) {
                if (c == '\t') {        /* expand TABs          */
                    puts (tab_seq+(col&07));
                    col += 8 - (col&07);
                } else {
                    ++col;          /* echo input           */
                    putc (c,stdout);
                }
                *p++ = c;
                ++n;
            } else {                        /* Buffer full          */
                putc ('\a', stdout);
            }
        }
    }

}

bool docmd(char *prompt, cmd_module * cmd_moduletbl)
{
	int i, j, tmp, ncmd, nhis = 0, phis = -1;

	char ch, cmdbuf[CMD_BUF_SIZE], hisbuf[MAXCMDHISTORY][256];
	int parc;
	char *parv[64];

    struct termios new_settings;
    struct termios initial_settings; 

    struct cmd_t cmd_list[MAXCMD];

    tcgetattr(STDIN_FILENO, &initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;        /* unbuffered input */
    /* Turn off echoing and CTRL-C, so we can trap it */
    new_settings.c_lflag &= ~(ECHO | ECHONL | ISIG);
    /* Hmm, in linux c_cc[] is not parsed if ICANON is off */
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    /* Turn off CTRL-C, so we can trap it */
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE '\0'
#endif
    new_settings.c_cc[VINTR] = _POSIX_VDISABLE;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    
    
	if ((ncmd = checkcmdtbl(cmd_list, cmd_moduletbl)) < 0)
		return false;
    
	if (ncmd == 0) {
		printf("Command list empty!\n");
		return true;
	}

	printf("\nYou have entered command shell now\n");
	printf("Input '?' for command list or 'help command_name' for help\n\n");


 	do {
        i= readline_into_buffer (prompt, cmdbuf);

// 		printf(prompt);
// 		i = 0;
// 		do {
// 			ch = getc();
//             if (ch != 0x8) 
//                 cmdbuf[i++] = ch;
//             else { 
//                 if (i >0) { 
//                     i--;
//                 }
//             }


// 		} while (ch != 0x0a);

//		i > 0 ? i-- : i;
//		cmdbuf[i] = '\0';
		parse(&parc, parv, cmdbuf, sizeof(parv));
		if (parc == 0)
			continue;
		if (!strcmp(parv[0], "?")) {
			j = 0;
			printf("Command list:\n");

			memset(cmdbuf, 0, sizeof(cmdbuf));
			memset(cmdbuf, ' ', 20);
			memcpy(cmdbuf, "?", 1);
			printf(cmdbuf);
			j++;

			memset(cmdbuf, ' ', 20);
			memcpy(cmdbuf, "help", 4);
			printf(cmdbuf);
			j++;

			memset(cmdbuf, ' ', 20);
			memcpy(cmdbuf, "quit (bye)", 10);
			printf(cmdbuf);
			j++;

			for (i = 0; i < ncmd; i++) {
				if (cmd_list[i].alias)
					sprintf(cmdbuf, "%s (%s)", cmd_list[i].cmd, cmd_list[i].alias);
				else
					sprintf(cmdbuf, "%s", cmd_list[i].cmd);
				tmp = strlen(cmdbuf);
				memset(&cmdbuf[tmp], ' ', 20 - (tmp > 20 ? 20 : tmp));
				cmdbuf[20] = 0;
				printf(cmdbuf);
				if (!((i + j + 1) % 4))
					printf("\n");
			}
			printf("\n");
		}
		else if (!strcmp(parv[0], "help")) {
			if (parc != 2) {
				printf("Please input 'help  command_name', \
					\n   you can input '?' to get command list\n");
				continue;
			}

			if (!strcmp(parv[1], "?"))
				printf("? - get command list  \
					\n   usage: ?\n");
			else if (!strcmp(parv[1], "help"))
				printf("help - get help for the command \
					\n   usage: help command\
					\n      eg: help stat \n");
			else if (!strcmp(parv[1], "quit") || !strcmp(parv[1], "bye"))
				printf("quit - quit program \
					\n   usage: quit \n");

			else {
				for (i = 0; i < ncmd; i++) {
					if (!strcmp(parv[1], cmd_list[i].cmd)
					    || (cmd_list[i].alias && !strcmp(parv[1], cmd_list[i].alias))) {
						printf(cmd_list[i].usage);
						break;
					}
				}
				if (i == ncmd)
					printf("Unrecog command\n");
			}
		}
		else if (!strcmp(parv[0], "quit") || !strcmp(parv[0], "bye") || !strcmp(parv[0], "q")) {
            tcsetattr(STDIN_FILENO, TCSANOW, &initial_settings);
            break;
		}
		else {
			for (i = 0; i < ncmd; i++)
				if (!strcmp(parv[0], cmd_list[i].cmd)
				    || (cmd_list[i].alias && !strcmp(parv[0], cmd_list[i].alias))) {
                    tcsetattr(STDIN_FILENO, TCSANOW, &initial_settings);
                    ((CMD_ROUTINE) cmd_list[i].func) (parc, parv);
                    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
					break;
				}
			if (i == ncmd)
				printf("Command not found, input '?' for command list\n");
		}

	} while (1);
	return true;
}
