#ifndef DISCO_H
#define DISCO_H

#include <QByteArray>
#include <QDomElement>
#include "addressable.h"
#include "account.h"
#include "server.h"
#include "stanza/infoquery.h"
#include "stanza/presence.h"
#include "../strswitch.h"

struct caps_data;
struct discofeature_t;
struct discoidentity_t;
struct discoitem_t;

class Disco : public QObject{
    Q_OBJECT

public:
    Disco(std::shared_ptr<Account> account, std::shared_ptr<Server> server, QObject* parent = nullptr);

    /** @brief Service Discovery Features
     *
     * Service Discovery Features as per XMPP Registrar
     * https://xmpp.org/registrar/disco-features.html
     * revision 2021-10-10
     */
    enum Feature{
        dnssrv,
        fullunicode,
        gc10,
        activity,
        address,
        amp,
        amp_errors,
        amp_action_alert,
        amp_action_drop,
        amp_action_error,
        amp_action_notify,
        amp_condition_deliver,
        amp_condition_expireat,
        amp_condition_matchresource,
        bytestreams,
        bytestreams_udp,
        caps,
        caps_optimize,
        chatstates,
        commands,
        compress,
        disco_info,
        disco_items,
        feature_neg,
        geoloc,
        httpauth,
        httpbind,
        ibb,
        mood,
        muc,
        muc_admin,
        muc_owner,
        muc_register,
        muc_roomconfig,
        muc_roominfo,
        muc_user,
        muc_hidden,
        muc_membersonly,
        muc_moderated,
        muc_nonanonymous,
        muc_open,
        muc_passwordprotected,
        muc_persistent,
        muc_public,
        muc_rooms,
        muc_semianonymous,
        muc_temporary,
        muc_unmoderated,
        muc_unsecured,
        offline,
        pubsub_access_authorize,
        pubsub_access_open,
        pubsub_access_presence,
        pubsub_access_roster,
        pubsub_access_whitelist,
        pubsub_auto_create,
        pubsub_auto_subscribe,
        pubsub_collections,
        pubsub_config_node,
        pubsub_create_and_configure,
        pubsub_create_nodes,
        pubsub_delete_any,
        pubsub_delete_nodes,
        pubsub_filtered_notifications,
        pubsub_get_pending,
        pubsub_instant_nodes,
        pubsub_item_ids,
        pubsub_last_published,
        pubsub_leased_subscription,
        pubsub_manage_subscription,
        pubsub_member_affiliation,
        pubsub_metadata,
        pubsub_modify_affiliations,
        pubsub_multi_collection,
        pubsub_multi_subscribe,
        pubsub_outcast_affiliation,
        pubsub_persistent_items,
        pubsub_presence_notifications,
        pubsub_presence_subscribe,
        pubsub_publish,
        pubsub_publish_options,
        pubsub_publisher_affiliation,
        pubsub_purge_nodes,
        pubsub_retract_items,
        pubsub_retrieve_affiliations,
        pubsub_retrieve_default,
        pubsub_retrieve_items,
        pubsub_retrieve_subscriptions,
        pubsub_subscribe,
        pubsub_subscription_options,
        pubsub_subscription_notifications,
        rosterx,
        sipub,
        soap,
        soap_fault,
        waitinglist,
        waitinglist_mailto,
        waitinglist_tel,
        xhtml_im,
        xdata_layout,
        xdata_validate,
        ipv6,
        jabber_client,
        jabber_component_accept,
        jabber_component_connect,
        jabber_iq_auth,
        jabber_iq_gateway,
        jabber_iq_last,
        jabber_iq_oob,
        jabber_iq_privacy,
        jabber_iq_private,
        jabber_iq_register,
        jabber_iq_roster,
        jabber_iq_rpc,
        jabber_iq_search,
        jabber_iq_version,
        jabber_server,
        jabber_x_data,
        jabber_x_encrypted,
        jabber_x_oob,
        jabber_x_signed,
        msglog,
        msgoffline,
        roster_delimiter,
        sslc2s,
        stringprep,
        bind,
        e2e,
        sasl,
        sasl_c2s,
        sasl_s2s,
        session,
        stanzas,
        streams,
        tls,
        tls_c2s,
        tls_s2s,
        rfc3264,
        archive_auto,
        archive_manage,
        archive_manual,
        archive_pref,
        avatar_data,
        avatar_metadata,
        avatar_metadata_notify,
        delay,
        audio,
        video,
        ping,
        receipts,
        ssn,
        time,
        xmllang,
        vcard_temp,
        styling,
        sid,
        caps2,
        caps2_optimize
    };
    Q_DECLARE_FLAGS(Features, Feature);

    /** @brief Service Discovery Identities
     *
     * Service Discovery Identities as per XMPP Registrar
     * https://xmpp.org/registrar/disco-categories.html
     * revision 2021-10-06
     */
    enum Identity{
        account_admin,
        account_anonymous,
        account_registered,

        auth_cert,
        auth_generic,
        auth_ldap,
        auth_ntlm,
        auth_pam,
        auth_radius,

        authz_ephemeral,

        automation_command_list,
        automation_command_node,
        automation_rpc,
        automation_soap,
        automation_translation,

        client_bot,
        client_console,
        client_game,
        client_handheld,
        client_pc,
        client_phone,
        client_sms,
        client_tablet,
        client_web,

        collaboration_whiteboard,

        component_archive,
        component_c2s,
        component_generic,
        component_load,
        component_log,
        component_presence,
        component_router,
        component_s2s,
        component_sm,
        component_stats,

