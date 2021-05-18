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
    onDatagramReceived: console.log("yolo")
    Component.onCompleted: () => start("127.0.0.1", 9991)
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

    compare(rxSocket.isBounded, true)

    compare(txSocket.sendDatagram(
    {
      port: 9991,
      data: "My Data Unicast"
    }), false)

    compare(txSocket.sendDatagram(
    {
      address: "127.0.0.1",
      data: "My Data Unicast"
    }), false)

    compare(txSocket.sendDatagram(
    {
      address: "127.0.0.1",
      port: 9991,
    }), false)

    compare(txSocket.sendDatagram(
    {
      address: "127.0.0.1",
      port: 9991,
      data: "My Data Unicast"
    }), true)

    spyRxSocketReceived.wait()

    const datagram = spyRxSocketReceived.signalArguments[0][0]

    compare(datagram.data, "My Data Unicast")
    compare(datagram.destinationAddress, "127.0.0.1")
    compare(datagram.destinationPort, 9991)
  }
}
