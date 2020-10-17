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

#ifndef NZBCHECK_H
#define NZBCHECK_H
#include <QObject>
#include <QStack>
#include <QString>
#include <QTextStream>
#include <QSet>
#include <QTimer>
#include <QCommandLineOption>
class NntpServerParams;
class NntpCon;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    #define MB_FLUSH flush
#else
    #define MB_FLUSH Qt::flush
#endif

class NzbCheck : public QObject
{
    Q_OBJECT

private:
    static constexpr const char *sAppName = "nzbCheck";
    static constexpr const char *sVersion = "1.2";
    static constexpr const char *sNntpServerStrRegExp = "^(([^:]+):([^@]+)@@@)?([\\w\\.\\-_]+):(\\d+):(\\d+):(no)?ssl$";
    static constexpr const char *sNntpArticleYencSubjectStrRegExp = "^\\[\\d+/\\d+\\]\\s+.+\\(\\d+/(\\d+)\\)$";

    enum class Opt {HELP = 0, VERSION,
                    PROGRESS, DEBUG, QUIET,
                    INPUT, SERVER, HOST, PORT, SSL, USER, PASS, CONNECTION
                   };

    static const QMap<Opt, QString>        sOptionNames;
    static const QList<QCommandLineOption> sCmdOptions;

    QString           _nzbPath;
    QStack<QString>   _articles;

    QTextStream       _cout; //!< stream for stdout
    QTextStream       _cerr; //!< stream for stderr

    int               _nbTotalArticles;
    int               _nbMissingArticles;
    int               _nbCheckedArticles;


    QList<NntpServerParams*> _nntpServers; //!< the servers parameters

    ushort            _debug;

    QSet<NntpCon*>    _connections;

    bool              _dispProgressBar;
    QTimer            _progressbarTimer;      //!< timer to refresh the upload information (progressbar bar, avg. speed)
    const int         _refreshRate;           //!< refresh rate

    bool              _quietMode;

    static const int sDefaultRefreshRate  = 200; //!< how often shall we refresh the progressbar bar?
#if defined( Q_OS_WIN )
    static const int sprogressbarBarWidth = 30;
#else
    static const int sprogressbarBarWidth = 50;
#endif
    static const QRegularExpression sNntpArticleYencSubjectRegExp;

public slots:
    void onDisconnected(NntpCon *con);
    void onRefreshprogressbarBar();


public:
    NzbCheck();
    ~NzbCheck();

    int parseNzb();
    void checkPost();

    bool parseCommandLine(int argc, char *argv[]);


    inline void missingArticle(const QString &article);
    inline QString getNextArticle();
    inline void articleChecked();

    inline int nbMissingArticles() const;
    inline bool debugMode() const;
    inline void setDebug(ushort level);


    inline void log(const QString     &aMsg);
    inline void log(const char        *aMsg);
    inline void log(const std::string &aMsg);
    inline void error(const QString     &aMsg);
    inline void error(const char        *aMsg);
    inline void error(const std::string &aMsg);

private:
    static const QString sASCII;
    void _showVersionASCII();
    void _syntax(char *appName);
};

void NzbCheck::missingArticle(const QString &article)
{
    if (!_quietMode)
        _cout << (_dispProgressBar ? "\n" : "")
              << tr("+ Missing Article on server: ") << article << "\n" << MB_FLUSH;
    ++_nbMissingArticles;
}

QString NzbCheck::getNextArticle()
{
    if (_articles.isEmpty())
        return QString();
    else
        return _articles.pop();
}

void NzbCheck::articleChecked() { ++_nbCheckedArticles; }

int NzbCheck::nbMissingArticles() const { return _nbMissingArticles; }

bool NzbCheck::debugMode() const { return _debug != 0; }
void NzbCheck::setDebug(ushort level) { _debug = level; }

void NzbCheck::log(const QString     &aMsg) { _cout << aMsg << "\n" << MB_FLUSH; }
void NzbCheck::log(const char        *aMsg) { _cout << aMsg << "\n" << MB_FLUSH; }
void NzbCheck::log(const std::string &aMsg) { _cout << aMsg.c_str() << "\n" << MB_FLUSH; }

void NzbCheck::error(const QString     &aMsg) { _cerr << aMsg << "\n" << MB_FLUSH; }
void NzbCheck::error(const char        *aMsg) { _cerr << aMsg << "\n" << MB_FLUSH; }
void NzbCheck::error(const std::string &aMsg) { _cerr << aMsg.c_str() << "\n" << MB_FLUSH; }

#endif // NZBCHECK_H