        conference_irc,
        conference_text,

        directory_chatroom,
        directory_group,
        directory_user,
        directory_waitinglist,

        gateway_aim,
        gateway_facebook,
        gateway_gadu_gadu,
        gateway_http_ws,
        gateway_icq,
        gateway_irc,
        gateway_lcs,
        gateway_mrim,
        gateway_msn,
        gateway_myspaceim,
        gateway_ocs,
        gateway_pstn,
        gateway_qq,
        gateway_sametime,
        gateway_simple,
        gateway_skype,
        gateway_sms,
        gateway_smtp,
        gateway_telegram,
        gateway_tlen,
        gateway_xfire,
        gateway_xmpp,
        gateway_yahoo,

        headline_newmail,
        headline_rss,
        headline_weather,

        hierarchy_branch,
        hierarchy_leaf,

        proxy_bytestreams,

        pubsub_collection,
        pubsub_leaf,
        pubsub_pep,
        pubsub_service,

        server_im,

        store_berkeley,
        store_file,
        store_generic,
        store_ldap,
        store_mysql,
        store_oracle,
        store_postgres,
    };
    Q_DECLARE_FLAGS(Identities, Identity);

    void initQuery();

    void processQuery(const InfoQuery& query);
    void queryPresence(const Presence& presence);

    static Identity str2identity(const discoidentity_t& identity);
    static discoidentity_t identity2str(Identity identity);

    static Feature str2feature(const discofeature_t& feature);
    static discofeature_t feature2str(Feature feature);

private:
    void queryItems(const discoitem_t& item);
    void queryInfo(const discoitem_t& item);

    std::shared_ptr<Account> m_sptrAccount;
    std::shared_ptr<Server>  m_sptrServer;

    std::shared_ptr<discoitem_t> m_rootItem;

    std::unordered_map<QByteArray, std::shared_ptr<discoitem_t>> m_umapCapabilities;
    std::unordered_map<jidfull_t, std::shared_ptr<discoitem_t>>  m_umapClientItems;

signals:
    void sendInfoQuery(const InfoQuery& iq);
    void gotFeature(const jidfull_t& jid, Disco::Feature feature);
    void pubsubNodeDiscovered(const discoitem_t& item);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Disco::Features);
Q_DECLARE_OPERATORS_FOR_FLAGS(Disco::Identities);


struct discofeature_t{
    QByteArray var;

    discofeature_t(QDomElement feature){
        if(feature.nodeName() != "feature"){
            throw std::invalid_argument("discofeature_t expects DOM element of feature");
        }
        var = feature.attribute("var").toUtf8();
    }
    discofeature_t() = default;
};

struct discoidentity_t{
    QByteArray category;
    QByteArray name;
    QByteArray type;

    discoidentity_t(QDomElement identity){
        if(identity.nodeName() != "identity"){
            throw std::invalid_argument("discoidentity_t expects DOM attribute of identity");
        }
        category = identity.attribute("category").toUtf8();
        name =     identity.attribute("name").toUtf8();
        type =     identity.attribute("type").toUtf8();
    }

    discoidentity_t() = default;
};

struct discoitem_t{
    jidfull_t jid;
    QByteArray name;
    QByteArray node;

    std::unordered_map<jidfull_t,std::shared_ptr<discoitem_t>> children;
    Disco::Features flgFeatures;
    Disco::Identities flgIdentities;

    discoitem_t()=default;

    discoitem_t(QDomElement item){
        if(item.nodeName() != "item"){
            throw std::invalid_argument("discoitem_t expects DOM element of item");
        }
        jid = item.attribute("jid");
        name = item.attribute("name").toUtf8();
        node = item.attribute("node").toUtf8();
    }

    discoitem_t(const discoitem_t& item){
        jid = item.jid;
        name = item.name;
        node = item.node;
    }

    void addChild(std::shared_ptr<discoitem_t> item){
        children.insert({item->jid, item});
    }

    std::shared_ptr<discoitem_t> findItem(const jidfull_t& jid){
        for(auto& child : children){
            if(child.first == jid){
                return child.second;
            }else{
                return child.second->findItem(jid);
            }
        }
        return nullptr;
    }

    discoitem_t& operator=(const discoitem_t& other)=default;
    discoitem_t& operator=(discoitem_t&& other)=default;
};

struct caps_data{
    static const Disco::Identity identity;
    static const std::vector<Disco::Feature> features;
    static const QCryptographicHash::Algorithm hashAlgo;
    static const QByteArray node;
    static const QByteArray name;
    static QByteArray getVerString(){
        QCryptographicHash hashEngine(hashAlgo);
        QByteArray verStr;
        discoidentity_t identityStr = Disco::identity2str(identity);
        std::vector<discofeature_t> featuresStr;
        for(auto feature : features){
            featuresStr.push_back(Disco::feature2str(feature));
        }
        std::sort(featuresStr.begin(),
                  featuresStr.end(),
                  [](const discofeature_t& l, const discofeature_t& r){
                  return l.var.compare(r.var);
        });
        verStr.append(identityStr.category);
        verStr.append("/");
        verStr.append(identityStr.type);
        verStr.append("//");
        verStr.append(name);
        verStr.append("<");
        for(auto featureStr : featuresStr){
            verStr.append(featureStr.var);
            verStr.append("<");
        }
        hashEngine.addData(verStr);
        QByteArray output = hashEngine.result();
        hashEngine.reset();
        return output;
    }
};


#endif // DISCO_H
