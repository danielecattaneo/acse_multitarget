ASM=../../bin/asm
MACE=../../bin/mace
asm_file=$(ASM)
mace_file=$(MACE)
# add .exe at the end on Windows
ifeq ($(OS), Windows_NT)
   asm_file=$(ASM).exe
   mace_file=$(MACE).exe
endif

all: verify

verify: verification.o $(mace_file)
	$(MACE) verification.o | tee mace.log
	@if [ $$(head -n 1 mace.log) = "1000" ]; then \
		printf "\033[32m%s\033[0m\n" "Verification Succeeded"; \
	else \
		printf "\033[31m%s\033[0m\n" "Verification Failed"; \
	fi

%.o: %.asm $(asm_file)
	$(ASM) $< $@

clean:
	rm -rf *.o *.log

.PHONY: all verify clean
