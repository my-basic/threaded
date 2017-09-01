## Introduction

This is an example of how to use MY-BASIC with multiple threads.

Read the [code and comments](threaded.c) for usage.

For info about the interpreter itself, see [https://github.com/paladin-t/my_basic/](https://github.com/paladin-t/my_basic/).

## Known issues

1. I have disabled `MB_ENABLE_UNICODE` and `MB_ENABLE_UNICODE_ID`, otherwise calling `setlocale` in `_print_string` may cause deadlocks with some systems.
2. It's not fully supported to run forked instances with multiple threads, cannot use referenced GC types in code, although simple data types are OK. If you are not sure about this, just don't use `mb_fork`.
