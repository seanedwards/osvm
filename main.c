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
    
    size_t ram_size = 150;
    size_t timeslice_size = 10;
    vm_flags_t flags = 0;
    
    while ((opt = getopt(argc, argv, "hdspabr:t:")) != -1) {
        switch (opt) {
            case 't':
                timeslice_size = atoi(optarg);
                break;
            case 'r':
                ram_size = atoi(optarg);
                break;
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
                fprintf(stderr, "Usage: %s [-ds] [-pab] [-r size] [-t count] <program> [...]\n"
                        "  -r         Size of RAM allocated for each process in words (default 150).\n"
                        "  -t         Size of timeslice given to each program by the scheduler.\n"
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
    
    char** processes = &argv[optind];
    size_t process_count = argc - optind;
    
    if (process_count == 0) {
        fprintf(stderr, "No processes specified. Use %s -h for usage info.", argv[0]);
        return 0;
    }
    
	vm_t* vm;
	vm = vm_init((uint32_t)(ram_size * process_count));
    vm->timeslice_size = (uint32_t)timeslice_size;
    vm->flags = flags;
    
    for (size_t proci = 0; proci < process_count; ++proci) {
        f = fopen(processes[proci], "r");
        
        if (f == NULL) {
            fprintf(stderr, "Could not open file for input: %s.\n", processes[proci]);
            vm_close(vm);
            return 1;
        }
        
        switch (parser) {
            case PARSE_PBRAIN:
                parse_pbrain(vm, f, ram_size * proci);
                vm_spawn(vm, ram_size * proci, ram_size);
                break;
            case PARSE_ASM:
                parse_asm(vm, f, ram_size * proci);
                break;
            case PARSE_BINARY:
                parse_binary(vm, f, ram_size * proci);
                break;
        }
        fclose(f);
    }

	vm_run(vm);

	vm_close(vm);
	return 0;
}

