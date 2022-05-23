#ifndef STREAM_H
#define STREAM_H

#include <QThread>
#include <QSslSocket>
#include <QtXml>
#include <QByteArray>
#include <QMessageBox>
#include <QDialog>
#include <QSslConfiguration>
#include <QMap>
#include <QSet>

#include "utils.h"
#include "account.h"
#include "server.h"
#include "streamfeature.h"
#include "saslmechanisms.h"
#include "creds.h"
#include "scramgenerator.h"
#include "strswitch.h"
#include "infoquery.h"

class Stream : public QObject{
    Q_OBJECT
public:
    Stream(std::shared_ptr<Account> client, std::shared_ptr<Server> server);
    ~Stream();

    static const unsigned int NONCE_INIT_LEN = 24;
private:
    enum class ReaderState{
        INIT,
        STREAM,
        FEATURES,
        STARTTLS,
        SASL,
        RESOURCEBIND
    };

    enum class StreamState{
        INIT,
        NEGOTIATE
    };

    inline QByteArray getHeader();
    inline QByteArray getHeaderEnd();
    inline QByteArray getSTARTTLS();
    inline QByteArray getProceed();
    inline QByteArray getAuth();
    inline QByteArray getResponse();
    void initSocket();
    void processFeatures();
    void reconnectSecure();

    inline void stateINIT(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change);
    inline void stateSTREAM(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change);
    inline void stateFEATURES(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change);
    inline void stateSTARTTLS(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change);
    inline void stateSASL(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change);
    inline void stateRESOURCEBIND(QXmlStreamReader::TokenType& token, QByteArray& name, bool& ft_change);

    QSslSocket* _socket = nullptr;
    std::shared_ptr<Account> _acc;
    std::shared_ptr<Server> _srv;

    QXmlStreamReader _reader;
    ReaderState _readerState = ReaderState::INIT;
    QMap<FeatureType, std::shared_ptr<Feature>> _features;
    QVector<SASLSupported> _saslSupport = {
        SASLSupported::SCRAM_SHA_1,
        SASLSupported::PLAIN
    };
    std::shared_ptr<SASL::SCRAMGenerator> _scramGenerator;
    QByteArray _srvStreamId;


public slots:
    void connectInsecure();

private slots:
    void sslSockStateChange(QAbstractSocket::SocketState sockState);
    void readData();
    void socketError(QAbstractSocket::SocketError error);
    void sslErrors(const QList<QSslError> &errors);
    void disconnectHandle();

signals:
    void finished();
};

#endif // STREAM_H
