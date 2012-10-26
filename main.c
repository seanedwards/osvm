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
	FILE* f;
    int opt;
    parse_type parser = PARSE_PBRAIN;
    
    int ram_size = 150;
    vm_flags_t flags = 0;
    
    const char* filename = 0;

    
    while ((opt = getopt(argc, argv, "hdspabi:r:")) != -1) {
        switch (opt) {
            case 'i':
                filename = optarg;
                break;
            case 'r':
                ram_size = atoi(optarg);
            case 'd':
                flags |= VM_DBGMODE;
                break;
            case 's':
                flags |= VM_STEPMODE;
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
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-ds] [-pab] -i <file>\n"
                        "  -i         Specify input program.\n"
                        "  -r         Size of RAM in words (default 100).\n"
                        "\n"
                        "Debugging options:\n"
                        "  -d         Run program with debug output.\n"
                        "  -s         Run program with step-through debugging enabled.\n"
                        "\n"
                        "Input formats:\n"
                        "  -p         Parse input file as PBRAIN12 code (default).\n"
                        "  -a         Parse input file as PASM high level assembly (not yet supported).\n"
                        "  -b         Parse input file as a binary RAM image.\n", argv[0]);
                return 0;
        }
    }

    if (filename == 0) {
        fprintf(stderr, "Must specify an input file.\n");
        exit(EXIT_FAILURE);
    }
    
	f = fopen(filename, "r");
    
    if (f == 0) {
        fprintf(stderr, "Could not open file '%s'.", filename);
        exit(EXIT_FAILURE);
    }
    
	vm_t* vm;
	vm = vm_init(ram_size);
    vm->flags = flags;
    
    switch (parser) {
        case PARSE_PBRAIN:
            parse_pbrain(vm, f);
            break;
        case PARSE_ASM:
            parse_asm(vm, f);
            break;
        case PARSE_BINARY:
            parse_binary(vm, f);
            break;
    }
	fclose(f);

	vm_run(vm);

	vm_close(vm);
	return 0;
}

