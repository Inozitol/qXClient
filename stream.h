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
#include "presence.h"
#include "contacts.h"
#include "message.h"
#include "chatchainmodel.h"

class Stream : public QObject{
    Q_OBJECT
public:
    Stream(std::shared_ptr<Account> client, std::shared_ptr<Server> server);
    ~Stream();

    static const unsigned int NONCE_INIT_LEN = 24;
    ChatChainModel* chatModel;
private:
    enum class ReaderState{
        INIT,
        STREAM,
        FEATURES,
        STARTTLS,
        SASL,
        RESOURCEBIND
    };

    enum class ReaderTrigger : quint32{
        FEATURE =   1 << 0,
        MESSAGE =   1 << 1,
        PRESENCE =  1 << 2,
        INFOQUERY = 1 << 3,
    };

    friend ReaderTrigger operator|(ReaderTrigger l, ReaderTrigger r);
    friend ReaderTrigger operator|=(ReaderTrigger& l, ReaderTrigger r);
    friend bool operator&(ReaderTrigger l, ReaderTrigger r);

    enum class IQPurpose{
        FEATURE,
        ROSTER
    };

    void initSocket();
    void reconnectSecure();

    void processFeatures();
    void processInfoQuery();
    void processMessage();

    inline void stateINIT(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger);
    inline void stateSTREAM(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger);
    inline void stateFEATURES(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger);
    inline void stateSTARTTLS(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger);
    inline void stateSASL(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger);
    inline void stateRESOURCEBIND(QXmlStreamReader::TokenType& token, QByteArray& name, ReaderTrigger& trigger);

    inline QByteArray getHeader();
    inline QByteArray getHeaderEnd();
    inline QByteArray getSTARTTLS();
    inline QByteArray getProceed();
    inline QByteArray getAuth();
    inline QByteArray getResponse();
    inline QByteArray getPresence(bool status);

    QSslSocket* _socket = nullptr;
    std::shared_ptr<Account> _acc;
    std::shared_ptr<Server> _srv;

    FeatureType _featuresActive{};
    QXmlStreamReader _reader;
    ReaderState _readerState = ReaderState::INIT;
    QMap<FeatureType, std::shared_ptr<Feature>> _features;
    QVector<SASLSupported> _saslSupport = {
        SASLSupported::SCRAM_SHA_1,
        SASLSupported::PLAIN
    };
    std::shared_ptr<SASL::SCRAMGenerator> _scramGenerator;
    QByteArray _srvStreamId;
    QMap<QByteArray, IQPurpose> _iqWaiting;
    QMap<QByteArray, InfoQuery> _iqResults;
    Contacts _contacts;

public slots:
    void connectInsecure();
    void queryRoster();
    void advertisePresence(bool status);

private slots:
    void sslSockStateChange(QAbstractSocket::SocketState sockState);
    void readData();
    void socketError(QAbstractSocket::SocketError error);
    void sslErrors(const QList<QSslError> &errors);
    void disconnectHandle();

signals:
    void finished();
    void coreEstablished();
};

#endif // STREAM_H
