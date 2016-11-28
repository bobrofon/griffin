[![Build Status](https://travis-ci.org/bobrofon/griffin.svg?branch=master)](https://travis-ci.org/bobrofon/griffin)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg?maxAge=2592000)](https://github.com/bobrofon/griffin/blob/master/LICENSE)
# griffin
Griffin - filesystem in userspace. Griffinfs is a proxy for procfs which hide all processes with name 'griffin'.

## But why?
It is a proof of concept that process in userspace can hide himself in linux system.

## dependencies
* libfuse >= 2.6, < 3

## build dependencies
* libfuse >= 2.6, < 3
* gcc >= 4.9

## How to use
Replace original /proc by griffinfs:
```
# make mount
```
Restore original /proc:
```
# make umount
```
