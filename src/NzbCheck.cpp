//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
// This file is a part of ngPost : https://github.com/mbruel/nzbCheck
//
// ngPost is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 3.0 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301,
// USA.
//
//========================================================================

#include "NzbCheck.h"
#include "NntpCon.h"
#include "NntpServerParams.h"
#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QRegularExpression>

const QRegularExpression NzbCheck::sNntpArticleYencSubjectRegExp = QRegularExpression(sNntpArticleYencSubjectStrRegExp);

const QMap<NzbCheck::Opt, QString> NzbCheck::sOptionNames =
{
    {Opt::HELP,        "help"},
    {Opt::VERSION,     "version"},
    {Opt::PROGRESS,    "progress"},
    {Opt::DEBUG,       "debug"},
    {Opt::QUIET,       "quit"},
    {Opt::INPUT,       "input"},
    {Opt::SERVER,      "server"},
    {Opt::HOST,        "host"},
    {Opt::PORT,        "port"},
    {Opt::SSL,         "ssl"},
    {Opt::USER,        "user"},
    {Opt::PASS,        "pass"},
    {Opt::CONNECTION,  "connection"},
};

const QList<QCommandLineOption> NzbCheck::sCmdOptions = {
    { sOptionNames[Opt::HELP],                tr( "Help: display syntax")},
    {{"v", sOptionNames[Opt::VERSION]},       tr( "app version")},
    { sOptionNames[Opt::PROGRESS],            tr( "display progress bar")},
    {{"d", sOptionNames[Opt::DEBUG]},         tr( "display debug information")},
    {{"q", sOptionNames[Opt::QUIET]},         tr( "quiet mode (no output on stdout)")},
    {{"i", sOptionNames[Opt::INPUT]},         tr( "input file : nzb file to check"), sOptionNames[Opt::INPUT]},

    {{"S", sOptionNames[Opt::SERVER]},        tr("NNTP server following the format (<user>:<pass>@@@)?<host>:<port>:<nbCons>:(no)?ssl"), sOptionNames[Opt::SERVER]},
    {{"h", sOptionNames[Opt::HOST]},          tr("NNTP server hostname (or IP)"), sOptionNames[Opt::HOST]},
    {{"P", sOptionNames[Opt::PORT]},          tr("NNTP server port"), sOptionNames[Opt::PORT]},
    {{"s", sOptionNames[Opt::SSL]},           tr("use SSL")},
    {{"u", sOptionNames[Opt::USER]},          tr("NNTP server username"), sOptionNames[Opt::USER]},
    {{"p", sOptionNames[Opt::PASS]},          tr("NNTP server password"), sOptionNames[Opt::PASS]},
    {{"n", sOptionNames[Opt::CONNECTION]},    tr("number of NNTP connections"), sOptionNames[Opt::CONNECTION]}
};

void NzbCheck::onDisconnected(NntpCon *con)
{
    _connections.remove(con);
    if (_connections.isEmpty())
    {
        if (_dispProgressBar)
        {
            disconnect(&_progressbarTimer, &QTimer::timeout, this, &NzbCheck::onRefreshprogressbarBar);
            onRefreshprogressbarBar();
            _cout << "\n" << MB_FLUSH;
        }

        if (!_quietMode)
            _cout << tr("Nb Missing Article(s): %1/%2").arg(_nbMissingArticles).arg(_nbTotalArticles) << "\n" << MB_FLUSH;
        qApp->quit();
    }
}

void NzbCheck::onRefreshprogressbarBar()
{
    float progressbar = static_cast<float>(_nbCheckedArticles);
    progressbar /= _nbTotalArticles;

    _cout << "\r[";
    int pos = static_cast<int>(std::floor(progressbar * sprogressbarBarWidth));
    for (int i = 0; i < sprogressbarBarWidth; ++i) {
        if (i < pos) _cout << "=";
        else if (i == pos) _cout << ">";
        else _cout << " ";
    }
    _cout << "] " << int(progressbar * 100) << " %"
              << " (" << _nbCheckedArticles << " / " << _nbTotalArticles << ")"
              << tr(" missing: ") << _nbMissingArticles;
    _cout.flush();

    if (_nbCheckedArticles < _nbTotalArticles)
        _progressbarTimer.start(_refreshRate);
}

