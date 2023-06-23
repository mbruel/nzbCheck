# nzbCheck
command line tool to check nzb files:<br/>
- First inside the nzb to see if each files has the expected number of Articles (following yEnc format for the subject of the files)
- then we check their availability on Usenet using the [NNTP Stat command](https://tools.ietf.org/html/rfc3977#section-6.2.4). 
<br/>
Implemented in C++11/Qt5, nzbCheck is released for Linux, Windows, MacOS and RPI.<br/>
<br/>
/!\ It is only checking if the Articles are available, there is no CRC check => some of them might be corrupted /!\

### Usage :
<pre>
Syntax: nzbcheck (options)* -i &lt;nzb file&gt;
	--help             : Help: display syntax
	-v or --version    : app version
	--progress         : display progress bar
	-d or --debug      : display debug information
	-q or --quiet      : quiet mode (no output on stdout)
	-i or --input      : input file : nzb file to check

// you can provide servers in one string using -S and/or split the parameters for ONE SINGLE server
	-S or --server     : NNTP server following the format (&lt;user&gt;:&lt;pass&gt;@@@)?&lt;host&gt;:&lt;port&gt;:&lt;nbCons&gt;:(no)?ssl
	-h or --host       : NNTP server hostname (or IP)
	-P or --port       : NNTP server port
	-s or --ssl        : use SSL
	-u or --user       : NNTP server username
	-p or --pass       : NNTP server password
	-n or --connection : number of NNTP connections

Examples:
  - nzbcheck --progress -S "user:password@@@news.usenetserver.com:563:50:ssl" -i /nzb/myNzbFile.nzb
  - nzbcheck --quiet -h news.usenetserver.com -P 563 -u user -p password -n 50 -s -i /nzb/myNzbFile.nzb
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

### Donations
I'm Freelance nowadays, working on several personal projects, so if you use the app and would like to contribute to the effort, feel free to donate what you can.<br/>
<br/>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=W2C236U6JNTUA&item_name=nzbCheck&currency_code=EUR"><img align="left" src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif" alt="nzbCheck"></a>
 or in Bitcoin at this address: **3BGbnvnnBCCqrGuq1ytRqUMciAyMXjXAv6**
<img align="right" align="bottom" width="120" height="120" src="https://raw.githubusercontent.com/mbruel/ngPost/master/pics/btc_qr.gif" alt="ngPost_QR">
