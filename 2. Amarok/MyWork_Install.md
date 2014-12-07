Install Amarok
======================
Prepare
----------------------
<br>
First, you need to prepare you kubuntu system for compile the Amarok.  

- Base
    - You need these packages
        * [Cmake](http://www.cmake.org/)
        * [Git](http://git-scm.com/)

- You need these packages in cmake step:
    - You need these packages
        * [KDE-Full](https://www.kde.org/)
            * You need [KDM](http://en.wikipedia.org/wiki/KDE_Display_Manager) first
        * [TagLib](http://taglib.github.io/)
        * [KDELibs](https://code.launchpad.net/kdelibs)
        * [MySql C Library](http://dev.mysql.com/downloads/connector/c/)
        * [Google Mock](https://code.google.com/p/googlemock/)
        * [GObject](https://developer.gnome.org/gobject/stable/)
        * [PKGConfig](http://www.freedesktop.org/wiki/Software/pkg-config/)
        * [Qt Cryptographic Architecture](http://delta.affinix.com/qca/)
        * [FFmpeg](https://www.ffmpeg.org/)
        * [QJSON](http://qjson.sourceforge.net/)
        * [Media Transfer Protocol](https://wiki.archlinux.org/index.php/MTP)
        * [GDK Pixbuf](https://developer.gnome.org/gdk-pixbuf/unstable/gdk-pixbuf-The-GdkPixbuf-Structure.html)
        * [Curl Library](http://curl.haxx.se/)
        * [XmlLib](http://effbot.org/librarybook/xmllib.htm)
        * [OpenSSL Library](https://www.openssl.org/)
        * [Loudmouth](https://github.com/mhallendal/loudmouth)
        * [libavcodec (FFmpeg extra package)](https://www.ffmpeg.org/libavcodec.html)
        * [libavformat (FFmpeg extra package)](https://www.ffmpeg.org/libavformat.html)
        * [libavdevice (FFmpeg extra package)](https://www.ffmpeg.org/libavdevice.html)
        * [libavutil (FFmpeg extra package)](https://www.ffmpeg.org/libavutil.html)
        * [libswscale (FFmpeg extra package)](https://www.ffmpeg.org/libswscale.html)
        * [libpostproc (FFmpeg extra package)](https://www.ffmpeg.org/doxygen/0.6/postprocess_8h.html)


- You need these packages in make step:
    - You need these packages
        - [TCP wrappers Library](http://packages.ubuntu.com/precise/i386/libwrap0-dev)
        - [Asynchronous I/O Library](http://www.gnu.org/software/libc/manual/html_node/Asynchronous-I_002fO.html)


Install
--------------
<br>
You can paste these command to auto install requisite packages.

1\. <font color='red'>[Important]</font> Install [Cmake](http://www.cmake.org/) 
```
sudo apt-get install cmake
```

2\. <font color='green'>[Optional]</font> Install [Git](http://git-scm.com/)
```
sudo apt-get install git
```

3\. <font color='red'>[Important]</font> Install [KDE-Full](https://www.kde.org/) package and install [KDM](http://en.wikipedia.org/wiki/KDE_Display_Manager) first.  
```
sudo apt-get install kdm kde-full
```

4\. <font color='orange'>[Needful]</font> Install [TagLib](http://taglib.github.io/) and its extras library.
```
sudo apt-get install libtagc0-dev libtag-extras-dev
```

5\. <font color='red'>[Important]</font> Install [KDELibs](https://code.launchpad.net/kdelibs)
```
sudo apt-get install kdelibs5-dev
```

6\. <font color='orange'>[Needful]</font>Install [MySql C Library](http://dev.mysql.com/downloads/connector/c/)
```
sudo apt-get install libmysqld-dev
```

7\. <font color='orange'>[Needful]</font> Install [Google Mock](https://code.google.com/p/googlemock/)  
Well, the "README" file is worng. You must need this.
```
sudo apt-get install google-mock
```

8\. <font color='green'>[Optional]</font> Install [GObject](https://developer.gnome.org/gobject/stable/)
```
sudo apt-get install gobject-introspection
```

9\. <font color='green'>[Optional]</font> Install [PKGConfig](http://www.freedesktop.org/wiki/Software/pkg-config/)
```
sudo apt-get install pkgconf
```

10\. <font color='orange'>[Needful]</font> Install [Qt Cryptographic Architecture](http://delta.affinix.com/qca/) (QCA2)
```
sudo apt-get install libqca2-dev
```

11\. <font color='green'>[Optional]</font> Install [FFmpeg](https://www.ffmpeg.org/)
```
sudo apt-get install libffmpegthumbnailer-dev libffmpegthumbnailer4
```

12\. <font color='green'>[Optional]</font> Install [QJSON](http://qjson.sourceforge.net/)
```
sudo apt-get install libqjson-dev
```

13\. <font color='green'>[Optional]</font> Install [Media Transfer Protocol](https://wiki.archlinux.org/index.php/MTP)
```
sudo apt-get install libmtp-dev
```

14\. <font color='green'>[Optional]</font> Install [GDK Pixbuf](https://developer.gnome.org/gdk-pixbuf/unstable/gdk-pixbuf-The-GdkPixbuf-Structure.html)
```
sudo apt-get install libgdk-pixbuf2.0-dev
```

15\. <font color='orange'>[Needful]</font> Install [Curl Library](http://curl.haxx.se/)
```
sudo apt-get install libcurl4-openssl-dev
```

16\. <font color='green'>[Optional]</font> Intsall [XmlLib](http://effbot.org/librarybook/xmllib.htm)
```
sudo apt-get install libxml2-dev
```

17\. <font color='orange'>[Needful]</font> Install [OpenSSL Library](https://www.openssl.org/)
Well, the "README" wrong again. It didn't write this.
```
sudo apt-get install libssl-dev
```

18\. <font color='green'>[Optional]</font> Install [Loudmouth](https://github.com/mhallendal/loudmouth)
```
sudo apt-get install libloudmouth1-dev
```

19\. <font color='green'>[Optional]</font> Install [libavcodec](https://www.ffmpeg.org/libavcodec.html) & [libavformat ](https://www.ffmpeg.org/libavformat.html) & [libavdevice](https://www.ffmpeg.org/libavdevice.html) & [libavutil](https://www.ffmpeg.org/libavutil.html) & [libswscale](https://www.ffmpeg.org/libswscale.html) & [libpostproc](https://www.ffmpeg.org/doxygen/0.6/postprocess_8h.html)  
These are FFmpeg extra package.
```
sudo apt-get install libavcodec-dev
sudo apt-get install libavformat-dev 
sudo apt-get install libavdevice-dev
sudo apt-get install libavutil-dev
sudo apt-get install libswscale-dev
sudo apt-get install libpostproc-dev
```

20\. <font color='orange'>[Needful]</font>Install [TCP wrappers Library](http://packages.ubuntu.com/precise/i386/libwrap0-dev)
```
sudo apt-get install libwrap0-dev
```

21\. <font color='orange'>[Needful]</font>Install [Asynchronous I/O Library](http://www.gnu.org/software/libc/manual/html_node/Asynchronous-I_002fO.html)
```
sudo apt-get install libaio-dev
```


Compile Amarok
----------------------
<br>
1\. Clone(Download) the source code, You can also download the source code from other ways.  
Then, you can change these code, or keep it.
```
git clone git://anongit.kde.org/amarok.git
```

2\. Go to the src folder.  
If you download the source code from other way, you need go to your folder.
```
cd amarok
```

3\. Read the "README" file.  
In this case, this file include packages which you need to install and optional packages.
```
cat README | less
```

4\. Read the "INSTALL" file
```
cat INSTALL | less
```

5\. Create a new folder name build, this folder will use to put the build file in there.  
When you Config make and Making, it will create many files, you don't want to these files into your source code.
```
mkdir build
```

6\. Go to the build folder
```
cd build  
```

7\.Config the make and disable the test function.  
Cmake the ".." folder (I can't explain "..")  
"-DKDE4_BUILD_TESTS=OFF" mean withuout [Google Mock](https://code.google.com/p/googlemock/) test function (Gtest).  
```
cmake -DKDE4_BUILD_TESTS=OFF ..
```

8\. Make
```
make  
```

9\. Use make to install the binary
```
sudo make install
```

<br>
<br>
You can step by step, from top to bottom, and you will get a compiled binary.