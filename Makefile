
rootdir=.

# Where the source files and libraries are
CC=gcc -c -std=c99
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

package:
	@rm -rf dist
	@mkdir dist
	@mkdir dist/obj
	@cp Makefile dist
	@cp *.c dist
	@cp *.h dist
	@cp -R programs dist


