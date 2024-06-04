#!/bin/bash

VIEWER_PATH=$(
  cd $(dirname $BASH_SOURCE[0])/
  pwd
)

export CRDC_WS=$VIEWER_PATH/../

if [ -z ${CRDC_EXPORT} ];then
    export CRDC_EXPORT=./
    echo "CRDC_EXPORT(for image dumping path) was not given. Use default path ${CRDC_EXPORT}"
else
    echo "Use given CRDC_EXPORT path: ${CRDC_EXPORT}"
fi

if [ -z ${CRDC_DATA_PACKAGE} ];then
    echo "CRDC_DATA_PACKAGE(for data folder name) enviornment variable was not given. Please set CRDC_DATA_PACKAGE"
    export CRDC_DATA_PACKAGE=./
    echo "CRDC_EXPORT(for image dumping path) was not given. Use default path ${CRDC_EXPORT}"
else
    echo "Use given CRDC_DATA_PACKAGE name: ${CRDC_DATA_PACKAGE}"
fi

$VIEWER_PATH/viewer --alsologtostderr true --stderrthreshold 3 --v 0 --minloglevel 0 --colorlogtostderr true


