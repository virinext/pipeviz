pipeviz
==========

Pipeviz is a graphedit for gstreamer-1.0. This is a gui tool for constructing and testing gstreamer pipelines. 

It allows you:

* to construct the pipelines via the gui interface

* to test different types of pipes easy

* save and open your graphs

Who might be interested in it?

* quality assurance

* technical support

* software engineers


![alt tag](https://cloud.githubusercontent.com/assets/10683398/6396608/94f89e3a-be09-11e4-982c-5bf3a57bc6f4.png)

Pre-requirements:
-----

* qt (4.0 5.0)

* gstreamer-1.0

* pkgconfig



Building:
-----

cd pipeviz

QMAKEFEATURES=. qmake pipeviz.pro

make gitinfo

make



Prebuilt binaries
-----

Prebuilt binaries for windows are available.

[Latest Release](https://github.com/virinext/pipeviz/releases/latest)
