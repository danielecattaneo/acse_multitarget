ASM=../../bin/asm
MACE=../../bin/mace

all: verify

verify: verification.o $(MACE)
	$(MACE) verification.o | tee mace.log
	@if [ $$(head -n 1 mace.log) = "1000" ]; then \
		printf "\033[32m%s\033[0m\n" "Verification Succeeded"; \
	else \
		printf "\033[31m%s\033[0m\n" "Verification Failed"; \
	fi

%.o: %.asm $(ASM)
	$(ASM) $< $@

clean:
	rm -rf *.o *.log

.PHONY: all verify clean
