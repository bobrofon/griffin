[![Build Status](https://travis-ci.org/bobrofon/griffin.svg?branch=master)](https://travis-ci.org/bobrofon/griffin)
# griffin
Griffin - filesystem in userspace. Griffinfs is a proxy for procfs which hide all processes with name 'griffin'.

## But why?
It is a proof of concept that process in userspace can hide himself in linux system.

## dependencies
* libfuse >= 2.6, < 3

## How to use
Replace original /proc by griffinfs:
```
# make mount
```
Restore original /proc:
```
# make umount
```
