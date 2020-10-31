#include <stdio.h>
#include <unistd.h>

#include "include/compiler/compiler.h"
#include "include/interpreter/interpreter.h"

int load_source_and_start_repl(char *file_name) {
//    FILE *source_file = fopen(file_name, "r");
//
//    if (!source_file) {
//        printf("Failed to open source file: %s.\n", file_name);
//        return 1;
//    }
//
//    fseek(source_file, 0, SEEK_END);
//    size_t source_size = ftell(source_file);
//    rewind(source_file);
//
//    char *source_buffer = (char *) malloc(sizeof(char) * (source_size + 1));
//    size_t read_size = fread(source_buffer, sizeof(char), source_size, source_file);
//    source_buffer[source_size] = '\0';
//
//    if (source_size != read_size) {
//        free(source_buffer);
//        printf("Error while reading source file.\n");
//        return 1;
//    }
//
//    context_t *ctx = create_context();
//    populate_builtins(ctx);
//
//    switch (parse_source(source_buffer, source_size, ctx)) {
//        case SUCCESS:
//            printf("Successfully compiled.\n");
//            break;
//        case INCOMPLETE_DECLARATION:
//            printf("Incomplete declaration detected.\n");
//            return 1;
//        case MALFORMED_FUNC:
//            printf("Malformed function declaration detected.\n");
//            return 1;
//        case MALFORMED_TYPE:
//            printf("Malformed type declaration detected.\n");
//            return 1;
//    }
//
//    if (ctx->functions) {
//        printf("Compiled functions:\n");
//
//        func_list_t f = ctx->functions;
//
//        while (f) {
//            printf("%s\n", f->function.name);
//            f = f->tail;
//        }
//    } else {
//        printf("No functions found in source.\n");
//    }
//
//    if (ctx->types) {
//        printf("Compiled types:\n");
//
//        type_list_t t = ctx->types;
//
//        while (t) {
////            printf("%s\n", t->type.name);
//            t = t->tail;
//        }
//    } else {
//        printf("No types found in source.\n");
//    }
//
//    start_repl_interpreter(ctx);
//
//    free_context(ctx);
//
//    fclose(source_file);
//    free(source_buffer);

    return 0;
}

void test_program(void) {

    context_builder_t *builder = create_context_builder();
    function_builder_t *first_builder = create_function_builder();
    expr_manager_t *expr_manager = create_expr_manager();

    type_t *number = builder_create_base_type_with_print(builder, "Number", 6, &print_number_type);

    int five = 5, seven = 7;

    constant_t c5;
    c5.value = &five;
    c5.type = number;

    constant_t c7;
    c7.value = &seven;
    c7.type = number;

    lambda_t *lam_s0 = builder_create_lambda(first_builder), *lam_s1 = builder_create_lambda(first_builder);

    var_t *x = &lam_s0->var, *y = &lam_s1->var;
    set_variable(x, "x", 1, number);
    set_variable(y, "y", 1, number);

    set_expression_to_variable(&lam_s1->expr, x);
    set_expression_to_abstraction(builder, &lam_s0->expr, lam_s1);

    expr_t *arg_0 = create_constant_expression(expr_manager, &c7), *arg_1 = create_constant_expression(expr_manager, &c5);

    func_t *first = compile_function(builder, first_builder, expr_manager, "first", 5);

    context_t *context = compile_context(builder);

    free_function_builder(first_builder);
    free_context_builder(builder);

    free_context(context);
    free_expr_manager(expr_manager);
}

int main(int argc, char *argv[]) {
    /* Called like:
     * ./help-lang source.help
     * */

    if (argc < 2) {
        fprintf(stderr, "No source file provided.\n");
        return 1;
    }
    char *file_name = argv[1];

    FILE *source_file = fopen(file_name, "r");

    if (!source_file) {
        printf("Failed to open source file: %s.\n", file_name);
        return 1;
    }

    fseek(source_file, 0, SEEK_END);
    size_t source_size = ftell(source_file);
    rewind(source_file);

    char *source_buffer = (char *) malloc(sizeof(char) * (source_size + 1));
    size_t read_size = fread(source_buffer, sizeof(char), source_size, source_file);
    source_buffer[source_size] = '\0';

    fclose(source_file);

    if (source_size != read_size) {
        free(source_buffer);
        printf("Error while reading source file.\n");
        return 1;
    }

    char *preprocessed;
    size_t preprocessed_size;

    preprocess(source_buffer, read_size, &preprocessed, &preprocessed_size);

    context_t *context;
    expr_manager_t *expr_manager;

    compile_source(preprocessed, preprocessed_size, &context, &expr_manager);

//    if (context->n_functions) {
//        printf("Compiled functions:\n");
//
//        for (size_t i = 0; i < context->n_functions; i++) {
//            printf("%s\n", context->functions[i].name);
//        }
//    } else {
//        printf("No functions found in source.\n");
//    }
//
//    if (context->n_base_types) {
//        printf("Compiled types:\n");
//
//        for (size_t i = 0; i < context->n_base_types; i++) {
//            printf("%s\n", context->base_types[i].name);
//        }
//    } else {
//        printf("No types found in source.\n");
//    }

    start_repl_interpreter(context, expr_manager);

    free_context(context);
    free_expr_manager(expr_manager);

    free(source_buffer);

    return 0;
}
