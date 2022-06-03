#ifndef STREAM_H
#define STREAM_H

#include <QThread>
#include <QEventLoop>
#include <QSslSocket>
#include <QtXml>
#include <QByteArray>
#include <QMessageBox>
#include <QDialog>
#include <QSslConfiguration>
#include <QMap>
#include <QSet>

#include "utils.h"
#include "streamfeature.h"
#include "../creds.h"
#include "../contact.h"
#include "../account.h"
#include "../server.h"
#include "../chatchain.h"
#include "../contacttreemodel.h"
#include "../stanza/infoquery.h"
#include "../stanza/message.h"
#include "../stanza/presence.h"
#include "../../strswitch.h"
#include "../../sasl/saslmechanisms.h"
#include "../../sasl/scramgenerator.h"

class Stream : public QObject{
    Q_OBJECT
public:
    Stream(std::shared_ptr<Account> client, std::shared_ptr<Server> server);
    ~Stream();

    jidbare_t accountJid();
    ContactTreeModel* getContactsModel();
    ChatChain* getChatChain(const jidbare_t& jid);
    void setDescriptor(qintptr descriptor);

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
    qintptr _descriptor;
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
    std::map<QByteArray, IQPurpose> _iqWaiting;
    std::map<QByteArray, InfoQuery> _iqResults;
    ContactTreeModel* _contacts;
    std::map<jidbare_t, ChatChain*> _chatChains;
    QMutex _chainMutex;

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
    void sendMessage(Message& msg);

signals:
    void finished();
    void coreEstablished();
    void newContact(Contact*);
    void receivedMessage(const Message& msg);
};

#endif // STREAM_H
