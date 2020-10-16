#ifndef NNTPCON_H
#define NNTPCON_H
#include "NntpServerParams.h"
class NzbCheck;

#include <QObject>
#include <QTcpSocket>
class QSslSocket;
class QSslError;
class QByteArray;

class NntpCon : public QObject
{
    Q_OBJECT

private:
    enum class PostingState {NOT_CONNECTED = 0, CONNECTED,
                             AUTH_USER, AUTH_PASS,
                             IDLE, CHECKING_ARTICLE};

    NzbCheck *const        _nzbCheck;
    const int               _id;        //!< connection id
    const NntpServerParams &_srvParams; //!< server parameters

    QTcpSocket   *_socket;         //!< Real TCP socket
    bool          _isConnected;    //!< to avoid to rely on iSocket && iSocket->isOpen()

    PostingState   _postingState;
    QString        _currentArticle;

public:
    NntpCon(NzbCheck *nzbCheck, int id, const NntpServerParams &srvParams);
    ~NntpCon();

signals:
    void startConnection();
    void killConnection();

//    void error(QTcpSocket::SocketError socketerror); //!< Socket Error
    void socketError(QString aError);                //!< Error during socket creation (ssl or not)
    void errorConnecting(QString aError);
    void disconnected(NntpCon *con);


public slots:
    void onStartConnection();
    void onKillConnection();


    void onConnected();
    void onEncrypted();

    void onDisconnected();    //!< Handle disconnection

    void onReadyRead(); //!< To be overridden in Child class
    void onSslErrors(const QList<QSslError> &errors); //!< SSL errors handler
    void onErrors(QAbstractSocket::SocketError);      //!< Socket errors handler


private:
    void _closeConnection();
    void _checkNextArticle();
};

#endif // NNTPCON_H
