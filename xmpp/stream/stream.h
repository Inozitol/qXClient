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
#include "../disco.h"
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

/** @brief Handles most of the semantics behind negotiations and IO.
 *
 * Negotiatiates the creation of XMPP stream and maintains it's lifetime.
 * Should run inside StreamPool class.
 *
 */
class Stream : public QObject{
    Q_OBJECT
public:
    Stream(const Account& account, const Server& server);
    Stream(Account&& account, Server&& server);
    ~Stream();

    /**
     * @brief Returns jid currently stored inside provided account.
     * @return jid of provided account
     *
     * Be aware that jid inside the account will change just before emitting coreEstablished()
     * (see processFeatures(), case RESOURCEBIND).
     *
     */
    jidbare_t accountJid();

    /**
     * @brief Returns QAbstractItemModel of Contact items.
     * @return Abstract item model consiting of Contact items.
     */
    ContactTreeModel* contactsModel();

    /**
     * @brief Returns a chat chain that's associated with the given bare jid.
     * @param jid A jid that's refering to a contact, into which a chat chain is tight.
     * @return A chat chain with which the jid is associated.
     */
    ChatChain* chatChain(const jidbare_t& jid);

    /**
     * @brief Length of nonce value used in SASL negotiation.
     */
    static const unsigned int NONCE_INIT_LEN = 24;

private:

    /**
     * @brief States between which the XMLReader (m_xmlReader) will switch as it reads.
     *
     * These states construct a kind of state machine, which decides on which part of negotiation the reader currently is.
     * Due to the fact that processFeatures() can change this state, it is not a completely isolated FSM.
     *
     */
    enum class ReaderState{
        INIT,
        STREAM,
        FEATURES,
        STARTTLS,
        SASL,
        RESOURCEBIND
    };

    /**
     * @brief Flags that can be triggered in the reader
     *
     * FEATURE - A feature has been created/modified in the m_umapEmpheralFeatures
     * INFOQUERY - InfoQuery stanza has been loaded and placed into m_umapIQResults
     *
     */
    enum ReaderTrigger{
        FEATURE =   1 << 0,
        INFOQUERY = 1 << 1,
    };

    /**
     * @brief Defines a reason behind expected InfoQuery
     *
     * FEATURE - Some feature expects this IQ, so we should call processFeatures()
     * ROSTER - A roster processor expects this IQ, which is handled by processInfoQuery()
     * DISCO - Service discovery probably sended result, which is handled by processInfoQuery()
     *
     */
    enum class IQPurpose{
        FEATURE,
        ROSTER,
        DISCO
    };

    /* Initializes SSL socket */
    void initSocket();

    /* Initializes signals to be called when core is established */
    void initSignals();

    /* Promotes the socket to use TLS encryption */
    void reconnectSecure();

    /* Handles logic behind features */
    void processFeatures();

    /* Handles InfoQueries unrelated to features (e.g. roster) */
    void processInfoQuery();

    /**
     * @brief Handles initialization state of the XML reader.
     * @param Current XML token
     * @param Name of the current XML entity
     * @param Flags to trigger later processors
     *
     * Always the first state in case of new stream
     * This will only save ID of the stream and switch into STREAM state.
     *
     */
    inline void stateINIT(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger);

    /**
     * @brief Handles stream state of the XML reader.
     * @param Current XML token
     * @param Name of the current XML entity
     * @param Flags to trigger later processors
     *
     * Reads all stanzas and some feature related elements.
     *
     */
    inline void stateSTREAM(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger);

    /**
     * @brief Handles feature state of the XML reader.
     * @param Current XML token
     * @param Name of the current XML entity
     * @param Flags to trigger later processors
     *
     * Reads first level elements of the element <features>.
     * Will switch into appropriate state, related to the discovered feature (in case it's supported and the state exists)
     *
     */
    inline void stateFEATURES(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger);

    /**
     * @brief Handles STARTTLS state of the XML reader.
     * @param Current XML token
     * @param Name of the current XML entity
     * @param Flags to trigger later processors
     *
     * Creates appropriate feature in m_umapEmpheralFeatures to be handled by processFeatures().
     * Feature processor will switch into this state manually, as it expects XML elements related to this feature.
     *
     */
    inline void stateSTARTTLS(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger);

    /**
     * @brief Handles SASL state of the XML reader.
     * @param Current XML token
     * @param Name of the current XML entity
     * @param Flags to trigger later processors
     *
     * Creates appropriate feature in m_umapEmpheralFeatures to be handled by processFeatures().
     * Feature processor will switch into this state manually, as it expects XML elements related to this feature.
     *
     */
    inline void stateSASL(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger);