NzbCheck::NzbCheck():QObject(),
    _nzbPath(), _articles(),
    _cout(stdout), _cerr(stderr),
    _nbTotalArticles(0),  _nbMissingArticles(0), _nbCheckedArticles(0),
    _nntpServers(),
    _debug(0), _connections(),
    _dispProgressBar(false), _progressbarTimer(), _refreshRate(sDefaultRefreshRate),
    _quietMode(false)
{}

NzbCheck::~NzbCheck()
{
    if (_dispProgressBar)
        _progressbarTimer.stop();

    qDeleteAll(_nntpServers);
}

int NzbCheck::parseNzb()
{
    QFile file(_nzbPath);
    if (file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QXmlStreamReader xmlReader(&file);
        while ( !xmlReader.atEnd() )
        {
            QXmlStreamReader::TokenType type = xmlReader.readNext();
            if (type == QXmlStreamReader::TokenType::StartElement
                    && xmlReader.name() == "file")
            {
                QString subject = xmlReader.attributes().value("subject").toString();
                QRegularExpressionMatch match = sNntpArticleYencSubjectRegExp.match(subject);
                int nbArticles = 0, nbExpectedArticles = 0;
                if (match.hasMatch())
                    nbExpectedArticles = match.captured(1).toInt();
                while ( !xmlReader.atEnd() )
                {
                    QXmlStreamReader::TokenType type = xmlReader.readNext();
                    if (type == QXmlStreamReader::TokenType::EndElement
                            && xmlReader.name() == "file")
                    {
                        if (debugMode())
                            _cout << tr("The file '%1' has %2 articles in the nzb (expected: %3)").arg(
                                         subject).arg(nbArticles).arg(nbExpectedArticles) << "\n" << MB_FLUSH;
                        if (nbArticles < nbExpectedArticles)
                        {
                            if (!_quietMode)
                                _cout << tr("- %1 missing Article(s) in nzb for '%2'").arg(
                                         nbExpectedArticles - nbArticles).arg(subject) << "\n" << MB_FLUSH;

                            _nbMissingArticles += nbExpectedArticles - nbArticles;
                        }

                        break;
                    }
                    else if (type == QXmlStreamReader::TokenType::StartElement
                            && xmlReader.name() == "segment")
                    {
                        ++nbArticles;
                        xmlReader.readNext();
                        _articles.push(QString("<%1>").arg(xmlReader.text().toString()));
                    }
                }
            }
        }

        if (xmlReader.hasError()) {
            _cerr << "parsing error: " << xmlReader.errorString()
                      << " at line: " << xmlReader.lineNumber() << "\n" << MB_FLUSH;
            return -2;
        }
        _nbTotalArticles = _articles.size();
        if (!_quietMode)
            _cout << tr("%1 has %2 articles").arg(QFileInfo(_nzbPath).fileName()).arg(_nbTotalArticles) << "\n" << MB_FLUSH;
        return _nbTotalArticles;
    }
    else
    {
        _cerr << tr("Error opening nzb file...") << "\n" << MB_FLUSH;
        return -1;
    }

}

