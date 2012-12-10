
rootdir=.

# Yes, I know gnu99 isn't technically c99. Don't tell prof. Dickens.
# I need it for anonymous structs. vm.h line 16.
CC=gcc -c -std=gnu99
LD=gcc 

# List of source files
sourcelist = $(shell find $(rootdir) -name '*.c' | sed "s,[.]/,,g")

# Where the generated documentation will be put

default: all 

all:
        # Compile source files
	@for file in $(sourcelist) ; do \
		$(CC) $$file -o obj/$$file.o ; \
	done

	$(LD) -o vm obj/*.o

	@echo "Build complete. Type ./vm -h for usage instructions."

clean:
        # Remove all the class files in the classpath
	@-find $(rootdir) \( -name "*~" -o -name "*.o" \) -exec rm '{}' \; 

	@rm -rf dist

dist:
	@mkdir dist
	@mkdir dist/obj
	@cp Makefile dist
	@cp test*.sh dist
	@cp *.c dist
	@cp *.h dist
	@cp -R programs dist


