# OpenFlow protocol software switch implementation

[![BSD license](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build Status](https://travis-ci.org/w180112/OpenflowSoftwareSWitch.svg?branch=master)](https://travis-ci.org/w180112/OpenflowSoftwareSWitch)

This is a small OpenFlow protocol based software switch implementation in C.

## System required:

1. Linux OS
2. RAM size larger than 2 GiB
3. x86 system or ARM system with little endian

## How to use:

Git clone this repository

	# git clone https://github.com/w180112/OpenflowSoftwareSWitch.git

Type

	# cd OpenflowSoftwareSWitch

Run

	# make

to compile

Then

	# ./osw <OpenFlow NIC name> <SDN controller ip>

If OpenFlow connection is established, a simple console interface will show

	OSW>

Try to type

	OSW> addbr br0

Then 

	OSW> addif br0 <NIC interface name>

If the prompt is missing, just press Enter.

To remove the binary files

	# make clean

## Note:

1. This implementation is under OpenFlow protocol 1.3
2. In Flowmod, I only implementation Flowmod type - "add flow" so far.

## Test environment:

1. CentOS 7.6 and AMD EPYC 7401P, 256GB ram server
2. Raspbain Feb. 2020 on RaspberryPi 4B v1.2
3. Can connect to Ryu SDN controller with simple_switch_13.py application

## TODO:

1. implementation Flowmod type - "delete flow" and flow timeout.
2. del bridge/port and make bridge can be joined into socket operation