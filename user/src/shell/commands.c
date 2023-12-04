typedef struct command_t {
    char name[32];
    int hash;
	int id;
} command_t;

#define TABLESIZE 12

command_t COMMANDS[TABLESIZE] = {
{
    "exit",
    2090237505,
    0
},
{
    "print",
    271190388,
    1
},
{
    "help",
    2090324720,
    2
},
{
    "clear",
    255553006,
    3
},
{
    "user",
    2090806918,
    4
},
{
    "pwd",
    193502992,
    5
},
{
    "cd",
    5863276,
    6
},
{
    "add-path",
    2135670865,
    7
},
{
    "del-path",
    599048442,
    8
},
{
    "path",
    2090608116,
    9
},
{
    "ncolor",
    263502008,
    10
},
{
    "pcolor",
    341772794,
    11
}
};
