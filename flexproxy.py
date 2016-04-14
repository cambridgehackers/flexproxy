#!/usr/bin/env python

# Copyright (c) Twisted Matrix Laboratories.
# See LICENSE for details.

from twisted.internet import reactor
from twisted.internet import task
from twisted.internet.defer import Deferred
from twisted.internet.protocol import ClientFactory
from twisted.internet.protocol import Protocol, Factory
from twisted.protocols.basic import LineReceiver

from twisted.internet import reactor, protocol


# a client protocol

class ProxyClient(protocol.Protocol):
    """Once connected, send a message, then print the result."""
    
    def connectionMade(self):
        self.server = self.factory.server
        if self.server.buffer:
            for s in self.server.buffer:
                self.transport.write(s)
            self.server.buffer = []

    def dataReceived(self, data):
        "As soon as any data is received, write it back."
        print("port %d dataReceived from server: %s" % (self.server.factory.port, data))
        self.server.transport.write(data)
        self.server.client = self

    def connectionLost(self, reason):
        print "connection lost"
        self.server.client = None

class ProxyClientFactory(protocol.ClientFactory):
    protocol = ProxyClient

    def __init__(self, server):
        self.server = server

    def clientConnectionFailed(self, connector, reason):
        print "Connection failed:", reason
    
    def clientConnectionLost(self, connector, reason):
        print "Connection lost:", reason


### Protocol Implementation

# This is just about the simplest possible protocol
class ProxyServer(Protocol):
    def connectionMade(self):
        self.buffer = []
        self.client = None
        self.port = self.factory.port
        print("connecting to multiplicty port %d" % self.port)
        # multiplicity.csail.mit.edu
        reactor.connectTCP("128.30.92.238", self.port, ProxyClientFactory(self))

    def connectionLost(self, reason):
        print("connection lost")
        if self.client:
            self.client.transport.loseConnection()

    def dataReceived(self, data):
        """
        As soon as any data is received, write it back.
        """
        if self.client:
            self.client.transport.write(data)
        else:
            self.buffer.append(data)

class ProxyServerFactory(Factory):
    def __init__(self,port):
        self.port=port

def main():
    factories = []
    for port in [1709, 48427, 56403]:
        f = ProxyServerFactory(port)
        f.protocol = ProxyServer
        factories.append(f)
        reactor.listenTCP(port, f)
    reactor.run()

if __name__ == '__main__':
    main()
