import QtQuick 2.3
import QtTest 1.3

import NetUdp 1.0 as NetUdp

TestCase
{
  name: "Unicast"

  NetUdp.Socket
  {
    id: txSocket
    Component.onCompleted: () => start()
  }

  NetUdp.Socket
  {
    id: rxSocket
    Component.onCompleted: () => start("127.0.0.1", 9992)
  }

  SignalSpy
  {
    id: spyTxSocketBounded
    target: txSocket
    signalName: "isBoundedChanged"
  }

  SignalSpy
  {
    id: spyRxSocketBounded
    target: rxSocket
    signalName: "isBoundedChanged"
  }

  SignalSpy
  {
    id: spyRxSocketReceived
    target: rxSocket
    signalName: "datagramReceived"
  }

  function waitForBounded()
  {
    if(!txSocket.isBounded)
      spyTxSocketBounded.wait()
    if(!rxSocket.isBounded)
      spyRxSocketBounded.wait()
  }

  function test_sendDatagramAsString()
  {
    waitForBounded()

    txSocket.sendDatagram({
      address: "127.0.0.1",
      port: 9992,
      data: [77,121,32,68,97,116,97]
    })

    spyRxSocketReceived.wait()

    const datagram = spyRxSocketReceived.signalArguments[0][0]

    compare(datagram.data, "My Data")
    compare(datagram.destinationAddress, "127.0.0.1")
    compare(datagram.destinationPort, 9992)
  }
}
