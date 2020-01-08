/**
 * Copyright (C) Olivier Le Doeuff 2019
 * Contact: olivier.ldff@gmail.com
 */

// Qt
import QtQuick 2.12

// Backend
import Qaterial 1.0 as Qaterial
import Stringify.Validator 1.0 as StringifyValidator
import NetUdp 1.0 as NetUdp

Column
{
    id: root
    property NetUdp.Server object: null
    Qaterial.Label
    {
        text: "isRunning : " + (root.object ? root.object.isRunning : "")
        width: parent.width
        elide: Text.ElideRight
        textType: Qaterial.Style.TextType.Caption
        color: root.object && root.object.isRunning ? Qaterial.Style.green : Qaterial.Style.red
    }
    Qaterial.Label
    {
        text: "isBounded : " + (root.object ? root.object.isBounded : "")
        width: parent.width
        elide: Text.ElideRight
        textType: Qaterial.Style.TextType.Caption
        color: root.object && root.object.isBounded ? Qaterial.Style.green : Qaterial.Style.red
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
        text: "rxAddress : " + (root.object ? root.object.rxAddress : "")
        onClicked: if(root.object) dialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                if(acceptableInput)
                    root.object.rxAddress = result
                else
                    snackbarManager.show({text : result + " isn't an ipv4 rxAddress"})
            },
            text: root.object.rxAddress,
            title: qsTr("Enter Bind Ip Address"),
            textTitle: qsTr("Ip"),
            helperText: "Should be 0.0.0.0 for multicast input",
            validator: StringifyValidator.Ipv4,
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
        text: "rxPort : " + (root.object ? root.object.rxPort : "")
        onClicked: if(root.object) dialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.rxPort = result
            },
            text: root.object.rxPort,
            title: qsTr("Enter Bind Listening port"),
            textTitle: qsTr("Listening port"),
            helperText: "Between 0 and 65535",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            validator: StringifyValidator.SocketPort,
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
        text: "txPort : " + (root.object ? root.object.txPort : "")
        onClicked: if(root.object) dialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.txPort = result
            },
            text: root.object.txPort,
            title: qsTr("Enter Bind Listening port"),
            textTitle: qsTr("Listening port"),
            helperText: "Between 0 and 65535",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            validator: StringifyValidator.SocketPort,
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
                validator: StringifyValidator.Ipv4,
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
                validator: StringifyValidator.Ipv4,
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
    Row
    {
        width: parent.width
        Qaterial.SwitchButton
        {
            text: "multicastLoopback"
            implicitHeight: 32
            checked: root.object && root.object.multicastLoopback
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
            onCheckedChanged:
            {
                if(root.object && root.object.multicastLoopback !== checked)
                    root.object.multicastLoopback = checked
            }
        }

        Qaterial.SwitchButton
        {
            text: "inputEnabled"
            implicitHeight: 32
            checked: root.object && root.object.inputEnabled
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
            onCheckedChanged:
            {
                if(root.object && root.object.inputEnabled !== checked)
                    root.object.inputEnabled = checked
            }
        }
    }

    Row
    {
        spacing: 10
        Column
        {
            Qaterial.Label
            {
                text: "rxBytesPerSeconds : " + (root.object ? root.object.rxBytesPerSeconds : "")
                textType: Qaterial.Style.TextType.Caption
            }

            Qaterial.Label
            {
                text: "txBytesPerSeconds : " + (root.object ? root.object.txBytesPerSeconds : "")
                textType: Qaterial.Style.TextType.Caption
            }

            Qaterial.Label
            {
                text: "rxBytesTotal : " + (root.object ? root.object.rxBytesTotal : "")
                textType: Qaterial.Style.TextType.Caption
            }

            Qaterial.Label
            {
                text: "txBytesTotal : " + (root.object ? root.object.txBytesTotal : "")
                textType: Qaterial.Style.TextType.Caption
            }
        }

        Column
        {
            Qaterial.Label
            {
                text: "rxPacketsPerSeconds : " + (root.object ? root.object.rxPacketsPerSeconds : "")
                textType: Qaterial.Style.TextType.Caption
            }

            Qaterial.Label
            {
                text: "txPacketsPerSeconds : " + (root.object ? root.object.txPacketsPerSeconds : "")
                textType: Qaterial.Style.TextType.Caption
            }

            Qaterial.Label
            {
                text: "rxPacketsTotal : " + (root.object ? root.object.rxPacketsTotal : "")
                textType: Qaterial.Style.TextType.Caption
            }

            Qaterial.Label
            {
                text: "txPacketsTotal : " + (root.object ? root.object.txPacketsTotal : "")
                textType: Qaterial.Style.TextType.Caption
            }
        }
    }

    Row
    {
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
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            textType: Qaterial.Style.TextType.Caption
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