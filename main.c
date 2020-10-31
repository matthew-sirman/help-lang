#include <stdio.h>
#include <unistd.h>

#include "include/compiler/compiler.h"
#include "include/interpreter/interpreter.h"

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
