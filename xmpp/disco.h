#ifndef DISCO_H
#define DISCO_H

#include <QByteArray>
#include <QDomElement>

struct discoidentity_t{
    QByteArray category;
    QByteArray name;
    QByteArray type;

    discoidentity_t(QDomElement identity){
        if(identity.nodeName() != "identity"){
            throw std::invalid_argument("discoid_t expects DOM element of identity");
        }
        category = identity.attribute("category").toUtf8();
        name =     identity.attribute("name").toUtf8();
        type =     identity.attribute("type").toUtf8();
    }
};

struct discofeature_t{
    QByteArray var;

    discofeature_t(QDomElement feature){
        if(feature.nodeName() != "feature"){
            throw std::invalid_argument("discofeature_t expects DOM element of feature");
        }
        var = feature.attribute("var").toUtf8();
    }
};

class Disco{
public:
    Disco();

    void insertIdentity(const discoidentity_t& identity);
    void insertFeature(const discofeature_t& feature);

    enum Feature{
        dnssrv,
        fullunicode,
        gc10,
        activity,
        address,
        amp,
        amp_erros,
        amp_alert,
        amp_drop,
        amp_notify,
        amp_deliver,
        amp_expire,
        amp_matchresource,
        bytestreams,
        bytestreams_utp,
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
        muc_presistent,
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
        pubsub_leased_published,
        pubsub_manage_subscription,
        pubsub_member_affiliation,
        pubsub_metadata,
        pubsub_modify_affiliations,
        pubsub_multi_colection,
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
        xmpp_bind,
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
        caps2,
        caps2_optimize
    };

private:
    Q_DECLARE_FLAGS(m_features, Feature);

};


#endif // DISCO_H
