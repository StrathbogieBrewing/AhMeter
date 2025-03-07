#!/bin/bash
set -e

source venv/bin/activate

export INFLUXDB_TOKEN=ykSIe2SkoQW2jA2mmfdRLLlIHeLHqYE3l3nTxhwQS5_cQc78d9GSTGfj6z_MY2lDinetLQgcRuW2drLu8p2XcA==

nohup python3 host/tinbus.py &
# python3 host/tinbus.py 