#!/bin/bash

rm -rf twister-out*

west twister -p native_sim -T tests/ -v --show_output
