typedef struct param_struct {

} param_struct;

typedef struct result_struct {

} result_struct;

typedef param_struct input_func(const char [20]);

typedef char* output_func(result_struct);

#include "monad.h"

int main(void)
{
    monad m;
    result_struct rs;
    param_struct ps;
    input_func in;
    output_func out;
    m.rs = rs;
    m.ps = ps;
    m.in = in;
    m.out = out;


    return 0;
}