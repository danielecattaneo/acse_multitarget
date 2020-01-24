target?=mace
dirs:=mace assembler acse tests

ifeq ($(target), mace)
all : executor asm compiler
else
all : compiler
endif

bin :
	mkdir -p bin

tests : 
	cd ./tests && $(MAKE)

executor :
	cd ./mace && $(MAKE)

asm :
	cd ./assembler && $(MAKE)

compiler :
	cd ./acse && $(MAKE)

clean :
	for i in $(dirs) ; do cd $$i && $(MAKE) clean; cd .. ; done
	rm -rf bin

.PHONY : all clean tests executor asm compiler
