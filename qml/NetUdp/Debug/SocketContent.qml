/**
 * Copyright (C) Olivier Le Doeuff 2019
 * Contact: olivier.ldff@gmail.com
 */

import QtQuick 2.12

import Qaterial 1.0 as Qaterial
import NetUdp 1.0 as NetUdp

Column
{
    id: root
    property NetUdp.Socket object: null

    RegExpValidator
    {
        id: _ipRegEx
        regExp: /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
    }

    IntValidator
    {
        id: _ttlValidator
        bottom: 1
        top: 255
    }

    IntValidator
    {
        id: _portValidator
        bottom: 0
        top: 65535
    }

    Qaterial.LabelCaption
    {
        text: "isRunning : " + (root.object ? root.object.isRunning : "")
        width: parent.width
        elide: Text.ElideRight
        color: root.object && root.object.isRunning ? Qaterial.Style.green : Qaterial.Style.red
    }
    Qaterial.LabelCaption
    {
        text: "isBounded : " + (root.object ? root.object.isBounded : "")
        width: parent.width
        elide: Text.ElideRight
        color: root.object && root.object.isBounded ? Qaterial.Style.green : Qaterial.Style.red
    }
    Qaterial.LabelCaption
    {
        id: _errorLabel
        Connections
        {
            target: root.object
            function onSocketError() { _errorLabel.text = description }
            function onIsBoundedChanged() { if(isBounded) _errorLabel.text = "" }
        }
        color: "red"
        width: parent.width
        elide: Text.ElideRight
    }
    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        font: Qaterial.Style.textTheme.caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "watchdogPeriod : " + (root.object ? root.object.watchdogPeriod : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.watchdogPeriod = result
            },
            text: root.object.watchdogPeriod,
            title: qsTr("Enter watchdogPeriod"),
            textTitle: qsTr("watchdogPeriod"),
            helperText: "In Ms",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        font: Qaterial.Style.textTheme.caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "rxAddress : " + (root.object ? root.object.rxAddress : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                if(acceptableInput)
                    root.object.rxAddress = result
                else
                    Qaterial.SnackbarManager.show({text : result + " isn't an ipv4 rxAddress"})
            },
            text: root.object.rxAddress,
            title: qsTr("Enter Bind Ip Address"),
            textTitle: qsTr("Ip"),
            helperText: "Should be 0.0.0.0 for multicast input",
            validator: _ipRegEx,
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        font: Qaterial.Style.textTheme.caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "rxPort : " + (root.object ? root.object.rxPort : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.rxPort = result
            },
            text: root.object.rxPort,
            title: qsTr("Enter Bind Listening port"),
            textTitle: qsTr("Listening port"),
            helperText: "Between 0 and 65535",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            validator: _portValidator,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        font: Qaterial.Style.textTheme.caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "txPort : " + (root.object ? root.object.txPort : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.txPort = result
            },
            text: root.object.txPort,
            title: qsTr("Enter Bind Listening port"),
            textTitle: qsTr("Listening port"),
            helperText: "Between 0 and 65535",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            validator: _portValidator,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.LabelCaption
    {
        text: "multicastGroups : " + (root.object ? root.object.multicastGroups.toString() : "")
        width: parent.width
        elide: Text.ElideRight
    }
    Row
    {
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: "Add Multicast address"
            onClicked: if(root.object) Qaterial.DialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    if(acceptableInput)
                    {
                        if(!root.object.joinMulticastGroup(result))
                        {
                            Qaterial.SnackbarManager.show({text : "Fail to join " + result})
                        }
                    }
                    else
                        Qaterial.SnackbarManager.show({text : result + " isn't an ipv4 address"})
                },
                title: qsTr("Enter New ip address to join"),
                textTitle: qsTr("Ip"),
                validator: _ipRegEx,
                inputMethodHints: Qt.ImhFormattedNumbersOnly,
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: "Remove Multicast address"
            onClicked: if(root.object) Qaterial.DialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    if(acceptableInput)
                    {
                        if(!root.object.leaveMulticastGroup(result))
                        {
                            Qaterial.SnackbarManager.show({text : "Fail to leave " + result})
                        }
                    }
                    else
                        Qaterial.SnackbarManager.show({text : result + " isn't an ipv4 address"})
                },
                title: qsTr("Enter ip address to leave"),
                textTitle: qsTr("Ip"),
                validator: _ipRegEx,
                inputMethodHints: Qt.ImhFormattedNumbersOnly,
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }
    }
    Qaterial.Label
    {
        text: "multicastInterfaces : " + (root.object ? root.object.multicastListeningInterfaces.toString() : "")
        width: parent.width
        elide: Text.ElideRight
        font: Qaterial.Style.textTheme.caption
    }
    Row
    {
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: "Add Multicast Listening interface"
            onClicked: if(root.object) Qaterial.DialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    if(!root.object.joinMulticastInterface(result))
                        Qaterial.SnackbarManager.show({text : "Fail to join " + result})
                },
                title: qsTr("Enter interface to join"),
                textTitle: qsTr("Interface"),
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: "Remove Multicast Listening interface"
            onClicked: if(root.object) Qaterial.DialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    if(!root.object.leaveMulticastInterface(result))
                        Qaterial.SnackbarManager.show({text : "Fail to remove " + result})
                },
                title: qsTr("Enter interface to remove"),
                textTitle: qsTr("Interface"),
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }
    }
    Qaterial.Label
    {
        text: "multicastOutgoingInterfaces : " + (root.object ? root.object.multicastOutgoingInterfaces.toString() : "")
        width: parent.width
        elide: Text.ElideRight
        font: Qaterial.Style.textTheme.caption
    }
    Row
    {
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: "Add Multicast Output interface"
            onClicked: if(root.object) Qaterial.DialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                  let arr = root.object.multicastOutgoingInterfaces
                  arr.push(result)
                  root.object.multicastOutgoingInterfaces = arr
                },
                title: qsTr("Enter interface to output on"),
                textTitle: qsTr("Interface"),
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: "Remove Multicast Output interface"
            onClicked: if(root.object) Qaterial.DialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    root.object.multicastOutgoingInterfaces = root.object.multicastOutgoingInterfaces.filter(e => e !== result);
                },
                title: qsTr("Enter interface to remove"),
                textTitle: qsTr("Interface"),
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }
    }

    Row
    {
        width: parent.width
        Qaterial.SwitchButton
        {
            text: "loopback"
            implicitHeight: 32
            checked: root.object && root.object.multicastLoopback
            implicitWidth: parent.width/2
            font: Qaterial.Style.textTheme.caption
            onClicked: if(root.object) root.object.multicastLoopback = checked
        }

        Qaterial.SwitchButton
        {
            text: "input"
            implicitHeight: 32
            checked: root.object && root.object.inputEnabled
            implicitWidth: parent.width/2
            font: Qaterial.Style.textTheme.caption
            onClicked: if(root.object) root.object.inputEnabled = checked
        }

    } // Row

    Row
    {
        width: parent.width
        Qaterial.SwitchButton
        {
            text: "2 sockets"
            implicitHeight: 32
            checked: root.object && root.object.separateRxTxSockets
            implicitWidth: parent.width/2
            font: Qaterial.Style.textTheme.caption
            onClicked: if(root.object) root.object.separateRxTxSockets = checked
        }

        Qaterial.SwitchButton
        {
            text: "worker thread"
            implicitHeight: 32
            checked: root.object && root.object.useWorkerThread
            implicitWidth: parent.width/2
            font: Qaterial.Style.textTheme.caption
            onClicked: if(root.object) root.object.useWorkerThread = checked
        }
    } // Row

    Row
    {
        spacing: 10
        Column
        {
            Qaterial.Label
            {
                text: "rxBytesPerSeconds : " + (root.object ? Qaterial.DataFormat.formatBytes(root.object.rxBytesPerSeconds) : "")
                font: Qaterial.Style.textTheme.caption
            }

            Qaterial.Label
            {
                text: "txBytesPerSeconds : " + (root.object ? Qaterial.DataFormat.formatBytes(root.object.txBytesPerSeconds) : "")
                font: Qaterial.Style.textTheme.caption
            }

            Qaterial.Label
            {
                text: "rxBytesTotal : " + (root.object ? Qaterial.DataFormat.formatBytes(root.object.rxBytesTotal) : "")
                font: Qaterial.Style.textTheme.caption
            }

            Qaterial.Label
            {
                text: "txBytesTotal : " + (root.object ? Qaterial.DataFormat.formatBytes(root.object.txBytesTotal) : "")
                font: Qaterial.Style.textTheme.caption
            }
        }

        Column
        {
            Qaterial.Label
            {
                text: "rxPacketsPerSeconds : " + (root.object ? root.object.rxPacketsPerSeconds : "")
                font: Qaterial.Style.textTheme.caption
            }

            Qaterial.Label
            {
                text: "txPacketsPerSeconds : " + (root.object ? root.object.txPacketsPerSeconds : "")
                font: Qaterial.Style.textTheme.caption
            }

            Qaterial.Label
            {
                text: "rxPacketsTotal : " + (root.object ? root.object.rxPacketsTotal : "")
                font: Qaterial.Style.textTheme.caption
            }

            Qaterial.Label
            {
                text: "txPacketsTotal : " + (root.object ? root.object.txPacketsTotal : "")
                font: Qaterial.Style.textTheme.caption
            }
        }
    }

    Row
    {
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: root.object && root.object.isRunning ? "Stop" : "Start"
            onClicked:
            {
                if(root.object)
                {
                    if(root.object.isRunning)
                        root.object.stop()
                    else
                        root.object.start()
                }
            }
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: root.object && "Restart"
            enabled: root.object && root.object.isRunning
            onClicked:
            {
                if(root.object)
                {
                    root.object.restart()
                }
            }
        }
    }
}
