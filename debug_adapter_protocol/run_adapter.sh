#!/usr/bin/env bash
set -e
# Start the adapter under debugpy so the VS Code debugger can attach.
# Listens on port 5678 and waits for the debugger to attach.
export PYTHONPATH=./src
poetry run python -m debugpy --listen 5678 --wait-for-client src/dap.py --protocol ws --host 0.0.0.0 --port 4711