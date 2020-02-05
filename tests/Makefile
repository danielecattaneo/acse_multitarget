dirs:=arrays sum dowhile fact mult_table power \
	  semi assign arrays2 misc shift

test: $(dirs)

$(dirs):
	$(MAKE) -C $@ -f ../Makefile.test

.PHONY: test clean $(dirs)

clean:
	for i in $(dirs); do $(MAKE) -C $$i -f ../Makefile.test clean ; done