    /**
     * @brief Handles resource bind state of the XML reader.
     * @param Current XML token
     * @param Name of the current XML entity
     * @param Flags to trigger later processors
     *
     * Creates appropriate feature in m_umapEmpheralFeatures to be handled by processFeatures().
     * Feature processor will NOT switch into this state manually, as it expects reponse in iq stanzas, which are handled by stream state.
     *
     */
    inline void stateRESOURCEBIND(QXmlStreamReader::TokenType& token, QByteArray& name, QFlags<ReaderTrigger>& trigger);

    /* Returns stream initialization tag */
    inline QByteArray getHeader();

    /* Returns stream ending tag */
    inline QByteArray getHeaderEnd();

    /* Returns request tag for STARTTLS */
    inline QByteArray getSTARTTLS();

    /* Returns proceed tag */
    inline QByteArray getProceed();

    /* Returns authorization tag for SASL */
    inline QByteArray getAuth();

    /* Returns response tag for SASL */
    inline QByteArray getResponse();

    /**
     * @brief Returns presence tag, with or without 'unavailable' attribute
     * @param status Whether the user is going online or offline
     */
    inline QByteArray getPresence(bool status);

    /* Returns enable tag for stream management */
    inline QByteArray getManagementEnable();

    /* Returns request (r) tag for stream management */
    inline QByteArray getManagementReq();

    /**
     * @brief Returns acknowledge (a) tag for stream management
     * @param Number of inbound stanzas processed
     */
    inline QByteArray getManagementAck(quint32 count);

    QSslSocket*                 m_ptrSocket = nullptr;
    std::unique_ptr<Account>    m_uptrAccount;
    std::unique_ptr<Server>     m_uptrServer;

    Feature::Type m_flgActiveFeatures{};
    std::unordered_map<Feature::Type, std::shared_ptr<Feature>> m_umapEpheralFeatures;
    std::unordered_map<Feature::Type, std::shared_ptr<Feature>> m_umapPersistentFeatures;

    bool m_isCoreEstablished = false;
    bool m_isConnClosing = false;

    QXmlStreamReader        m_xmlReader;
    ReaderState             m_xmlReaderState = ReaderState::INIT;

    QVector<SASLSupported>  m_qvecSASL = {
        SASLSupported::SCRAM_SHA_1,
        SASLSupported::PLAIN
    };
    std::shared_ptr<SASL::SCRAMGenerator> m_SCRAMGenerator;

    QByteArray m_streamID;
    std::unordered_map<QByteArray, IQPurpose> m_umapIQWaiting;
    std::unordered_map<QByteArray, InfoQuery> m_umapIQResults;

    ContactTreeModel*                         m_ptrContactsModel;
    std::unordered_map<jidbare_t, ChatChain*> m_umapChatChains;

public slots:
    /**
     * @brief Connects the socket without TLS layer
     *
     * Will connect and send stream opening tag
     *
     */
    void connectInsecure();

    /**
     * @brief Sends a request to receive a users roster
     */
    void queryRoster();

    /**
     * @brief Sends a request to service discovery
     */
    void queryDisco();

    /**
     * @brief Advertises clients presence to the server
     * @param status Online/Offline state
     */
    void advertisePresence(bool status);

    /**
     * @brief Initialies disconnect process
     *
     * Sends ending stream header and switches stream into closing state.
     */
    void initDisconnect();

private slots:

    /**
     * @brief Called whenever the socket switches its state
     * @param sockState State into which it has been switched.
     */
    void sslSockStateChange(QAbstractSocket::SocketState sockState);

    /**
     * @brief Reads every incoming data from server
     *
     * Branches into all XML reader states, and calls feature/iq processors if needed.
     *
     */
    void readData();

    /**
     * @brief Called whenever a socket related error occured.
     * @param error What kind of error.
     */
    void socketError(QAbstractSocket::SocketError error);

    /**
     * @brief Called whenever a TLS related error occured.
     * @param errors What kind o error.
     */
    void sslErrors(const QList<QSslError> &errors);

    /**
     * @brief Sends stanza
     * @param stanza Stanza to be send
     */
    void sendStanza(const Stanza& stanza);

    /**
     * @brief Finishes disconnect process
     *
     * Called whenever the server sends it's closing tag.
     * In case local stream wasn't in closing state, will also send a closing tag.
     *
     */
    void finishDisconnect();

signals:

    /**
     * @brief Emited whenever the stream has disconnected
     */
    void disconnected();

    /**
     * @brief Emited whenever the stream core has been established
     *
     * Core is established after the stream has negotiated through TLS, SASL and received resource binding.
     */
    void coreEstablished();

    /**
     * @brief Emited when a new message has arrived
     * @param msg Message that arrived
     */
    void receivedMessage(const Message& msg);
};

#endif // STREAM_H
