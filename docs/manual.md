#### Introduction
This is an introduction to the pylle (**Py**thon **L**ightweight **L**ogging **E**nvironment) data logging daemon. The aim of this software package is to provide a solution for continuous data logging with the [DATAQ DI-149](http://www.dataq.com/products/di-149/) A/D converter. At the time of writing it can be divided into the following parts:

1. The [data logging daemon](#the-data-logging-daemon).
2. An associated [PostgreSQL database](#the-postgresql-database) and its maintenance tools.
3. A [websocket server](#the-websocket-server) for streaming the collected data realtime.
4. A [client application](#the-client-application) based on jQuery/flot for visualizing the stream of the websocket server.

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

##### The PostgreSQL database
The most important feature is that it contains a table called *live*, which is for storing the most recent data:
```
CREATE TABLE live (
    ch integer,
    nsec1970 integer,
    fracsec integer
);
```
Column *ch* holds the data in digitization units (an integer in the interval [-1024,1024)), columns *nsec1970* and *fracsec* hold the appropriate timestamp (decomposed into a UNIX epoch + microseconds). This table is only for storing the most recent data, therefore we delete records older then a certain date regularly using a [cronjob](../db_maintenance/archiveAndClean.R). This cronjob creates additional tables named *dayYYYYmmdd*. The purpose of creating them is to have an easy, searchable way to access daily data. After a while those tables should be deleted [too](../db_maintenance/sqldel.R).

##### The websocket server
The [websocket server](../websocket.py) uses [AutobahnPython](https://github.com/tavendo/AutobahnPython) and is a slight modification of their [broadcast](https://github.com/tavendo/AutobahnPython/tree/master/examples/twisted/websocket/broadcast) example. Momentarily it broadcasts the latest entry of the live table of the database as a JSON string to all connected clients. Its operation is completely independent from the data logging daemon -- in fact, it can be even hosted on a different PC. This might not be the most practical way to implement the service. It would also make sense to trigger data broadcasts via the buffer readouts of pylle. The websocket server can be configured for automatic (re)start via adding the following line to ```/etc/inittab```:
```
i1:2:respawn:python /home/bin/websocket.py
```

##### The client application
The client application is a realtime visualization of the data broadcasted by the websocket server using jQuery/flot. It uses the following javascript libraries:
```
<script type="text/javascript" src="jquery/jquery.min.js"></script>
<script type="text/javascript" src="flot/jquery.flot.min.js"></script>
<script type="text/javascript" src="flot/jquery.flot.time.min.js"></script>
```
The easiest way to get the jquery and flot libraries under Debian is by installing the ```libjs-jquery``` and ```libjs-flot``` packages. Here is the [javascript](../client/plot.js) for the application. A production example can be found at the [homepage for atmospheric eletricity recordings](http://aero.nck.ggki.hu) in Nagycenk Observatory.
