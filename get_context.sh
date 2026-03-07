#!/bin/bash
echo "Looking for TK_SELECT"
grep -n "TK_SELECT" px_vm.h || echo "Not found"
echo "Looking for OP_SELECT"
grep -n "OP_SELECT" px_vm.h | grep -v SUB_LABEL | grep -v LABEL_OP
