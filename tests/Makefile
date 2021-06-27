dirs:=$(patsubst %/,%,$(shell echo */)) # automatically lists the dirs

.PHONY: test
test: $(dirs)

.PHONY: $(dirs)
$(dirs):
	$(MAKE) -C $@ -f ../Makefile.test

.PHONY: clean
clean:
	for i in $(dirs); do $(MAKE) -C $$i -f ../Makefile.test clean ; done
