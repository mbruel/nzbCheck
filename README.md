# nzbCheck
command line tool to check nzb files by fetching the headers of each Article using the [Nntp Stat command](https://tools.ietf.org/html/rfc3977#section-6.2.4)

/!\ It is only checking if the Articles are available, there is no CRC check => some of them might be corrupted /!\

### Usage :
<pre>
Syntax: nzbcheck (options)* -i &lt;nzb file&gt;
	--help             : Help: display syntax
	-v or --version    : app version
	--progress         : display progress bar
	-d or --debug      : display debug information
	-q or --quit       : quiet mode (no output on stdout)
	-i or --input      : input file : nzb file to check
	-S or --server     : NNTP server following the format (<user>:<pass>@@@)?<host>:<port>:<nbCons>:(no)?ssl
	-h or --host       : NNTP server hostname (or IP)
	-P or --port       : NNTP server port
	-s or --ssl        : use SSL
	-u or --user       : NNTP server username
	-p or --pass       : NNTP server password
	-n or --connection : number of NNTP connections
</pre>

### Output:
number of missing Articles (or negative value if any syntax error or parsing issue)

### How to build
#### Dependencies:

    build-essential (C++ compiler, libstdc++, make,...)
    qt5-default (Qt5 libraries and headers)
    qt5-qmake (to generate the moc files and create the Makefile)
    libssl (v1.0.2 or v1.1) but it should be already installed on your system

#### Build:

    go to the src folder
    qmake
    make

Easy! it should have generate the executable **nzbcheck**<br/>
you can copy it somewhere in your PATH so it will be accessible from anywhere<br/>

As it is made in C++/QT, you can build it and run it on any OS (Linux / Windows / MacOS / Android) <br/>
releases have only been made for Linux x64 and Windows x64 (for 7 and above) and MacOS<br/>
in order to build on other OS, the easiest way would be to [install QT](https://www.qt.io/download) and load the project in QtCreator<br/>
