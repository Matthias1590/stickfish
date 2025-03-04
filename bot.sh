#!/bin/bash

set -xe
make
cd lichess-bot
set +x
source ./venv/bin/activate
set -x
python3 lichess-bot.py -u
