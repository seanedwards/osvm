#include "vm.h"
#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

#include <getopt.h>

typedef enum
{
    PARSE_BINARY,
    PARSE_PBRAIN,
    PARSE_ASM
} parse_type;

int main(int argc, char** argv)
{
	vm_t* vm;
	FILE* f;
    int opt;
    parse_type parser = PARSE_PBRAIN;
    
    const char* filename = 0;

	vm = vm_init();
    
    while ((opt = getopt(argc, argv, "dspabi:")) != -1) {
        switch (opt) {
            case 'i':
                filename = optarg;
                break;
            case 'd':
                vm->flags |= VM_DBGMODE;
                break;
            case 's':
                vm->flags |= VM_STEPMODE;
                break;
                
            case 'p':
                parser = PARSE_PBRAIN;
                break;
            case 'a':
                parser = PARSE_ASM;
                break;
            case 'b':
                parser = PARSE_BINARY;
                break;
                
            case 'h':
                fprintf(stderr, "Usage: %s [-ds] -i <file>\n", argv[0]);
                vm_close(vm);
                return 0;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-ds] -i <file>\n", argv[0]);
                vm_close(vm);
                exit(EXIT_FAILURE);
        }
    }

    if (filename == 0) {
        fprintf(stderr, "Must specify an input file.\n");
        vm_close(vm);
        exit(EXIT_FAILURE);
    }
    
	f = fopen(filename, "r");
    
    if (f == 0) {
        fprintf(stderr, "Could not open file '%s'.", filename);
        vm_close(vm);
        exit(EXIT_FAILURE);
    }
    
    char txtbuf[2048];
	size_t flen = fread(txtbuf, sizeof(char), sizeof(txtbuf), f);
	fclose(f);
    
    switch (parser) {
        case PARSE_PBRAIN:
            parse_pbrain(vm, txtbuf, flen);
            break;
        case PARSE_ASM:
            parse_asm(vm, txtbuf, flen);
            break;
        case PARSE_BINARY:
            parse_binary(vm, txtbuf, flen);
            break;
    }

	vm_run(vm);

	vm_close(vm);
	return 0;
}

