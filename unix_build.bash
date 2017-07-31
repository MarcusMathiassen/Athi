#!/usr/bin/env bash

output=-o\ athi

wlvl=-Wall\ -Wno-unused-function
compiler=clang++
std=-std=c++1z
olvl=-O3

src=./src/*.cpp
libs=-lglew\ -lglfw\ -framework\ OpenGL

start=`date +%s`
$compiler $src $libs  $olvl $wlvl $output $std
end=`date +%s`

echo Compilation finished. Time: $((end-start)) seconds
