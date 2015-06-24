typedef void (*xcommand_t)(void);

typedef struct cmd_function_s
{
	struct cmd_function_s *next;
	char *name;
	xcommand_t function;
	int flags;
}
cmd_function_t;

cmd_function_t *Cmd_GetBase(void);
cmd_function_t *Cmd_FindCmd(char *cmd_name);
int Cmd_AddCommand(char *cmd_name, xcommand_t function);
xcommand_t Cmd_HookCmd(char *cmd_name, xcommand_t newfuncs);
cmd_function_t *Cmd_FindCmdPrev(char *cmd_name);
bool Cmd_RemoveCmd(char *cmd_name);
bool Cmd_BlockCmd(char *cmd_name);
bool Cmd_CallCmd(char *cmd_name);
void Cmd_Init(void);