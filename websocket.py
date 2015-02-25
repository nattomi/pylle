###############################################################################
##
##  Copyright (C) 2011-2013 Tavendo GmbH
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
###############################################################################

import sys 
import psycopg2

from twisted.internet import reactor
from twisted.python import log
from twisted.web.server import Site
from twisted.web.static import File

from autobahn.twisted.websocket import WebSocketServerFactory, \
                                       WebSocketServerProtocol, \
                                       listenWS


class BroadcastServerProtocol(WebSocketServerProtocol):

   def onOpen(self):
      self.factory.register(self)

   def onMessage(self, payload, isBinary):
      if not isBinary:
         msg = "{} from {}".format(payload.decode('utf8'), self.peer)
         self.factory.broadcast(msg)

   def connectionLost(self, reason):
      WebSocketServerProtocol.connectionLost(self, reason)
      self.factory.unregister(self)


class BroadcastServerFactory(WebSocketServerFactory):
   """
   Simple broadcast server broadcasting any message it receives to all
   currently connected clients.
   """

   def __init__(self, url, debug = False, debugCodePaths = False):
      WebSocketServerFactory.__init__(self, url, debug = debug, debugCodePaths = debugCodePaths)
      self.clients = []
      self.streamvalue = "some default value"
      self.stream()

   def stream(self):
      cur.execute("""SELECT * FROM live ORDER BY nsec1970 + fracsec/1e6 DESC LIMIT 1""")
      row = cur.fetchone()
      self.streamvalue = '{"x":' + str(int(round((row[1] + row[2]/1e6)*1000))) + ',"y":' + str(row[0]) + '}'
      self.broadcast(self.streamvalue)
      reactor.callLater(5, self.stream)

   def register(self, client):
      if not client in self.clients:
         print("registered client {}".format(client.peer))
         self.clients.append(client)

   def unregister(self, client):
      if client in self.clients:
         print("unregistered client {}".format(client.peer))
         self.clients.remove(client)

   def broadcast(self, msg):
      print("broadcasting message '{}' ..".format(msg))
      for c in self.clients:
         c.sendMessage(msg.encode('utf8'))
         print("message sent to {}".format(c.peer))


class BroadcastPreparedServerFactory(BroadcastServerFactory):
   """
   Functionally same as above, but optimized broadcast using
   prepareMessage and sendPreparedMessage.
   """

   def broadcast(self, msg):
      print("broadcasting prepared message '{}' ..".format(msg))
      preparedMsg = self.prepareMessage(msg)
      for c in self.clients:
         c.sendPreparedMessage(preparedMsg)
         print("prepared message sent to {}".format(c.peer))


if __name__ == '__main__':
   if len(sys.argv) > 1 and sys.argv[1] == 'debug':
      log.startLogging(sys.stdout)
      debug = True
   else:
      debug = False

   try:
      conn = psycopg2.connect("dbname='' user='' host='' password=''")
   except:
      print "I am unable to connect to the database"
   cur = conn.cursor()


   ServerFactory = BroadcastServerFactory
   #ServerFactory = BroadcastPreparedServerFactory

   factory = ServerFactory("ws://localhost:8888",
                           debug = debug,
                           debugCodePaths = debug)

   factory.protocol = BroadcastServerProtocol
   factory.setProtocolOptions(allowHixie76 = True)
   listenWS(factory)

   webdir = File(".")
   web = Site(webdir)
   reactor.listenTCP(8080, web)

   reactor.run()
