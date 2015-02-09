#### Introduction

This is an introduction to the pylle (**Py**thon **L**ightweight **L**ogging **E**nvironment) data logging daemon. The aim of this software package is to provide a solution for continuous data logging with the [DATAQ DI-149](http://www.dataq.com/products/di-149/) A/D converter. At the time of writing it can be divided into the following parts:

1. The [data logging daemon](##### The data logging daemon).
2. An associated PostgreSQL database and its maintenance tools.
3. A websocket server for streaming the collected data realtime.
4. A client application based on jQuery/flot for visualizing the stream of the websocket server.

##### The data logging deamon