void NzbCheck::checkPost()
{
    int nbCons = 0;
    for (NntpServerParams *srvParam : _nntpServers)
        nbCons += srvParam->nbCons;

    nbCons = std::min(_nbTotalArticles, nbCons);

    int nb = 0;
    for (NntpServerParams *srvParam : _nntpServers)
    {
        for (int i = 1 ; i <= srvParam->nbCons; ++i)
        {
            NntpCon *con = new NntpCon(this, i, *srvParam);
            connect(con, &NntpCon::disconnected, this, &NzbCheck::onDisconnected, Qt::DirectConnection);
            emit con->startConnection();

            _connections.insert(con);

            if (++nb == nbCons)
                break;
        }
        if (nb == nbCons)
            break;
    }

//    if (debugMode())
        _cout << tr("Using %1 Connections").arg(nbCons) << "\n" << MB_FLUSH;

    if (_dispProgressBar)
    {
        connect(&_progressbarTimer, &QTimer::timeout, this, &NzbCheck::onRefreshprogressbarBar, Qt::DirectConnection);
        _progressbarTimer.start(_refreshRate);
    }
}

bool NzbCheck::parseCommandLine(int argc, char *argv[])
{
    QString appVersion = QString("%1_v%2").arg(sAppName).arg(sVersion);
    QCommandLineParser parser;
    parser.setApplicationDescription(appVersion);
    parser.addOptions(sCmdOptions);


    // Process the actual command line arguments given by the user
    QStringList args;
    for (int i = 0; i < argc; ++i)
        args << argv[i];

    if (!parser.parse(args))
    {
        _cerr << tr("Error syntax: %1\nTo list the available options use --help\n").arg(parser.errorText()).arg(argv[0]) << "\n" << MB_FLUSH;
        return false;
    }


    if (parser.isSet(sOptionNames[Opt::HELP]))
    {
        _showVersionASCII();
        _syntax(argv[0]);
        return false;
    }

    if (parser.isSet(sOptionNames[Opt::VERSION]))
    {
        _showVersionASCII();
        return false;
    }

    if (!parser.isSet(sOptionNames[Opt::INPUT]))
    {
        _cerr << tr("Error syntax: you should provide at least one input file or directory using the option -i");
        return false;
    }
    else
    {
        _nzbPath = parser.value(sOptionNames[Opt::INPUT]);
        QFileInfo fi(_nzbPath);
        if (!fi.exists() || !fi.isFile() || !fi.isReadable())
        {
            _cerr << tr("Error: please provide a readable nzb file...") << "\n" << MB_FLUSH;
            return false;
        }
    }

    if (parser.isSet(sOptionNames[Opt::PROGRESS]))
        _dispProgressBar = true;

    if (parser.isSet(sOptionNames[Opt::QUIET]))
    {
        _quietMode       = true;
        _dispProgressBar = false;
    }

    if (parser.isSet(sOptionNames[Opt::DEBUG]))
        _debug = 1;


    if (parser.isSet(sOptionNames[Opt::SERVER]))
    {
        _nntpServers.clear();
        QRegularExpression regExp(sNntpServerStrRegExp,  QRegularExpression::CaseInsensitiveOption);
        for (const QString &serverParam : parser.values(sOptionNames[Opt::SERVER]))
        {
            QRegularExpressionMatch match = regExp.match(serverParam);
            if (match.hasMatch())
            {
                bool    auth  = !match.captured(1).isEmpty();
                QString user  = match.captured(2);
                QString pass  = match.captured(3);
                QString host  = match.captured(4);
                ushort  port  = match.captured(5).toUShort();
                int     nbCon = match.captured(6).toInt();
                bool    ssl   = match.captured(7).isEmpty();
#ifdef __DEBUG__
                qDebug() << "NNTP Server: " << user << ":" << pass
                         << "@" << host << ":" << port << ":" << nbCon << ":" << ssl;
#endif
                NntpServerParams *server = new NntpServerParams(host,
                                                                port,
                                                                auth,
                                                                user.toStdString(),
                                                                pass.toStdString(),
                                                                nbCon,
                                                                ssl);
                _nntpServers << server;
            }
            else
            {
                _cerr << tr("Syntax error on server details for %1, the format should be: %2").arg(
                           serverParam).arg("(<user>:<pass>@@@)?<host>:<port>:<nbCons>:(no)?ssl");
                return false;
            }
        }
    }

    // Server Section under
    // check if the server params are given in the command line
    if (parser.isSet(sOptionNames[Opt::HOST]))
    {
        QString host = parser.value(sOptionNames[Opt::HOST]);

        NntpServerParams *server = new NntpServerParams(host);
        _nntpServers << server;

        if (parser.isSet(sOptionNames[Opt::SSL]))
        {
            server->useSSL = true;
            server->port = NntpServerParams::sDefaultSslPort;
        }

        if (parser.isSet(sOptionNames[Opt::PORT]))
        {
            bool ok;
            ushort port = parser.value(sOptionNames[Opt::PORT]).toUShort(&ok);
            if (ok)
                server->port = port;
            else
            {
                _cerr << tr("You should give an integer for the port (option -P)");
                return false;
            }
        }

        if (parser.isSet(sOptionNames[Opt::USER]))
        {
            server->auth = true;
            server->user = parser.value(sOptionNames[Opt::USER]).toStdString();

            if (parser.isSet(sOptionNames[Opt::PASS]))
                server->pass = parser.value(sOptionNames[Opt::PASS]).toStdString();

        }

        if (parser.isSet(sOptionNames[Opt::CONNECTION]))
        {
            bool ok;
            int nbCons = parser.value(sOptionNames[Opt::CONNECTION]).toInt(&ok);
            if (ok)
                server->nbCons = nbCons;
            else
            {
                _cerr << tr("You should give an integer for the number of connections (option -n)");
                return false;
            }
        }
    }

    if (_nntpServers.isEmpty())
    {
        _cerr << tr("Error: you should at least provide one Usenet provider using -S or (-h, -p, -u, -P...)") << "\n" << MB_FLUSH;
        return false;
    }



    return true;
}

