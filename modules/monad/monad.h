#ifndef _MONAD_H_
#define _MONAD_H_

typedef struct monad {
    param_struct ps;
    result_struct rs;
    input_func in;
    output_func out;
    comp_func comp;
} monad;

static result_struct perform_monad(param_struct);


#endif _MONAD_H_