
VM = ../meld -d -f

all:
	@echo "Type 'make test' to test. Or type 'make compiled' to recompile tests."

test-serial:
	@bash test_all.sh sl

test-mpi:
	@bash test_all.sh mpi

compiled:
	@mkdir -p code
	@meld-compile-directory progs code
	@rm -f code/*.ast code/*.code
