all:
	@gcc -fPIC -shared -o libfoo.so foo.c
	@gcc sl_loader.c; ./a.out

clean:
	@rm -f libfoo.so a.out
