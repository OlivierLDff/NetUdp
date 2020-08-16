import QtQuick 2.3
import QtTest 1.3

import NetUdp 1.0 as NetUdp

TestCase
{
  name: "Multicast"

  NetUdp.Socket
  {
    id: txSocket

    // Should be true on unix systems
    multicastLoopback: true

    Component.onCompleted: () => start()
  }

  NetUdp.Socket
  {
    id: rxSocket

    multicastGroups: [ "239.7.8.9" ]
    // Should be true on windows systems
    multicastLoopback: true

    Component.onCompleted: () => start(7891)
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

    // Wait for multicast group to be joined
    wait(100)
  }

  function test_sendDatagramAsString()
  {
    waitForBounded()

    compare(rxSocket.isBounded, true)
    txSocket.sendDatagram({
      address: "239.7.8.9",
      port: 7891,
      data: "My Data Multicast"
    })

    spyRxSocketReceived.wait()

    const datagram = spyRxSocketReceived.signalArguments[0][0]

    compare(datagram.data, "My Data Multicast")
  }
}
