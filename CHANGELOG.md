# Changelog

<a name="2.0.6"></a>
## 2.0.6 (2024-08-13)

### Fixed

- 🐛 Correctly call &#x60;bind&#x60; only when necessary ([#16](https://github.com/OlivierLDff/NetUdp/issues/16)) [[7c50285](https://github.com/OlivierLDff/NetUdp/commit/7c5028504b2b604d70d04e2fe810e2983ba28ea9)]
- 🐛 Don&#x27;t link to &#x60;Qt::Qml&#x60; when &#x60;NETUDP_ENABLE_QML&#x60; is &#x60;OFF&#x60; ([#13](https://github.com/OlivierLDff/NetUdp/issues/13)) [[1fd6669](https://github.com/OlivierLDff/NetUdp/commit/1fd6669a2c953b9550e5cf8a2bde6e79914c5ca1)]
- 💚 Fix CI to support Qt5 and update Qt6 ([#14](https://github.com/OlivierLDff/NetUdp/issues/14)) [[fced1e8](https://github.com/OlivierLDff/NetUdp/commit/fced1e80aba12bfeaf0489f2fa241a9af8c41388)]

### Miscellaneous

- 📝 CI badges [[35db493](https://github.com/OlivierLDff/NetUdp/commit/35db493597eefdb1c573accf5293678ec4c5c007)]
-  👷 Rename CI to qt5 &amp; qt6 [[1d4bf14](https://github.com/OlivierLDff/NetUdp/commit/1d4bf1495892d6e49db907fed8b9f826745e0011)]
- 📝 Maintenance memo [[40ff866](https://github.com/OlivierLDff/NetUdp/commit/40ff866071730fc31d4fe3e8e3de4fc63156e693)]
- 📝 Update Changelog ([#11](https://github.com/OlivierLDff/NetUdp/issues/11)) [[30cfe50](https://github.com/OlivierLDff/NetUdp/commit/30cfe506ed8357277a5b218f1cc7b3fd46cf09d2)]


<a name="2.0.5"></a>
## 2.0.5 (2022-10-03)

### Fixed

- 🐛 QT_VERSION_MAJOR not defined [[b12d711](https://github.com/OlivierLDff/NetUdp/commit/b12d7112cf3f6bbd89279335e6dbaf6f085988be)]

### Miscellaneous

- 📝 Update Changelog ([#10](https://github.com/OlivierLDff/NetUdp/issues/10)) [[bd72d22](https://github.com/OlivierLDff/NetUdp/commit/bd72d222cabdb7de0bf58abf7b3ccc3282147084)]


<a name="2.0.4"></a>
## 2.0.4 (2022-10-03)

### Miscellaneous

-  👷 gitmoji changelog [[b8bc549](https://github.com/OlivierLDff/NetUdp/commit/b8bc549520d5c82a8fb4a48eb51b9de1b540e186)]
-  Qt6 ([#9](https://github.com/OlivierLDff/NetUdp/issues/9)) [[98b0c53](https://github.com/OlivierLDff/NetUdp/commit/98b0c537f8a04c664846a5eea553b9c212776c83)]


<a name="2.0.3"></a>
## 2.0.3 (2022-08-02)

### Changed

- 📌 v2.0.3 [[0bd8053](https://github.com/OlivierLDff/NetUdp/commit/0bd80530750200e5194e9cf9d4366f569be4a3e5)]

### Removed

- 🔇 Remove non important logs when input is disabled [[02644ca](https://github.com/OlivierLDff/NetUdp/commit/02644ca511a3f21be06ab9aa83767b6a1150b5ce)]


<a name="2.0.2"></a>
## 2.0.2 (2021-08-29)

### Changed

- 📌 v2.0.2 [[19911ef](https://github.com/OlivierLDff/NetUdp/commit/19911efee28e1eab52ad5947fd20298648081c70)]
- ⬆️ CPM v0.32.3 [[c455ebf](https://github.com/OlivierLDff/NetUdp/commit/c455ebfc4fe028a31829c7f2f4559eac0629b2f0)]


<a name="2.0.1"></a>
## 2.0.1 (2021-08-29)

### Changed

- 📌 v2.0.1 [[0d77664](https://github.com/OlivierLDff/NetUdp/commit/0d7766484a496dd34db478ab1b2ada3d3d3d0e57)]
- 🚚 FetchRecyler -&gt; FetchRecycler [[3a1ff85](https://github.com/OlivierLDff/NetUdp/commit/3a1ff851d2f00e1d94fed772bca8ba9dec78b0de)]

### Miscellaneous

- 📝 Remove note about spdlog [[1dc3947](https://github.com/OlivierLDff/NetUdp/commit/1dc394789693aa617530b9b44907675a773a8762)]
- 🔨 NetUdp::NetUdp library alias [[a48aa1d](https://github.com/OlivierLDff/NetUdp/commit/a48aa1de29afd24e3eef397087f67e8f0516d737)]


<a name="2.0.0"></a>
## 2.0.0 (2021-08-26)

### Added

- ➕ Manage dependencies via CPM [[3ca280a](https://github.com/OlivierLDff/NetUdp/commit/3ca280ad428c1adac76d8d80ec2d0a4c4d09e969)]
- 🔊 Print build command at the of cmake [[29b932a](https://github.com/OlivierLDff/NetUdp/commit/29b932a1edcb1e759bf7e67459bd984b31638062)]

### Changed

- 📌 v2.0.0 [[ada0e6d](https://github.com/OlivierLDff/NetUdp/commit/ada0e6d48b29a077f3b7ce77f113c295cdd421fc)]
- ⚡ include moc_Worker for faster compilation time [[e2ccfcc](https://github.com/OlivierLDff/NetUdp/commit/e2ccfcc438c93b6f00789a349af6f5eb4b7e36ba)]
- ♻️ IInterface forward declare [[cb04326](https://github.com/OlivierLDff/NetUdp/commit/cb043261679c0f01e4f8d61ccc5cb6f1b5f851ba)]
- ♻️ interface -&gt; iface to avoid conflict with MSVC # define interface struct https://stackoverflow.com/questions/25234203/what-is-the-interface-keyword-in-msvc [[fc426ac](https://github.com/OlivierLDff/NetUdp/commit/fc426ac716766d2d6efb62783100db40820e5c77)]
- ♻️ pimpl WorkerPrivate [[32996c7](https://github.com/OlivierLDff/NetUdp/commit/32996c77ef923e64582280cae5d7f9b7d4d778e3)]
- ♻️ pimpl SocketPrivate [[f0a601e](https://github.com/OlivierLDff/NetUdp/commit/f0a601e7013675ad8fd719dd8d80a01ca6973efb)]
- ♻️ pimpl RecycledDatagramPrivate recycler::Buffer is now private dependency [[3e2199d](https://github.com/OlivierLDff/NetUdp/commit/3e2199d608c4cdc5960d4b415970ef8a0ecfa4cb)]
- ⚡ NETUDP_ENABLE_UNITY_BUILD [[55b0ecf](https://github.com/OlivierLDff/NetUdp/commit/55b0ecfc8273604007045c7bb652afadbc3bec06)]
- ♻️ cmake-format compliant target_link_libraries [[d585f5a](https://github.com/OlivierLDff/NetUdp/commit/d585f5ab30ad98bd593cb62f51cc04e7d6b32d43)]
- 🔧 clang-format script [[21a3c34](https://github.com/OlivierLDff/NetUdp/commit/21a3c34127d693a8cb6b5f0d6204604d00af7ca6)]
- 🎨 cmake-format [[5484702](https://github.com/OlivierLDff/NetUdp/commit/5484702aa8984a29f8d93bbcda2e1303b1958392)]
- 🎨 Run js-beautify [[66d3f4c](https://github.com/OlivierLDff/NetUdp/commit/66d3f4c0d76c5eb5c0cd5a0bbb4e8af294f2646c)]

### Removed

- ➖ spdlog [[6357da4](https://github.com/OlivierLDff/NetUdp/commit/6357da41fc14a15e79cc58e842eb2ffe39cfbab7)]

### Fixed

- 🐛 missing static qualifier [[6df0063](https://github.com/OlivierLDff/NetUdp/commit/6df0063f79ae33b654cefc5bd6d3bb847dd6ab02)]
- 🐛 Use steady_clock instead of system to avoid rollback [[c1d1d81](https://github.com/OlivierLDff/NetUdp/commit/c1d1d814f13aaf39f7334c252bf080c1eb2ead83)]

### Miscellaneous

- 📝 remove test from dependency test, as user won&#x27;t enable test, so he doesn&#x27;t care about extra test dependencies. [[27c5932](https://github.com/OlivierLDff/NetUdp/commit/27c5932a8c7af4a93cbcd5c86675d43b5bf8ed9e)]
- 📝 update Readme example to match new namespace &amp; include path [[f3af610](https://github.com/OlivierLDff/NetUdp/commit/f3af610185db8a0c1e930343b1e033b4fb432417)]
- 📝 v2.0.0 changelog [[8f4d29d](https://github.com/OlivierLDff/NetUdp/commit/8f4d29d93c5074469acfc7e35224455b0563d7c3)]
- 📝 gitmoji badge [[925a34c](https://github.com/OlivierLDff/NetUdp/commit/925a34c2ccbb6e2bbb7d47673c42f0b4e4e5655d)]
- 📝 update dependencies graph with spdlog being removed [[7c58377](https://github.com/OlivierLDff/NetUdp/commit/7c58377a5bb36c2981332ac3ba67c1c9ec9da07a)]
- 🔨 Make recycler private since all Recycler include were moved inside pimpl [[2e1bf58](https://github.com/OlivierLDff/NetUdp/commit/2e1bf5828e5de5a6b866dbc8aec5c77d9decbe6c)]
- 💥 net::udp -&gt; netudp [[c68ee1f](https://github.com/OlivierLDff/NetUdp/commit/c68ee1ff659cd1472564a699d8579f0b22f36ace)]
- 🔨 General cmake improvement: - include guard - default CMAKE_BUILD_TYPE - NETUDP_VERBOSE [[4a1cf1f](https://github.com/OlivierLDff/NetUdp/commit/4a1cf1f849cf83149e0e68356b1e349bea2b850d)]
-  👷 Update CI to Qt5.15.2 [[6ff3059](https://github.com/OlivierLDff/NetUdp/commit/6ff305959179f57265f648562de0d470b5134bba)]
- 🔨 First step toward Qt6 support [[ea10456](https://github.com/OlivierLDff/NetUdp/commit/ea1045614f63103486e3d80987f9e3ac1aa698d1)]
- 💥 Net/Udp -&gt; NetUdp This will break every include. I&#x27;m doing that because Net/Udp was annoying, confusing, and unclear [[cc7a6c6](https://github.com/OlivierLDff/NetUdp/commit/cc7a6c64d54718fe1bb184aa6449edbde6646a5c)]
- 🙈 Use git-lfs [[f25e110](https://github.com/OlivierLDff/NetUdp/commit/f25e11066e067ec6de6a2fa37088cab02ffc8db4)]
- 📝 Update Readme with dependencies [[ab3704f](https://github.com/OlivierLDff/NetUdp/commit/ab3704fa1180d8c833f837877c0bc42b290fd32c)]
- 🙈 gitattributes [[878d8ad](https://github.com/OlivierLDff/NetUdp/commit/878d8ad1f39996262c65b4eb0b78404e8f638096)]
- 🔨 Only set USE_FOLDER if main project [[a0d3e03](https://github.com/OlivierLDff/NetUdp/commit/a0d3e0313d677e82f6a5dff3ff8049456e11710c)]
- 🔨 NETUDP_MAIN_PROJECT [[54cd9c3](https://github.com/OlivierLDff/NetUdp/commit/54cd9c39cb5f109318f05be8ae1254ab9b0d28b9)]
- 📄 Add MIT license in every files [[b684df8](https://github.com/OlivierLDff/NetUdp/commit/b684df83a575e85c06064160b981d2c4f5499353)]


<a name="1.3.1"></a>
## 1.3.1 (2021-05-18)

### Changed

- 📌 Update to v1.3.1 [[f747731](https://github.com/OlivierLDff/NetUdp/commit/f747731efa1bffdbc71bd29109137a74327a7a62)]
- 🎨 Run clang-format [[9203254](https://github.com/OlivierLDff/NetUdp/commit/9203254adde24eb5e4a83ae99752c4376efe3f46)]

### Fixed

- 🐛 include missing QElapsedTimer [[1b4978c](https://github.com/OlivierLDff/NetUdp/commit/1b4978c113552eac297ca8c888b7645851aa85db)]

### Miscellaneous

-  👷 format [[f9cc511](https://github.com/OlivierLDff/NetUdp/commit/f9cc511ce24b3ee68ad9f6f38c15b2a72f4b4ff5)]


<a name="1.3.0"></a>
## 1.3.0 (2021-05-18)

### Added

- ✅ Check releaseBindedPort [[a133a59](https://github.com/OlivierLDff/NetUdp/commit/a133a59c0ea47540a27e260e13fcf942ccdb6710)]

### Changed

- 📌 Update to v1.3.0 [[0a50ad5](https://github.com/OlivierLDff/NetUdp/commit/0a50ad5fe8482ca8535e5f53b89bbadd409f3320)]
- ♻️ Worker: allocate QElapsedTimer statically [[ed0b020](https://github.com/OlivierLDff/NetUdp/commit/ed0b020f018f0f8a1e31d32b664fdcf588bf55fd)]
- 🚨 Add missing override specifier [[7621bb2](https://github.com/OlivierLDff/NetUdp/commit/7621bb29c6bf0bad2f2c78e4a30f59e82b3d9ed9)]
- ♻️ Move disableSocketTimeout to a variable [[e60fff4](https://github.com/OlivierLDff/NetUdp/commit/e60fff4f6de6cd3653930eb367e3ba13e7c48918)]
- 🚨 Iterate with reference to avoid copy [[125f8f3](https://github.com/OlivierLDff/NetUdp/commit/125f8f37dcfe41a97264afd45ca3634d079e8046)]

### Fixed

- 💚 Add missing libxcb-randr0 to run qt quick test in CI [[a9e2cff](https://github.com/OlivierLDff/NetUdp/commit/a9e2cffb3db0023e62a66b16188a4d5e36b97975)]

### Miscellaneous

- 💥 Use raw pointer for worker &amp; worker thread. Parent Worker to Socket [[79d8c15](https://github.com/OlivierLDff/NetUdp/commit/79d8c152416ed6441079879d0f48ad0ffacc9efa)]
- 📝 Add v1.2.2 changelog [[9be7d82](https://github.com/OlivierLDff/NetUdp/commit/9be7d829872b1277b0b125787f3b2acf4a7a4b7b)]


<a name="1.2.2"></a>
## 1.2.2 (2021-05-04)

### Added

- 🔊 Log more info about fail send datagram [[601174e](https://github.com/OlivierLDff/NetUdp/commit/601174e901a1f5c38108bb63a65de0d498f379f1)]

### Changed

- 📌 Update to v1.2.2 [[3a18a29](https://github.com/OlivierLDff/NetUdp/commit/3a18a29387e405717af850ffe02e9bc166041eb1)]

### Fixed

- 🐛 Fix reset Datagram. To reset to Datagram default state. [[501ff6f](https://github.com/OlivierLDff/NetUdp/commit/501ff6f2e78166856c7b486427ec2efe2216ee3b)]


<a name="1.2.1"></a>
## 1.2.1 (2021-02-15)

### Added

- 🔊 Log missing cmake info : NETUDP_ENABLE_PCH, NETUDP_ENABLE_EXAMPLES, NETUDP_ENABLE_TESTS [[3e5be25](https://github.com/OlivierLDff/NetUdp/commit/3e5be25dadde8e1bda92e85d658fbc53d7647634)]

### Changed

- 📌 Update to v1.2.1 [[191243c](https://github.com/OlivierLDff/NetUdp/commit/191243c4b3705a966dcb8109ce3bc7be5d841492)]

### Fixed

- 🐛 Fix compilation with disabled precompiled header + explicit qt library include header (ie &lt;QtCore/QObject&gt; instead of &lt;QObject&gt; [[e1d21b7](https://github.com/OlivierLDff/NetUdp/commit/e1d21b7c46a1c08fa5a8658c21ad6456b2f2d9a7)]

### Miscellaneous

- 📝 Ci Status Badge in Readme [[2ccf276](https://github.com/OlivierLDff/NetUdp/commit/2ccf276cafe9c266abb20256b9f234d46bb3765c)]


<a name="1.2.0"></a>
## 1.2.0 (2021-02-03)

### Fixed

- 🐛 Fix compilation with -DNETUDP_ENABLE_QML&#x3D;OFF [[00ebb28](https://github.com/OlivierLDff/NetUdp/commit/00ebb28521b4879b19fc5f62cfff0c14babad3ce)]
- 🐛 Fix qrc:/NetUdp/Debug/SocketContent.qml:57: ReferenceError: isBounded is not defined [[06146af](https://github.com/OlivierLDff/NetUdp/commit/06146af65a4842055d370b8351e306f6b7e31de0)]
- 🐛 Use new Qaterial v1.4 textTheme [[373124b](https://github.com/OlivierLDff/NetUdp/commit/373124b738f3b0e5cd9b1f7bf97e17042d29c535)]
- 🐛 Fix potential nullptr access [[fdf70ca](https://github.com/OlivierLDff/NetUdp/commit/fdf70ca1d9ee4c9696f3fa4718791506d6a900f5)]

### Miscellaneous

- 📝 Update Readme with v1.2 release notes [[dfbb183](https://github.com/OlivierLDff/NetUdp/commit/dfbb183be690e4d2607e689ccff7c3035c6108e2)]
-  Update CMakeLists.txt [[d24f2b5](https://github.com/OlivierLDff/NetUdp/commit/d24f2b53615848f5207057b23d3c6f1cff83601d)]
-  Update Readme.md [[aee6419](https://github.com/OlivierLDff/NetUdp/commit/aee641955f1c1117bb1a08dd8654e8f18019bfc0)]
-  resize Datagram + resize RecycledDatagram [[6878a6a](https://github.com/OlivierLDff/NetUdp/commit/6878a6a228fed0bbf2abc977e22c1a5a3b85a8ed)]
-  Fix watchdog memory leak + force isBounded to false when onStop is called. [[31c472a](https://github.com/OlivierLDff/NetUdp/commit/31c472ad75a51493ef138799b2d6394f69bb47c4)]
-  Qaterial.Data to Qaterial.DataFormat [[19a0a31](https://github.com/OlivierLDff/NetUdp/commit/19a0a312e9a84b644613a82af5263c521c932811)]
-  Improve logs [[ebbd056](https://github.com/OlivierLDff/NetUdp/commit/ebbd05604020faf80b96ade506576a98a52fc684)]
-  Important fix that could cause crash! Erase iterator only if valid [[615b3ee](https://github.com/OlivierLDff/NetUdp/commit/615b3ee019f16b579642f8f9026912e0d014f741)]
-  Private class in namespace to avoid side effects [[3dd08ca](https://github.com/OlivierLDff/NetUdp/commit/3dd08ca5438256b4136193fba732ecb46e7b8677)]
-  Assert interface is valid [[29a3bab](https://github.com/OlivierLDff/NetUdp/commit/29a3babe0cc3bd54da10649430c0e34825642df9)]
-  Forward declare QUdpSocket and QElapsedTimer [[9fe1def](https://github.com/OlivierLDff/NetUdp/commit/9fe1defa7772b7a52813896322734538a0001219)]
-  Update Readme.md [[2c871cc](https://github.com/OlivierLDff/NetUdp/commit/2c871cc2d0c4c7f83f33c29f4336923d6ad3cc80)]
-  Create Licence [[a567fbf](https://github.com/OlivierLDff/NetUdp/commit/a567fbff7e05a3277960e5929a6bb2fd201a2b66)]


<a name="1.1.0"></a>
## 1.1.0 (2020-08-16)

### Miscellaneous

-  Merge pull request [#5](https://github.com/OlivierLDff/NetUdp/issues/5) from OlivierLDff/dev [[af213c5](https://github.com/OlivierLDff/NetUdp/commit/af213c546f6454870cb969fb7446dce4feb854ee)]
-  Update Readme &amp; Debug [[baa6e7f](https://github.com/OlivierLDff/NetUdp/commit/baa6e7fb4d39b865d2fe0c7c828ea9cd35e5b209)]
-  v1.1 basic [[621808c](https://github.com/OlivierLDff/NetUdp/commit/621808c309d1777b87a54329140e23d4a94cee5d)]
-  registerTypes to registerQmlTypes, loadResources to loadQmlResources [[58afadf](https://github.com/OlivierLDff/NetUdp/commit/58afadff1a3cad351d55ade737c1eeaabcb59b97)]
-  Fix stl include [[e263c2b](https://github.com/OlivierLDff/NetUdp/commit/e263c2bd3396ef2985b3ca24298af1703fab159d)]
-  ci [[7d0dd71](https://github.com/OlivierLDff/NetUdp/commit/7d0dd710c6dd4154459b9fabe22b2d1950b6f282)]
-  Major fix for incoming multicast packet: Introduce multicastListeningInterfaces &amp; multicastListenOnAllInterfaces (that is on by default). Those property allow a fine control of which interface should be listened for incoming multicast packet. By default now, all interfaces are listened, not only the default os one. [[74ea0a6](https://github.com/OlivierLDff/NetUdp/commit/74ea0a6d7f1368425d45671303e64ce37671995b)]
-  Fix logging category &quot;net.udp.server&quot; to &quot;net.udp.socket&quot; and improve start log for worker [[635dfe3](https://github.com/OlivierLDff/NetUdp/commit/635dfe323bf5630683173e3f0d606df0845e5998)]
-  Fix missing &lt;set&gt; header in Socket.hpp [[385dd11](https://github.com/OlivierLDff/NetUdp/commit/385dd118561b5161df7e5d79e811fbad70ff7038)]
-  Avoid connection to a nullptr rxSocket when 2 sockets is on, but input is off [[526f16a](https://github.com/OlivierLDff/NetUdp/commit/526f16a5c0a165361b808f5b440d2fda4bd54e02)]
-  Allow ttl of 0 and set it as default. The os is in charge of choosing the best ttl [[96d646e](https://github.com/OlivierLDff/NetUdp/commit/96d646e57d9146fbdeea9548f2230741d3f42ffe)]
-  Better destruction of worker socket when using a worker thread [[b97ae5e](https://github.com/OlivierLDff/NetUdp/commit/b97ae5e9664fcb60862602c941d0aeb41be340dc)]
-  QList&lt;QString&gt; to QStringList + fix multicastGroups getter and setter that turned into slot with refacotr [[a727fd3](https://github.com/OlivierLDff/NetUdp/commit/a727fd36be7fc098f2094a855b27d0f8acb160ae)]
-  Refactor Server to Socket + fix send udp datagram to non reachable host on 127.0.0.1 [[4f6ab70](https://github.com/OlivierLDff/NetUdp/commit/4f6ab709b4c5c6a65d8b18dc9cdf372c43cf433d)]
-  More example : EchoClient &amp; EchoServer [[ba85170](https://github.com/OlivierLDff/NetUdp/commit/ba85170ad213bd42ba4a797037e18fd12c9da366)]
-  Rename namespace to net::udp, restart socket with a queued connected, install loggers in example. [[ac25cd6](https://github.com/OlivierLDff/NetUdp/commit/ac25cd6822862d052f2ee3d68ded1c6f08c90557)]
-  Update debug qml to use new Qaterial singleton dialog manager and snackbar manager [[8c03b37](https://github.com/OlivierLDff/NetUdp/commit/8c03b37cc8df8c4cbf9cfce2d63c92ae006bede4)]
-  _separateRxTxSockets only if separateRxTxSockets() || txPort() [[93d82a0](https://github.com/OlivierLDff/NetUdp/commit/93d82a0b25f1362beebdfbfafeb61051ad63f8e0)]
-  Psn use PCH header and spdlog as logger backend [[4fcf965](https://github.com/OlivierLDff/NetUdp/commit/4fcf96562aef927474aff9c3e4fdee3af57ba880)]
-  Stringify Formatter to display bitrates [[7e1c31f](https://github.com/OlivierLDff/NetUdp/commit/7e1c31f356aa776bf1f04c8d257020cc118b194e)]
-  Fix leaveAllMulticastGroup that was referencing an iterator that got delete in function call. Fix it with QString copy. Also remove ServerImpl that is annoying to read [[fd25018](https://github.com/OlivierLDff/NetUdp/commit/fd250185d340708af7cf924453b63edff53e7f2b)]
-  Force Use separate socket if txport or if multicast loopback. Multicast loopback when listening on same port on linux cause read of empty datagram [[665dbb3](https://github.com/OlivierLDff/NetUdp/commit/665dbb36038a264b7b91d2bef2b8c80fcc15e616)]
-  Detect if(!datagram.isValid()) [[d1def46](https://github.com/OlivierLDff/NetUdp/commit/d1def46f08c7ff9028d1b0466d52df601cf5f211)]
-  Check datagram size is valid datagram.data().size() [[c1ecf10](https://github.com/OlivierLDff/NetUdp/commit/c1ecf107d3ed322c32f2f87b91bf0cf135cd516c)]
-  Update readme to add info about isPacketValid overridable function in ServerWorker children [[61a1250](https://github.com/OlivierLDff/NetUdp/commit/61a1250f0a841ac1c222553a99a8e39bc05b753f)]
-  Move qml folder to qml/NetUdp [[02d9f44](https://github.com/OlivierLDff/NetUdp/commit/02d9f44b8162ee20a8e72ce12eee8cd064e1f95e)]
-  MOve NetUdp.hpp to Net/Udp/NetUdp.hpp to match library standards [[7dd2294](https://github.com/OlivierLDff/NetUdp/commit/7dd2294b618441be4d17c197b12f624197d76731)]
-  Don&#x27;t send message if not bounded [[8075582](https://github.com/OlivierLDff/NetUdp/commit/80755821274d09c40f3ea43e85e681d510efb2a0)]
-  Fix qml [[f8375d2](https://github.com/OlivierLDff/NetUdp/commit/f8375d20326da4364768fcdca905fcc11c185e09)]
-  fix line endings [[4e248bb](https://github.com/OlivierLDff/NetUdp/commit/4e248bbdbaf1e25903f9522fc415f49251ae2593)]
-  Fix example + setIsBounded/Running are now setBounded/setRunning [[daa403a](https://github.com/OlivierLDff/NetUdp/commit/daa403a61274e43f5bf5ce303df6fd190c10cba1)]
-  Remove dependencies to QSuperMacros and Stringify [[567b737](https://github.com/OlivierLDff/NetUdp/commit/567b7374661fa0542c29769a558439f5ca1dca69)]
-  Doc + Example [[1afabf6](https://github.com/OlivierLDff/NetUdp/commit/1afabf61aabd0c3805c1f2b2f6be42d16441eaab)]
-  fix missing &lt;memory&gt; include [[2f5fd1b](https://github.com/OlivierLDff/NetUdp/commit/2f5fd1b27a29ada49167967fe29073f3801ff9ec)]
-  Creation of a worker thread can be changed at runtime [[b1b367b](https://github.com/OlivierLDff/NetUdp/commit/b1b367b4c5bf32a5931443fc8d0c8ab23cdd8218)]
-  Recycler for Datagram [[e377b15](https://github.com/OlivierLDff/NetUdp/commit/e377b1589b2aec7bf3c0ed9f61da91325ca6ca3c)]
-  Use new Net::Udp namespace [[9216954](https://github.com/OlivierLDff/NetUdp/commit/9216954c57bb28bf77c2cfe9a3486ceb7b51d64b)]
-  Update to cmake 3.14 as min version [[9793913](https://github.com/OlivierLDff/NetUdp/commit/9793913409281870a1a67d8777bcea86b788705e)]
-  leaveAllMulticastGroups function in AbstractServer [[902718d](https://github.com/OlivierLDff/NetUdp/commit/902718d5dfb123539bf8bfe86c24f4305cc1c03e)]
-  Update isBounded to false on stop server [[cd2f233](https://github.com/OlivierLDff/NetUdp/commit/cd2f233277bda425f63414ccac432792158ac91c)]
-  Remove call to qt process event [[f279155](https://github.com/OlivierLDff/NetUdp/commit/f279155965a1cb897d9d4456ad522a762d16e7c3)]
-  Merge remote-tracking branch &#x27;origin/master&#x27; [[ce28cda](https://github.com/OlivierLDff/NetUdp/commit/ce28cdabf93c6ad9e1b0a58f1dbf94244a7c288e)]
-  Fix header include order [[30a8563](https://github.com/OlivierLDff/NetUdp/commit/30a8563b6be0a521fa74242e5e220479d83e656e)]
-  fix log format [[9fa8a58](https://github.com/OlivierLDff/NetUdp/commit/9fa8a581f1de44335141269052f819413ec54dbb)]
-  QSM_REGISTER_TO_QML [[0e1bb37](https://github.com/OlivierLDff/NetUdp/commit/0e1bb3777fc2614e3a785c757f7cd97c5554d5bf)]
-  Include QThread that was missing for compilation (weird) [[1638e51](https://github.com/OlivierLDff/NetUdp/commit/1638e51186492b6ecdfabcbef5e8773b41d197f6)]
-  _separateRxTxSocketsChanged to _separateRxTxSockets [[34d33b2](https://github.com/OlivierLDff/NetUdp/commit/34d33b24779be47e296bcd9b50f4ca833db378c0)]
-  Less private include in header of worker [[001db91](https://github.com/OlivierLDff/NetUdp/commit/001db9142c040cf9d8004b0c922ca83e533cb2ab)]
-  ServerWorker can now instantiate 2 sockets, one for receiving and the other for receiving udp packet if required. ReadPendingDatagram loop also regularly call processEvent in the thread (~5ms) [[7c5fb3d](https://github.com/OlivierLDff/NetUdp/commit/7c5fb3d0782d5f1192e284ad9ed6fe7e4a433f5a)]
-  Add a txPort to choose the port when only outputEnabled is set [[563638b](https://github.com/OlivierLDff/NetUdp/commit/563638bbe83cac13c89df49dc1d707ac84844436)]
-  Move server.qml content into ServerContent for reuse into children debug object of Server [[db2cd33](https://github.com/OlivierLDff/NetUdp/commit/db2cd332389d8dd58e4a803270fb39f158adf72c)]
-  Packet Counter [[2dd5684](https://github.com/OlivierLDff/NetUdp/commit/2dd5684fb4131735565ebfb8e4183b372444022e)]
-  Remove useless leaveMulticastGroup that are handled when socket is destroyed [[a619e40](https://github.com/OlivierLDff/NetUdp/commit/a619e400dcfddecefaa2b7f4d54feaad9e34485c)]
-  Fix logs [[b75d070](https://github.com/OlivierLDff/NetUdp/commit/b75d07073a2762e97f13f88a25bfafd0dbb25886)]
-  Remove debug lines [[8564bc2](https://github.com/OlivierLDff/NetUdp/commit/8564bc26eeba50f4ab1c7694c29d25150b177e55)]
-  Everything was working with interface output for multicast packet. It was an error from tests [[a16a2d1](https://github.com/OlivierLDff/NetUdp/commit/a16a2d17479ca70bf4ae7ac62d465ce00883b890)]
-  Fix ttl option. Still bug in windows when changing iface [[8af82db](https://github.com/OlivierLDff/NetUdp/commit/8af82db525413f6a24759f60b4a5008941a02e76)]
-  makeDatagram &amp; setBufferSize in Datagram [[b5b1cf8](https://github.com/OlivierLDff/NetUdp/commit/b5b1cf834c2ce910a6ea179ede9fcf01a0b17c92)]
-  Check worker is running and bounded before sending datagram to it [[b31389b](https://github.com/OlivierLDff/NetUdp/commit/b31389bfcb506fa39b5d715fbed9869cbdc8cdb3)]
-  Stringify for version [[aa15d0e](https://github.com/OlivierLDff/NetUdp/commit/aa15d0eefa4d1006c078074b4707a1c5b1b1e3e2)]
-  Remove useless include [[b19d6ed](https://github.com/OlivierLDff/NetUdp/commit/b19d6ed6ec6dba9b94b580ea018fe6dea9636fdd)]
-  Avoid warning when object is null [[6185eb6](https://github.com/OlivierLDff/NetUdp/commit/6185eb66a7ffa4a290a6bbb4d0c31a33b65df36c)]
-  input enabled on udp server to disable receiving datagram [[9f672a4](https://github.com/OlivierLDff/NetUdp/commit/9f672a4a2763b178ed19df213cd12b15f3e4d67a)]
-  Use new Stringify.Validator module [[01b424c](https://github.com/OlivierLDff/NetUdp/commit/01b424c7e17166d95c08eea5cccce390be567a59)]
-  Merge remote-tracking branch &#x27;origin/master&#x27; [[5acf735](https://github.com/OlivierLDff/NetUdp/commit/5acf73562a9b42627e8ba40598cf2b5a7d0b62cd)]
-  add debug color [[f8feba3](https://github.com/OlivierLDff/NetUdp/commit/f8feba352f09e6b8b37c128742d39a29d345050b)]
-  Fix log warning [[463814b](https://github.com/OlivierLDff/NetUdp/commit/463814b914a76374ad97883fb496a7cfb1447257)]
-  update to C++14 [[1e80306](https://github.com/OlivierLDff/NetUdp/commit/1e80306e0862cf0c401e3664cc3ec98277381785)]
-  Remove useless cmake log line [[1d27da9](https://github.com/OlivierLDff/NetUdp/commit/1d27da96a4e2bedc6bdd785a3fea1b52e406fad2)]
-  Better Registration [[3d6c10a](https://github.com/OlivierLDff/NetUdp/commit/3d6c10ae686b58262a4d58e79c35bff29e203c9b)]
-  Fix bug when sending fail, we were adding -1 bytesWritten instead of 0 [[d20ebfb](https://github.com/OlivierLDff/NetUdp/commit/d20ebfb564627fe3f7160a927f1cd01a42f7b38c)]
-  fix message status spaces [[48ac257](https://github.com/OlivierLDff/NetUdp/commit/48ac257322fa69fa49df759c3ab6d6018b4c3a7f)]
-  Debug qml classes [[3af1b6e](https://github.com/OlivierLDff/NetUdp/commit/3af1b6e73b834ac0503810f319ebe1a4a25fd6f8)]
-  Q_SIGNAL + Q_EMIT [[a50b8cc](https://github.com/OlivierLDff/NetUdp/commit/a50b8cca4e794b3216e757b2c120843ed7b63231)]
-  Fix compilation [[1b61501](https://github.com/OlivierLDff/NetUdp/commit/1b61501d7ac5bbc795db17f3ae8230ec3121e1b4)]
-  Register + fix some stuff [[4c9ddaf](https://github.com/OlivierLDff/NetUdp/commit/4c9ddafdfd26238f076349330b501b2081b48132)]
-  fc [[37ff6ab](https://github.com/OlivierLDff/NetUdp/commit/37ff6abc116d4fa430d2d75a03cadb302107f5f0)]


