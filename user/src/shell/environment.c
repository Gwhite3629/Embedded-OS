typedef struct environment_t {
    char USERNAME[32];
    char path[4096];
    char home[4096];
} environment_t;

environment_t ENV = {
    "DEFAULT",
    "",
    ""
};
