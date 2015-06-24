//Declare funcs called when commands are executed
//在这里声明指令执行时调用的函数

//for private usage
typedef void (*xcommand_t) (void);

typedef struct cmd_function_s
{
	struct cmd_function_s *next;
	char *name;
	xcommand_t function;
	int flags;
}
cmd_function_t;

extern cmd_function_t *(*Cmd_GetCmdBase)(void);

cmd_function_t *Cmd_FindCmd(char *cmd_name);
xcommand_t Cmd_HookCmd(char *cmd_name, xcommand_t newfuncs);
cmd_function_t *Cmd_FindCmdPrev(char *cmd_name);
char *Cmd_CompleteCommand(char *partial, qboolean next);