CC := cc
PY := python3
PYC := python3-config
PIP := pip3

all: clean fernet

fernet.c: fernet.pyx
	python3 setup.py build_ext  --inplace

clean:
	-rm -r fernet.c fernet.h *.so build

deps:
	$(PIP) install cpython
	$(PIP) install cryptography

test: test.c fernet.h
	$(CC) $(firstword $^) -o $@ $(shell $(PYC) --cflags) $(shell $(PYC) --embed --ldflags) -L. -lfernet -Wl,-rpath=. -lfernet



.PHONY: clean