void NzbCheck::_showVersionASCII()
{
    _cout << sASCII
          << "                          v" << sVersion << "\n\n" << MB_FLUSH;

}

void NzbCheck::_syntax(char *appName)
{
    QString app = QFileInfo(appName).fileName();
    _cout << tr("Syntax: ") << app << " (options)* -i <nzb file>\n";
    for (const QCommandLineOption & opt : sCmdOptions)
    {
        if (opt.valueName() == sOptionNames[Opt::SERVER])
            _cout << "\n// " << tr("you can provide servers in one string using -S and/or split the parameters for ONE SINGLE server") << "\n";

        if (opt.names().size() == 1)
            _cout << QString("\t--%1: %2\n").arg(opt.names().first(), -17).arg(tr(opt.description().toLocal8Bit().constData()));
        else
            _cout << QString("\t-%1: %2\n").arg(opt.names().join(" or --"), -18).arg(tr(opt.description().toLocal8Bit().constData()));
    }
    _cout << "\nExamples:\n"
          << "  - " << appName << " --progress -S \"user:password@@@news.usenetserver.com:563:50:ssl\" -i /nzb/myNzbFile.nzb\n"
          << "  - " << appName << " --quiet -h news.usenetserver.com -P 563 -u user -p password -n 50 -s -i /nzb/myNzbFile.nzb\n\n";

}

const QString NzbCheck::sASCII = QString("\
                 ___.   _________ .__                   __    \n\
     ____ _______\\_ |__ \\_   ___ \\|  |__   ____   ____ |  | __\n\
    /    \\\\___   /| __ \\/    \\  \\/|  |  \\_/ __ \\_/ ___\\|  |/ /\n\
   |   |  \\/    / | \\_\\ \\     \\___|   Y  \\  ___/\\  \\___|    < \n\
   |___|  /_____ \\|___  /\\______  /___|  /\\___  >\\___  >__|_ \\\n\
        \\/      \\/    \\/        \\/     \\/     \\/     \\/     \\/\n\
");
