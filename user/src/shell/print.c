#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print.h"
#include "help.h"
#include "environment.h"
#include "config.h"

int print(config_t cfg)
{
    printf("%s%s%s:%s%s%s > ", colors[cfg.name_color], ENV.USERNAME, KNRM, colors[cfg.path_color], ENV.path, KNRM);
    
    return 0;
}
