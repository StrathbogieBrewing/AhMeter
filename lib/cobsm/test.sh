#!/bin/bash
set -e

gcc cobsm_test.c cobsm.c utils.c -o cobsm_test
./cobsm_test
