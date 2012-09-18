#include "vm.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char** argv)
{
	vm_t* vm;
	FILE* f;

	if (argc == 1)
		printf("Must specify a file name.\n");

	vm = vm_init();

	f = fopen(argv[1], "r");
	fread(vm_ram32(vm, 0), sizeof(uint32_t), MAX_MEM, f);
	fclose(f);

	vm_run(vm);

	vm_close(vm);
	return 0;
}

