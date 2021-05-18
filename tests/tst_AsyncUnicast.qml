import QtQuick 2.3
import QtTest 1.3

import NetUdp 1.0 as NetUdp

TestCase
{
  name: "Async Unicast"

  NetUdp.Socket
  {
    id: txSocket
    useWorkerThread: true
    Component.onCompleted: () => start()
  }

  NetUdp.Socket
  {
    id: rxSocket
    useWorkerThread: true
    Component.onCompleted: () => start("127.0.0.1", 9993)
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
  }

  function test_sendDatagramAsString()
  {
    waitForBounded()

    txSocket.sendDatagram(
    {
      address: "127.0.0.1",
      port: 9993,
      data: "My Data Async"
    })

    spyRxSocketReceived.wait()

    const datagram = spyRxSocketReceived.signalArguments[0][0]

    compare(datagram.data, "My Data Async")
    compare(datagram.destinationAddress, "127.0.0.1")
    compare(datagram.destinationPort, 9993)
  }
}
