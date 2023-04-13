#!/bin/sh

export GUROBI_HOME="/opt/gurobi605/linux64"
echo $GUROBI_HOME
export PATH=$PATH:$GUROBI_HOME/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GUROBI_HOME/lib
export GRB_LICENSE_FILE="/home/ahmad/gurobi.lic"

sudo ip link set dev ens37 down
sudo ip link set dev ens37 address 00:0c:00:22:72:36
sudo ip link set dev ens37
ip link show ens37


