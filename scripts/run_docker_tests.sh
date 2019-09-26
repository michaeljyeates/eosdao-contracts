#!/usr/bin/env bash

TAG=v1

docker build -t eosdao/contracts:$TAG .
docker run -t eosdao/contracts:$TAG
