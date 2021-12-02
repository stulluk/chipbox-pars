
#ifndef __CSAPI_CMDS_H__
#define __CSAPI_CMDS_H__ 

#ifndef CMD_L
#define CMD_L extern
#endif

#define MAXCMD 256
#define REGISTER_CSAPI_MODULE(name) \
{  name, \
   sizeof(name)/sizeof(struct cmd_t)}
  
typedef void (* CMD_ROUTINE) (int parc, char *parv[]);

struct cmd_t {
	char *cmd;		// cmd==NULL to end of cmd table
	char *alias;		// alias for cmd
	CMD_ROUTINE func;	// pointer to command routine
	char *usage;		// usage for the command
};



typedef struct _cmd_module {
	struct cmd_t * csapi_cmd;
	int cmd_num;
}cmd_module;


CMD_L void parse(int *parc, char *parv[], char *src,int npar);
CMD_L void parseex(int *parc,char *parv[],char *src,int maxpar,char separator);
// CMD_L bool docmd(char *prompt,struct cmd_t *cmdtbl);
CMD_L bool docmd(char *prompt, cmd_module * cmd_moduletbl);

#endif

