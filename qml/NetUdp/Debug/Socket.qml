/**
 * Copyright (C) Olivier Le Doeuff 2019
 * Contact: olivier.ldff@gmail.com
 */

// Qt
import QtQuick 2.12

// Backend
import Qaterial 1.0 as Qaterial
import NetUdp 1.0 as NetUdp
import NetUdp.Debug 1.0 as Debug

Qaterial.DebugObject
{
  id: root
  property NetUdp.Socket object: null
  title: "" + (root.object ? root.object : "Server null")

  content: Debug.SocketContent
  {
    object: root.object
  }
}
