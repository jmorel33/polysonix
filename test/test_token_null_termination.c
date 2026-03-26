#include <stdio.h>
#include <string.h>
#include <assert.h>

#define PX_VM_IMPLEMENTATION
#include "px_vm.h"

int main() {
    printf("Running test_token_null_termination...\n");

    // 1. Test long identifier in tokenizer
    const char* long_ident_expr = "THIS_IS_A_VERY_LONG_IDENTIFIER_THAT_EXCEEDS_THIRTY_ONE_CHARS";
    Token tokens[2];
    int count = tokenize(long_ident_expr, tokens, 2);
    assert(count == 1);
    assert(tokens[0].type == TOKEN_VARIABLE);

    // The macro should have copied 31 chars and null-terminated at 32nd byte
    assert(strlen(tokens[0].value) == 31);
    assert(tokens[0].value[31] == '\0');
    printf("  Tokenizer long identifier: PASSED\n");

    // 2. Test exact 31-char identifier
    const char* exact_31_expr = "A123456789012345678901234567890";
    count = tokenize(exact_31_expr, tokens, 2);
    assert(count == 1);
    assert(strlen(tokens[0].value) == 31);
    assert(tokens[0].value[31] == '\0');
    assert(strcmp(tokens[0].value, exact_31_expr) == 0);
    printf("  Tokenizer 31-char identifier: PASSED\n");

    // 3. Test short identifier
    const char* short_expr = "abc";
    count = tokenize(short_expr, tokens, 2);
    assert(count == 1);
    assert(strlen(tokens[0].value) == 3);
    assert(strcmp(tokens[0].value, "abc") == 0);
    printf("  Tokenizer short identifier: PASSED\n");

    // 4. Test macro directly with dirty buffer
    char buffer[32];
    memset(buffer, 'A', 32);
    PX_STRNCPY_SAFE(buffer, "TOO_LONG_OF_A_STRING_FOR_THIS_BUFFER", 32);
    assert(strlen(buffer) == 31);
    assert(buffer[31] == '\0');
    printf("  PX_STRNCPY_SAFE macro: PASSED\n");

    printf("All test_token_null_termination PASSED!\n");
    return 0;
}
