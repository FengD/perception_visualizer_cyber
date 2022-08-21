#!/bin/bash

DAYDREAM_PATH=$(
  cd $(dirname $BASH_SOURCE[0])/
  pwd
)

export CRDC_WS=$DAYDREAM_PATH/../

$DAYDREAM_PATH/viewer


