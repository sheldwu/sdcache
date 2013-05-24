#!/bin/sh
ps aux|grep ddcache|awk '{if(NR==2){print $2;}}'|xargs kill -9
