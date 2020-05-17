#!/usr/local/bin/perl

$str = "Hello, kernel!";
syscall(210, $str);
