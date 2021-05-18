import QtQuick 2.0
import QtQuick.Controls 2.0

import Qaterial 1.0 as Qaterial
import NetUdp 1.0 as NetUdp
import NetUdp.Debug 1.0 as NetUdpDebug

ApplicationWindow
{
  id: root
  width: 800
  height: 600

  property
  var listenedInterfaces: []
  property
  var outputInterfaces: []

  Qaterial.ScrollablePage
  {
    anchors.fill: parent
    Column
    {
      spacing: 4

      Button
      {
        text: "send unicast"
        onClicked: () => txSocket.sendDatagram(
        {
          address: "127.0.0.1",
          port: 9999,
          data: "My Data"
        })
      }

      Button
      {
        text: "send binary unicast"
        onClicked: () => txSocket.sendDatagram(
        {
          address: "127.0.0.1",
          port: 9999,
          data: [0, 1, 2, 3]
        })
      }

      NetUdp.Socket
      {
        id: txSocket

        Component.onCompleted: () => start()
      }

      NetUdp.Socket
      {
        id: rxSocket

        onDatagramReceived: function(datagram)
        {
          console.log(`datagram : ${JSON.stringify(datagram)}`)
          console.log(`datagram.data (string) : "${datagram.data}"`)
          let byteArray = []
          for(let i = 0; i < datagram.data.length; ++i)
          {
            //console.log(`datagram.data.charCodeAt[${i}]:${datagram.data.charCodeAt(i)}`)
            byteArray.push(datagram.data.charCodeAt(i))
          }
          console.log(`datagram.data (bytes): [${byteArray}]`)
          console.log(`datagram.destinationAddress : ${datagram.destinationAddress}`)
          console.log(`datagram.destinationPort : ${datagram.destinationPort}`)
          console.log(`datagram.senderAddress : ${datagram.senderAddress}`)
          console.log(`datagram.senderPort : ${datagram.senderPort}`)
          console.log(`datagram.ttl : ${datagram.ttl}`)
        }

        Component.onCompleted: () => start("127.0.0.1", 9999)
      }

      Button
      {
        text: "Fetch interfaces"
        onClicked: () => NetUdp.InterfacesProvider.fetchInterfaces()
        Component.onCompleted: () => NetUdp.InterfacesProvider.fetchInterfaces()
      }

      Label
      {
        text: `listened interfaces : ${listenedInterfaces}`
      }

      Label
      {
        text: `output interfaces : ${outputInterfaces}`
      }

      Row
      {
        Column
        {
          Repeater
          {
            model: NetUdp.InterfacesProvider.interfaces

            delegate: SwitchDelegate
            {
              width: 300
              text: `listen ${modelData}`
              checked: listenedInterfaces.indexOf(modelData) >= 0
              onClicked: function()
              {
                if(listenedInterfaces.indexOf(modelData) >= 0)
                {
                  if(!checked)
                  {
                    listenedInterfaces = listenedInterfaces.filter(e => e !== modelData)
                  }
                }
                else
                {
                  if(checked)
                  {
                    let copy = listenedInterfaces
                    copy.push(modelData)
                    listenedInterfaces = copy
                  }
                }
              }
            }
          }
        } // Column

        Column
        {
          Repeater
          {
            model: NetUdp.InterfacesProvider.interfaces

            delegate: SwitchDelegate
            {
              width: 300
              text: `Send to ${modelData}`
              checked: outputInterfaces.indexOf(modelData) >= 0
              onClicked: function()
              {
                if(outputInterfaces.indexOf(modelData) >= 0)
                {
                  if(!checked)
                  {
                    outputInterfaces = outputInterfaces.filter(e => e !== modelData)
                  }
                }
                else
                {
                  if(checked)
                  {
                    let copy = outputInterfaces
                    copy.push(modelData)
                    outputInterfaces = copy
                  }
                }
              }
            }
          }
        } // Column
      }

      NetUdp.Socket
      {
        id: multicastSocket

        multicastListeningInterfaces: root.listenedInterfaces
        multicastOutgoingInterfaces: root.outputInterfaces

        multicastGroups: ["239.1.3.4"]
        multicastLoopback: true

        onDatagramReceived: function(datagram)
        {
          console.log(`datagram : ${JSON.stringify(datagram)}`)
          console.log(`datagram.data (string) : "${datagram.data}"`)
          let byteArray = []
          for(let i = 0; i < datagram.data.length; ++i)
          {
            //console.log(`datagram.data.charCodeAt[${i}]:${datagram.data.charCodeAt(i)}`)
            byteArray.push(datagram.data.charCodeAt(i))
          }
          console.log(`datagram.data  (bytes): [${byteArray}]`)
          console.log(`datagram.destinationAddress : ${datagram.destinationAddress}`)
          console.log(`datagram.destinationPort : ${datagram.destinationPort}`)
          console.log(`datagram.senderAddress : ${datagram.senderAddress}`)
          console.log(`datagram.senderPort : ${datagram.senderPort}`)
          console.log(`datagram.ttl : ${datagram.ttl}`)
        }

        // Listen port 1234
        Component.onCompleted: () => start(1234)
      }

      Button
      {
        text: "send multicast"
        onClicked: () => multicastSocket.sendDatagram(
        {
          address: "239.1.3.4",
          port: 1234,
          data: "Multicast Data"
        })
      }

      NetUdpDebug.Socket
      {
        title: "txSocket"
        object: txSocket
        width: root.width
      }
      NetUdpDebug.Socket
      {
        title: "rxSocket"
        object: rxSocket
        width: root.width
      }
      NetUdpDebug.Socket
      {
        title: "multicastSocket"
        object: multicastSocket
        width: root.width
      }

    }
  }

  Component.onCompleted: () => visible = true
}
