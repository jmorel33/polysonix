CC = gcc
CFLAGS = -I. -Wall -Wextra -O2
LIBS = -lm

TESTS = test_vm_div_zero test_vm_jump_bounds test_vm_compliance test_lfsr test_security_wave_idx

.PHONY: all clean run_tests

all: $(TESTS)

test_security_wave_idx: test/test_security_wave_idx.c polysonix.h px_patching.h
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

test_vm_div_zero: test/test_vm_div_zero.c px_vm.h
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

test_vm_jump_bounds: test/test_vm_jump_bounds.c px_vm.h
	$(CC) $(CFLAGS) -DPX_VM_IMPLEMENTATION $< -o $@ $(LIBS)

test_vm_compliance: test/test_px_vm_impl.c test/test_px_vm_client.c px_vm.h
	$(CC) $(CFLAGS) -c test/test_px_vm_impl.c -o test_px_vm_impl.o
	$(CC) $(CFLAGS) -c test/test_px_vm_client.c -o test_px_vm_client.o
	$(CC) test_px_vm_impl.o test_px_vm_client.o -o $@ $(LIBS)

test_lfsr: test/test_lfsr.c px_vm.h
	$(CC) $(CFLAGS) -DPX_VM_IMPLEMENTATION $< -o $@ $(LIBS)

run_tests: $(TESTS)
	@echo "Running test_vm_div_zero..."
	@./test_vm_div_zero > /dev/null
	@echo "Running test_vm_jump_bounds..."
	@./test_vm_jump_bounds > /dev/null
	@echo "Running test_vm_compliance..."
	@./test_vm_compliance > /dev/null
	@echo "Running test_lfsr..."
	@./test_lfsr > /dev/null
	@echo "Running test_security_wave_idx..."
	@./test_security_wave_idx > /dev/null
	@echo "All tests passed!"

clean:
	rm -f $(TESTS) *.o
