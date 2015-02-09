#### Introduction

This is an introduction to the pylle (**Py**thon **L**ightweight **L**ogging **E**nvironment) data logging daemon. The aim of this software package is to provide a solution for continuous data logging with the [DATAQ DI-149](http://www.dataq.com/products/di-149/) A/D converter. At the time of writing it can be divided into the following parts:

1. The [data logging daemon](##### The data logging daemon).
2. An associated PostgreSQL database and its maintenance tools.
3. A websocket server for streaming the collected data realtime.
4. A client application based on jQuery/flot for visualizing the stream of the websocket server.

##### The data logging deamon

The data logging daemon [pylle](../pylle) is the heart of the package. The principles of operation is as follows:

1. Get a timestamp.
2. Run for some seconds and then read out the content of the buffer of the A/D converter.
3. Dump the timestamp and the content of the buffer into a text file.
4. (optional) Trigger a separate process for doing additional processing on the buffer content (such as uploading it into a database table, see for example the R script [insert.R](../insert.R).
5. Go to 1.

The daemon can be started via
```pylle start```
and stopped via
```pylle stop```. A sample [init.d](../pylle.initd) script is also included for being able to handle pylle as a standard Debian service.




