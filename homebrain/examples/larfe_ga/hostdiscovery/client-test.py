#!/usr/bin/python2
# -*- coding: utf-8 -*-

import sys
import socket

SSDP_ADDR = "239.255.255.250";
SSDP_PORT = 1900;

ssdpRequest = "M-SEARCH * lehb"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(ssdpRequest, (SSDP_ADDR, SSDP_PORT))
print sock.recv(4000)

