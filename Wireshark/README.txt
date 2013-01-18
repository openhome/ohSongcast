Songcast Wireshark Plugins
--------------------------

The plugins have been compiled and tested against Wireshark
version 1.8.4. They may or may not work with older versions.

The code needs to be built inside the Wireshark source tree.

* Download the Wireshark source code from:
     http://www.wireshark.org/download.html

* Unpack the source code archive.

* Copy the contents of the 'ohSongacst/Wireshark/plugins' folder
  into the 'plugins' folder of the Wireshark source tree.

* Build Wireshark as explained in the Developers Guide:
     http://www.wireshark.org/docs/wsdg_html_chunked/

* The plugin DLLs 'ohz.dll' and 'ohm.dll' can be added to an 
  existing Wireshark installation by copying them in its plugin
  folder, which will be something like this:
     C:\Program Files\Wireshark\plugins\1.8.4\ 
