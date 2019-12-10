/**
 * Copyright (C) Olivier Le Doeuff 2019
 * Contact: olivier.ldff@gmail.com
 */

// Qt
import QtQuick 2.12

// Backend
import Qaterial 1.0 as Qaterial
import Stringify 1.0 as Stringify
import NetUdp 1.0 as NetUdp

Qaterial.DebugObject
{
    id: root
    property NetUdp.Server object: null
    title: "" + (root.object ? root.object : "Server null")

    content: Column
    {
        Qaterial.Label
        {
            text: "isRunning : " + (root.object ? root.object.isRunning : "")
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }
        Qaterial.Label
        {
            text: "isBounded : " + (root.object ? root.object.isBounded : "")
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }
        Qaterial.Label
        {
            //visible: root.object && root.object.error
            //text: "error : " + (root.object ? root.object.error : "")
            id: _errorLabel
            Connections
            {
                target: root.object
                onSocketError: _errorLabel.text = description
                onIsBoundedChanged: if(isBounded) _errorLabel.text = ""
            }
            color: "red"
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            textType: Qaterial.Style.TextType.Caption
            highlighted: false
            backgroundImplicitHeight: 20
            text: "watchdogPeriodMs : " + (root.object ? root.object.watchdogPeriodMs : "")
            onClicked: if(root.object) dialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    root.object.watchdogPeriodMs = result
                },
                text: root.object.watchdogPeriodMs,
                title: qsTr("Enter watchdogPeriodMs"),
                textTitle: qsTr("watchdogPeriodMs"),
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
            textType: Qaterial.Style.TextType.Caption
            highlighted: false
            backgroundImplicitHeight: 20
            text: "address : " + (root.object ? root.object.address : "")
            onClicked: if(root.object) dialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    if(acceptableInput)
                        root.object.address = result
                    else
                        snackbarManager.show({text : result + " isn't an ipv4 address"})
                },
                text: root.object.address,
                title: qsTr("Enter Bind Ip Address"),
                textTitle: qsTr("Ip"),
                helperText: "Should be 0.0.0.0 for multicast input",
                validator: Stringify.Ipv4RegexValidator,
                inputMethodHints: Qt.ImhFormattedNumbersOnly,
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }

        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            textType: Qaterial.Style.TextType.Caption
            highlighted: false
            backgroundImplicitHeight: 20
            text: "port : " + (root.object ? root.object.port : "")
            onClicked: if(root.object) dialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    root.object.port = result
                },
                text: root.object.port,
                title: qsTr("Enter Bind Listening port"),
                textTitle: qsTr("Listening port"),
                helperText: "Between 0 and 65535",
                inputMethodHints: Qt.ImhFormattedNumbersOnly,
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }

        Qaterial.Label
        {
            text: "multicastGroups : " + (root.object ? root.object.multicastGroups : "")
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }
        Row
        {
            Qaterial.FlatButton
            {
                topInset: 0
                bottomInset: 0
                textType: Qaterial.Style.TextType.Caption
                backgroundImplicitHeight: 20
                text: "Add Multicast address"
                onClicked: if(root.object) dialogManager.openTextField({
                    acceptedCallback: function(result, acceptableInput)
                    {
                        if(acceptableInput)
                        {
                            if(!root.object.joinMulticastGroup(result))
                            {
                                snackbarManager.show({text : "Fail to join " + result})
                            }
                        }
                        else
                            snackbarManager.show({text : result + " isn't an ipv4 address"})
                    },
                    title: qsTr("Enter New ip address to join"),
                    textTitle: qsTr("Ip"),
                    validator: Stringify.Ipv4RegexValidator,
                    inputMethodHints: Qt.ImhFormattedNumbersOnly,
                    selectAllText: true,
                    standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
                })
            }
            Qaterial.FlatButton
            {
                topInset: 0
                bottomInset: 0
                textType: Qaterial.Style.TextType.Caption
                backgroundImplicitHeight: 20
                text: "Remove Multicast address"
                onClicked: if(root.object) dialogManager.openTextField({
                    acceptedCallback: function(result, acceptableInput)
                    {
                        if(acceptableInput)
                        {
                            if(!root.object.leaveMulticastGroup(result))
                            {
                                snackbarManager.show({text : "Fail to leave " + result})
                            }
                        }
                        else
                            snackbarManager.show({text : result + " isn't an ipv4 address"})
                    },
                    title: qsTr("Enter ip address to leave"),
                    textTitle: qsTr("Ip"),
                    validator: Stringify.Ipv4RegexValidator,
                    inputMethodHints: Qt.ImhFormattedNumbersOnly,
                    selectAllText: true,
                    standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
                })
            }
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            textType: Qaterial.Style.TextType.Caption
            highlighted: false
            backgroundImplicitHeight: 20
            text: "multicastInterfaceName : " + (root.object ? root.object.multicastInterfaceName : "")
            onClicked: if(root.object) dialogManager.openTextField({
                acceptedCallback: function(result, acceptableInput)
                {
                    root.object.multicastInterfaceName = result
                },
                text: root.object.multicastInterfaceName,
                title: qsTr("Enter Output multicastInterfaceName"),
                textTitle: qsTr("Output multicastInterfaceName"),
                selectAllText: true,
                standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
            })
        }
        Qaterial.SwitchButton
        {
            text: "multicastLoopback"
            checked: root.object && root.object.multicastLoopback
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
            onCheckedChanged:
            {
                if(root.object && root.object.multicastLoopback !== checked)
                    root.object.multicastLoopback = checked
            }
        }

        Qaterial.Label
        {
            text: "rxBytesPerSeconds : " + (root.object ? root.object.rxBytesPerSeconds : "")
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }

        Qaterial.Label
        {
            text: "txBytesPerSeconds : " + (root.object ? root.object.txBytesPerSeconds : "")
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }

        Qaterial.Label
        {
            text: "rxBytesTotal : " + (root.object ? root.object.rxBytesTotal : "")
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }

        Qaterial.Label
        {
            text: "txBytesTotal : " + (root.object ? root.object.txBytesTotal : "")
            width: parent.width
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
        }

        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            textType: Qaterial.Style.TextType.Caption
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
    }
}
