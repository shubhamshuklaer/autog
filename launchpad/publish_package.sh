#!/bin/bash

# exit when any command fails
set -e
cd build
dput ppa:shubhamshuklaerssss/autog autog_*-1_source.changes
