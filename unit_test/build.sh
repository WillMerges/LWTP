#!/bin/bash

gcc ../lwtp.c test.c -I.. -pthread -o unit_test -ggdb
gcc ../lwtp.c benchmark.c -I.. -pthread -o benchmark -ggdb
