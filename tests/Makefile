dirs:=arrays sum dowhile fact mult_table power \
	  semi assign arrays arrays2 misc shift

test :
	for i in $(dirs); do $(MAKE) -C $$i -f ../Makefile.test ; done

.PHONY : test clean

clean :
	for i in $(dirs); do $(MAKE) -C $$i -f ../Makefile.test clean ; done
