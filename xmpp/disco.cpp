#include "disco.h"

const Disco::Identity caps_data::identity = Disco::Identity::client_pc;
const std::vector<Disco::Feature> caps_data::features = {
    Disco::Feature::disco_info,
    Disco::Feature::caps,
    Disco::Feature::avatar_metadata_notify
};
const QCryptographicHash::Algorithm caps_data::hashAlgo = QCryptographicHash::Sha1;
const QByteArray caps_data::node = "https://github.com/Inozitol/qXClient";
const QByteArray caps_data::name = "Gajim 0.0.1";

Disco::Disco(std::shared_ptr<Account> account, std::shared_ptr<Server> server, QObject* parent)
    :QObject(parent),m_sptrAccount(account), m_sptrServer(server){}

void Disco::processQuery(const InfoQuery& query){
    QDomElement payload = query.root().firstChildElement();
    switch(word2int(query.getType())){
    case IntFromString::get:
        switch(word2int(query.payloadNS().toUtf8())){
        case IntFromString::http_jabber_disco_info:
            if(payload.hasAttribute("node")){
                QByteArray node = payload.attribute("node").toUtf8();

                // Answering Entity Capabilites [XEP-0115]
                if(node == caps_data::node + '#' + caps_data::getVerString().toBase64()){
                    InfoQuery iq_result;
                    iq_result.setFrom(m_sptrAccount->jid().toByteArray());
                    iq_result.setTo(query.getFrom());
                    iq_result.setId(query.getID());
                    iq_result.setType("result");

                    QDomElement iq_payload = iq_result.createElementNS(
                                "http://jabber.org/protocol/disco#info",
                                 "query");
                    iq_payload.setAttribute("node", caps_data::node + '#' + caps_data::getVerString().toBase64());

                    discoidentity_t identity_struct = Disco::identity2str(caps_data::identity);
                    QDomElement iq_identity = iq_result.createElement("identity");
                    iq_identity.setAttribute("category", identity_struct.category);
                    iq_identity.setAttribute("type",     identity_struct.type);
                    iq_identity.setAttribute("name",     caps_data::name);
                    iq_payload.appendChild(iq_identity);

                    for(auto feature : caps_data::features){
                        discofeature_t feature_struct = Disco::feature2str(feature);
                        QDomElement iq_feature = iq_result.createElement("feature");
                        iq_feature.setAttribute("var", feature_struct.var);
                        iq_payload.appendChild(iq_feature);
                    }

                    iq_result.root().appendChild(iq_payload);

                    emit sendInfoQuery(iq_result);
                }
            }
            break;
        default:
            break;
        }

        break;
    case IntFromString::result:
        switch(word2int(query.payloadNS().toUtf8())){
        case IntFromString::http_jabber_disco_info:
        {
            jidfull_t fromJid = query.getFrom();
            std::shared_ptr<discoitem_t> ptrItem;
            if(fromJid.local.isEmpty()){ // For #info about server items
                if(fromJid == m_sptrServer->jid().bare()){
                    ptrItem = m_rootItem;
                }else{
                    ptrItem = m_rootItem->findItem(fromJid);
                }
            }else{ // For #info about clients
                if(m_umapClientItems.count(fromJid)){
                    ptrItem = m_umapClientItems.at(fromJid);
                }else{
                    ptrItem = nullptr;
                }
            }
            if(!ptrItem){
                break;
            }

            QDomNodeList infoList = payload.childNodes();
            for(int i=0; i<infoList.length(); i++){
                QDomElement currItem = infoList.at(i).toElement();

                switch(word2int(currItem.nodeName().toUtf8())){
                case IntFromString::identity:
                {
                    Identity identity = str2identity(currItem);
                    ptrItem->flgIdentities |= identity;
                }
                    break;
                case IntFromString::feature:
                {
                    Feature feature = str2feature(currItem);
                    ptrItem->flgFeatures |= feature;
                    emit gotFeature(fromJid, feature);
                }
                    break;
                default:
                    break;
                }
            }

            if(ptrItem->flgIdentities.testAnyFlags({Identity::pubsub_collection,
                                                   Identity::pubsub_leaf,
                                                   Identity::pubsub_pep,
                                                   Identity::pubsub_service})){
                emit pubsubNodeDiscovered(*ptrItem);
            }

        }
            break;
        case IntFromString::http_jabber_disco_items:
        {
            std::shared_ptr<discoitem_t> ptrItem;
            if(query.getFrom() == m_sptrServer->jid().bare().toByteArray()){
                ptrItem = m_rootItem;
            }else{
                ptrItem = m_rootItem->findItem(query.getFrom());
            }
            if(!ptrItem){
                break;
            }
            QDomNodeList itemsList = payload.childNodes();
            for(int i=0; i<itemsList.length(); i++){
                QDomElement currItem = itemsList.at(i).toElement();
                jidfull_t itemJid = currItem.attribute("jid");
                std::shared_ptr<discoitem_t> newItem = std::make_shared<discoitem_t>(currItem);
                ptrItem->addChild(newItem);
                queryItems(*newItem);
                queryInfo(*newItem);
            }
        }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Disco::initQuery(){
    InfoQuery iq_info;
    iq_info.setFrom(m_sptrAccount->jid().toByteArray());
    iq_info.setTo(m_sptrServer->jid().bare().toByteArray());
    iq_info.generateID();
    iq_info.setType("get");

    QDomElement query = iq_info.createElementNS("http://jabber.org/protocol/disco#info","query");
    iq_info.insertNode(query);

    emit sendInfoQuery(iq_info);

    InfoQuery iq_items;
    iq_items.setFrom(m_sptrAccount->jid().toByteArray());
    iq_items.setTo(m_sptrServer->jid().bare().toByteArray());
    iq_items.generateID();
    iq_items.setType("get");

    query = iq_items.createElementNS("http://jabber.org/protocol/disco#items","query");
    iq_items.insertNode(query);

    emit sendInfoQuery(iq_items);

    m_rootItem = std::make_shared<discoitem_t>();
    m_rootItem->jid = m_sptrServer->jid();
}

void Disco::queryItems(const discoitem_t& item){
    InfoQuery iq_items;
    iq_items.setFrom(m_sptrAccount->jid().toByteArray());
    iq_items.setTo(item.jid.toByteArray());
    iq_items.generateID();
    iq_items.setType("get");

    QDomElement query = iq_items.createElementNS("http://jabber.org/protocol/disco#items","query");
    if(!item.node.isEmpty()){
        query.setAttribute("node",item.node);
    }
    iq_items.insertNode(query);

    emit sendInfoQuery(iq_items);
}

void Disco::queryInfo(const discoitem_t& item){
    InfoQuery iq_info;
    iq_info.setFrom(m_sptrAccount->jid().toByteArray());
    iq_info.setTo(item.jid.toByteArray());
    iq_info.generateID();
    iq_info.setType("get");

    QDomElement query = iq_info.createElementNS("http://jabber.org/protocol/disco#info","query");
    if(!item.node.isEmpty()){
        query.setAttribute("node",item.node);
    }
    iq_info.insertNode(query);

    emit sendInfoQuery(iq_info);
}

void Disco::queryPresence(const Presence& presence){
    QDomElement capabilities = presence.root().firstChildElement("c", "http://jabber.org/protocol/caps");
    if(capabilities.isNull()){
        return;
    }

    QByteArray node = capabilities.attribute("node").toUtf8();
    QByteArray ver  = capabilities.attribute("ver").toUtf8();

    std::shared_ptr<discoitem_t> clientItem;
    if(m_umapCapabilities.count(ver)){
        clientItem = m_umapCapabilities.at(ver);
        m_umapClientItems.insert({clientItem->jid, clientItem});
    }else{
        clientItem = std::make_shared<discoitem_t>();
        clientItem->jid = presence.getFrom();
        clientItem->node = node+'#'+ver;

        m_umapCapabilities.insert({ver, clientItem});
        m_umapClientItems.insert({clientItem->jid, clientItem});
        queryInfo(*clientItem);
    }
}

Disco::Identity Disco::str2identity(const discoidentity_t& identity){
    switch(word2int(identity.category)){
    case IntFromString::account:
        switch(word2int(identity.type)){
        case IntFromString::admin:
            return Identity::account_admin;
            break;
        case IntFromString::anonymous:
            return Identity::account_anonymous;
            break;
        case IntFromString::registered:
            return Identity::account_registered;
            break;
        default:
            break;
        }
        break;
    case IntFromString::auth:
        switch(word2int(identity.type)){
        case IntFromString::cert:
            return Identity::auth_cert;
            break;
        case IntFromString::generic:
            return Identity::auth_generic;
            break;
        case IntFromString::ldap:
            return Identity::auth_ldap;
            break;
        case IntFromString::ntlm:
            return Identity::auth_ntlm;
            break;
        case IntFromString::pam:
            return Identity::auth_pam;
            break;
        case IntFromString::radius:
            return Identity::auth_radius;
            break;
        default:
            break;
        }
        break;
    case IntFromString::authz:
        switch(word2int(identity.type)){
        case IntFromString::ephemeral:
            return Identity::authz_ephemeral;
            break;
        default:
            break;
        }
        break;
    case IntFromString::automation:
        switch(word2int(identity.type)){
        case IntFromString::command_list:
            return Identity::automation_command_list;
            break;
        case IntFromString::command_node:
            return Identity::automation_command_node;
            break;
        case IntFromString::rpc:
            return Identity::automation_rpc;
            break;
        case IntFromString::soap:
            return Identity::automation_soap;
            break;
        case IntFromString::translation:
            return Identity::automation_translation;
            break;
        default:
            break;
        }
        break;
    case IntFromString::client:
        switch(word2int(identity.type)){
        case IntFromString::bot:
            return Identity::client_bot;
            break;
        case IntFromString::console:
            return Identity::client_console;
            break;
        case IntFromString::game:
            return Identity::client_game;
            break;
        case IntFromString::handheld:
            return Identity::client_handheld;
            break;
        case IntFromString::pc:
            return Identity::client_pc;
            break;
        case IntFromString::phone:
            return Identity::client_phone;
            break;
        case IntFromString::sms:
            return Identity::client_sms;
            break;
        case IntFromString::tablet:
            return Identity::client_tablet;
            break;
        case IntFromString::web:
            return Identity::client_web;
            break;
        default:
            break;
        }
        break;
    case IntFromString::collaboration:
        switch(word2int(identity.type)){
        case IntFromString::whiteboard:
            return Identity::collaboration_whiteboard;
            break;
        default:
            break;
        }
        break;
    case IntFromString::component:
        switch(word2int(identity.type)){
        case IntFromString::archive:
            return Identity::component_archive;
            break;
        case IntFromString::c2s:
            return Identity::component_c2s;
            break;
        case IntFromString::generic:
            return Identity::component_generic;
            break;
        case IntFromString::load:
            return Identity::component_load;
            break;
        case IntFromString::log:
            return Identity::component_log;
            break;
        case IntFromString::presence:
            return Identity::component_presence;
            break;
        case IntFromString::router:
            return Identity::component_router;
            break;
        case IntFromString::s2s:
            return Identity::component_s2s;
            break;
        case IntFromString::sm:
            return Identity::component_sm;
            break;
        case IntFromString::stats:
            return Identity::component_stats;
            break;
        default:
            break;
        }
        break;
    case IntFromString::conference:
        switch(word2int(identity.type)){
        case IntFromString::irc:
            return Identity::conference_irc;
            break;
        case IntFromString::text:
            return Identity::conference_text;
            break;
        default:
            break;
        }
        break;
    case IntFromString::directory:
        switch(word2int(identity.type)){
        case IntFromString::chatroom:
            return Identity::directory_chatroom;
            break;
        case IntFromString::group:
            return Identity::directory_group;
            break;
        case IntFromString::user:
            return Identity::directory_user;
            break;
        case IntFromString::waitinglist:
            return Identity::directory_waitinglist;
            break;
        default:
            break;
        }
        break;
    case IntFromString::gateway:
        switch(word2int(identity.type)){
        case IntFromString::aim:
            return Identity::gateway_aim;
            break;
        case IntFromString::facebook:
            return Identity::gateway_facebook;
            break;
        case IntFromString::gadu_gadu:
            return Identity::gateway_gadu_gadu;
            break;
        case IntFromString::http_ws:
            return Identity::gateway_http_ws;
            break;
        case IntFromString::icq:
            return Identity::gateway_icq;
            break;
        case IntFromString::irc:
            return Identity::gateway_irc;
            break;
        case IntFromString::lcs:
            return Identity::gateway_lcs;
            break;
        case IntFromString::mrim:
            return Identity::gateway_mrim;
            break;
        case IntFromString::msn:
            return Identity::gateway_msn;
            break;
        case IntFromString::myspaceim:
            return Identity::gateway_myspaceim;
            break;
        case IntFromString::ocs:
            return Identity::gateway_ocs;
            break;
        case IntFromString::pstn:
            return Identity::gateway_pstn;
            break;
        case IntFromString::qq:
            return Identity::gateway_qq;
            break;
        case IntFromString::sametime:
            return Identity::gateway_sametime;
            break;
        case IntFromString::simple:
            return Identity::gateway_simple;
            break;
        case IntFromString::skype:
            return Identity::gateway_skype;
            break;
        case IntFromString::sms:
            return Identity::gateway_sms;
            break;
        case IntFromString::smtp:
            return Identity::gateway_smtp;
            break;
        case IntFromString::telegram:
            return Identity::gateway_telegram;
            break;
        case IntFromString::tlen:
            return Identity::gateway_tlen;
            break;
        case IntFromString::xfire:
            return Identity::gateway_xfire;
            break;
        case IntFromString::xmpp:
            return Identity::gateway_xmpp;
            break;
        case IntFromString::yahoo:
            return Identity::gateway_yahoo;
            break;
        default:
            break;
        }
        break;
    case IntFromString::headline:
        switch(word2int(identity.type)){
        case IntFromString::newmail:
            return Identity::headline_newmail;
            break;
        case IntFromString::rss:
            return Identity::headline_rss;
            break;
        case IntFromString::weather:
            return Identity::headline_weather;
            break;
        default:
            break;
        }
        break;
    case IntFromString::hierarchy:
        switch(word2int(identity.type)){
        case IntFromString::branch:
            return Identity::hierarchy_branch;
            break;
        case IntFromString::leaf:
            return Identity::hierarchy_leaf;
            break;
        default:
            break;
        }
        break;
    case IntFromString::proxy:
        switch(word2int(identity.type)){
        case IntFromString::bytestreams:
            return Identity::proxy_bytestreams;
            break;
        default:
            break;
        }
        break;
    case IntFromString::pubsub:
        switch(word2int(identity.type)){
        case IntFromString::collection:
            return Identity::pubsub_collection;
            break;
        case IntFromString::leaf:
            return Identity::pubsub_leaf;
            break;
        case IntFromString::pep:
            return Identity::pubsub_pep;
            break;
        case IntFromString::service:
            return Identity::pubsub_service;
            break;
        default:
            break;
        }
        break;
    case IntFromString::server:
        switch(word2int(identity.type)){
        case IntFromString::im:
            return Identity::server_im;
            break;
        default:
            break;
        }
        break;
    case IntFromString::store:
        switch(word2int(identity.type)){
        case IntFromString::berkeley:
            return Identity::store_berkeley;
            break;
        case IntFromString::file:
            return Identity::store_file;
            break;
        case IntFromString::generic:
            return Identity::store_generic;
            break;
        case IntFromString::ldap:
            return Identity::store_ldap;
            break;
        case IntFromString::mysql:
            return Identity::store_mysql;
            break;
        case IntFromString::oracle:
            return Identity::store_oracle;
            break;
        case IntFromString::postgres:
            return Identity::store_postgres;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

discoidentity_t Disco::identity2str(Identity identity){
    discoidentity_t output;
    switch(identity){
    case Disco::account_admin:
        output.category = "account";
        output.type = "admin";
        break;
    case Disco::account_anonymous:
        output.category = "account";
        output.type = "anonymous";
        break;
    case Disco::account_registered:
        output.category = "account";
        output.type = "registered";
        break;
    case Disco::auth_cert:
        output.category = "auth";
        output.type = "cert";
        break;
    case Disco::auth_generic:
        output.category = "auth";
        output.type = "generic";
        break;
    case Disco::auth_ldap:
        output.category = "auth";
        output.type = "ldap";
        break;
    case Disco::auth_ntlm:
        output.category = "auth";
        output.type = "ntlm";
        break;
    case Disco::auth_pam:
        output.category = "auth";
        output.type = "pam";
        break;
    case Disco::auth_radius:
        output.category = "auth";
        output.type = "radius";
        break;
    case Disco::authz_ephemeral:
        output.category = "authz";
        output.type = "ephemeral";
        break;
    case Disco::automation_command_list:
        output.category = "automation";
        output.type = "command-list";
        break;
    case Disco::automation_command_node:
        output.category = "automation";
        output.type = "command-node";
        break;
    case Disco::automation_rpc:
        output.category = "automation";
        output.type = "rpc";
        break;
    case Disco::automation_soap:
        output.category = "automation";
        output.type = "soap";
        break;
    case Disco::automation_translation:
        output.category = "automation";
        output.type = "translation";
        break;
    case Disco::client_bot:
        output.category = "client";
        output.type = "bot";
        break;
    case Disco::client_console:
        output.category = "client";
        output.type = "console";
        break;
    case Disco::client_game:
        output.category = "client";
        output.type = "game";
        break;
    case Disco::client_handheld:
        output.category = "client";
        output.type = "handheld";
        break;
    case Disco::client_pc:
        output.category = "client";
        output.type = "pc";
        break;
    case Disco::client_phone:
        output.category = "client";
        output.type = "phone";
        break;
    case Disco::client_sms:
        output.category = "client";
        output.type = "sms";
        break;
    case Disco::client_tablet:
        output.category = "client";
        output.type = "tablet";
        break;
    case Disco::client_web:
        output.category = "client";
        output.type = "web";
        break;
    case Disco::collaboration_whiteboard:
        output.category = "collaboration";
        output.type = "whiteboard";
        break;
    case Disco::component_archive:
        output.category = "component";
        output.type = "archive";
        break;
    case Disco::component_c2s:
        output.category = "component";
        output.type = "c2s";
        break;
    case Disco::component_generic:
        output.category = "component";
        output.type = "generic";
        break;
    case Disco::component_load:
        output.category = "component";
        output.type = "load";
        break;
    case Disco::component_log:
        output.category = "component";
        output.type = "log";
        break;
    case Disco::component_presence:
        output.category = "component";
        output.type = "presence";
        break;
    case Disco::component_router:
        output.category = "component";
        output.type = "router";
        break;
    case Disco::component_s2s:
        output.category = "component";
        output.type = "s2s";
        break;
    case Disco::component_sm:
        output.category = "component";
        output.type = "sm";
        break;
    case Disco::component_stats:
        output.category = "component";
        output.type = "stats";
        break;
    case Disco::conference_irc:
        output.category = "conference";
        output.type = "irc";
        break;
    case Disco::conference_text:
        output.category = "conference";
        output.type = "text";
        break;
    case Disco::directory_chatroom:
        output.category = "directory";
        output.type = "chatroom";
        break;
    case Disco::directory_group:
        output.category = "directory";
        output.type = "group";
        break;
    case Disco::directory_user:
        output.category = "directory";
        output.type = "user";
        break;
    case Disco::directory_waitinglist:
        output.category = "directory";
        output.type = "waitinglist";
        break;
    case Disco::gateway_aim:
        output.category = "gateway";
        output.type = "aim";
        break;
    case Disco::gateway_facebook:
        output.category = "gateway";
        output.type = "facebook";
        break;
    case Disco::gateway_gadu_gadu:
        output.category = "gateway";
        output.type = "gadu-gadu";
        break;
    case Disco::gateway_http_ws:
        output.category = "gateway";
        output.type = "http-ws";
        break;
    case Disco::gateway_icq:
        output.category = "gateway";
        output.type = "icq";
        break;
    case Disco::gateway_irc:
        output.category = "gateway";
        output.type = "irc";
        break;
    case Disco::gateway_lcs:
        output.category = "gateway";
        output.type = "lcs";
        break;
    case Disco::gateway_mrim:
        output.category = "gateway";
        output.type = "mrim";
        break;
    case Disco::gateway_msn:
        output.category = "gateway";
        output.type = "msn";
        break;
    case Disco::gateway_myspaceim:
        output.category = "gateway";
        output.type = "myspaceim";
        break;
    case Disco::gateway_ocs:
        output.category = "gateway";
        output.type = "ocs";
        break;
    case Disco::gateway_pstn:
        output.category = "gateway";
        output.type = "pstn";
        break;
    case Disco::gateway_qq:
        output.category = "gateway";
        output.type = "qq";
        break;
    case Disco::gateway_sametime:
        output.category = "gateway";
        output.type = "sometime";
        break;
    case Disco::gateway_simple:
        output.category = "gateway";
        output.type = "simple";
        break;
    case Disco::gateway_skype:
        output.category = "gateway";
        output.type = "skype";
        break;
    case Disco::gateway_sms:
        output.category = "gateway";
        output.type = "sms";
        break;
    case Disco::gateway_smtp:
        output.category = "gateway";
        output.type = "smtp";
        break;
    case Disco::gateway_telegram:
        output.category = "gateway";
        output.type = "telegram";
        break;
    case Disco::gateway_tlen:
        output.category = "gateway";
        output.type = "tlen";
        break;
    case Disco::gateway_xfire:
        output.category = "gateway";
        output.type = "xfire";
        break;
    case Disco::gateway_xmpp:
        output.category = "gateway";
        output.type = "xmpp";
        break;
    case Disco::gateway_yahoo:
        output.category = "gateway";
        output.type = "yahoo";
        break;
    case Disco::headline_newmail:
        output.category = "headline";
        output.type = "newmail";
        break;
    case Disco::headline_rss:
        output.category = "headline";
        output.type = "rss";
        break;
    case Disco::headline_weather:
        output.category = "headline";
        output.type = "weather";
        break;
    case Disco::hierarchy_branch:
        output.category = "hierarchy";
        output.type = "branch";
        break;
    case Disco::hierarchy_leaf:
        output.category = "hierarchy";
        output.type = "leaf";
        break;
    case Disco::proxy_bytestreams:
        output.category = "proxy";
        output.type = "bytestreams";
        break;
    case Disco::pubsub_collection:
        output.category = "pubsub";
        output.type = "collection";
        break;
    case Disco::pubsub_leaf:
        output.category = "pubsub";
        output.type = "leaf";
        break;
    case Disco::pubsub_pep:
        output.category = "pubsub";
        output.type = "pep";
        break;
    case Disco::pubsub_service:
        output.category = "pubsub";
        output.type = "service";
        break;
    case Disco::server_im:
        output.category = "server";
        output.type = "im";
        break;
    case Disco::store_berkeley:
        output.category = "store";
        output.type = "berkeley";
        break;
    case Disco::store_file:
        output.category = "store";
        output.type = "file";
        break;
    case Disco::store_generic:
        output.category = "store";
        output.type = "generic";
        break;
    case Disco::store_ldap:
        output.category = "store";
        output.type = "ldap";
        break;
    case Disco::store_mysql:
        output.category = "store";
        output.type = "mysql";
        break;
    case Disco::store_oracle:
        output.category = "store";
        output.type = "oracle";
        break;
    case Disco::store_postgres:
        output.category = "store";
        output.type = "postgres";
        break;
    }
    return output;
}

Disco::Feature Disco::str2feature(const discofeature_t& feature){
    switch(word2int(feature.var)){
    case IntFromString::ns_xml_xmpp_bind:
        return Feature::bind;
        break;
    case IntFromString::ns_xml_xmpp_e2e:
        return Feature::e2e;
        break;
    case IntFromString::ns_xml_xmpp_sasl:
        return Feature::sasl;
        break;
    case IntFromString::ns_xml_xmpp_sasl_c2s:
        return Feature::sasl_c2s;
        break;
    case IntFromString::ns_xml_xmpp_sasl_s2s:
        return Feature::sasl_s2s;
        break;
    case IntFromString::ns_xml_xmpp_session:
        return Feature::session;
        break;
    case IntFromString::ns_xml_xmpp_stanzas:
        return Feature::stanzas;
        break;
    case IntFromString::ns_xml_xmpp_streams:
        return Feature::streams;
        break;
    case IntFromString::ns_xml_xmpp_tls:
        return Feature::tls;
        break;
    case IntFromString::ns_xml_xmpp_tls_c2s:
        return Feature::tls_c2s;
        break;
    case IntFromString::ns_xml_xmpp_tls_s2s:
        return Feature::tls_s2s;
        break;
    case IntFromString::ns_xmpp_sid_0:
        return Feature::sid;
        break;
    case IntFromString::ns_xmpp_archive_auto:
        return Feature::archive_auto;
        break;
    case IntFromString::ns_xmpp_archive_manage:
        return Feature::archive_manage;
        break;
    case IntFromString::ns_xmpp_archive_manual:
        return Feature::archive_manual;
        break;
    case IntFromString::ns_xmpp_archive_pref:
        return Feature::archive_pref;
        break;
    case IntFromString::ns_xmpp_avatar_data:
        return Feature::avatar_data;
        break;
    case IntFromString::ns_xmpp_avatar_metadata:
        return Feature::avatar_metadata;
        break;
    case IntFromString::ns_xmpp_avatar_metadata_notify:
        return Feature::avatar_metadata_notify;
        break;
    case IntFromString::ns_xmpp_delay:
        return Feature::delay;
        break;
    case IntFromString::ns_xmpp_jingle_apps_rtp_audio:
        return Feature::audio;
        break;
    case IntFromString::ns_xmpp_jingle_apps_rtp_video:
        return Feature::video;
        break;
    case IntFromString::ns_xmpp_ping:
        return Feature::ping;
        break;
    case IntFromString::ns_xmpp_receipts:
        return Feature::receipts;
        break;
    case IntFromString::ns_xmpp_ssn:
        return Feature::ssn;
        break;
    case IntFromString::ns_xmpp_time:
        return Feature::time;
        break;
    case IntFromString::ns_xmpp_styling_0:
        return Feature::styling;
        break;
    case IntFromString::ns_xmpp_caps:
        return Feature::caps2;
        break;
    case IntFromString::ns_xmpp_caps_optimize:
        return Feature::caps2_optimize;
        break;
    case IntFromString::ns_rfc_3264:
        return Feature::rfc3264;
        break;
    case IntFromString::dnssrv:
        return Feature::dnssrv;
        break;
    case IntFromString::fullunicode:
        return Feature::fullunicode;
        break;
    case IntFromString::gc_10:
        return Feature::gc10;
        break;
    case IntFromString::http_jabber_activity:
        return Feature::activity;
        break;
    case IntFromString::http_jabber_address:
        return Feature::address;
        break;
    case IntFromString::http_jabber_amp:
        return Feature::amp;
        break;
    case IntFromString::http_jabber_amp_errors:
        return Feature::amp_errors;
        break;
    case IntFromString::http_jabber_amp_action_alert:
        return Feature::amp_action_alert;
        break;
    case IntFromString::http_jabber_amp_action_drop:
        return Feature::amp_action_drop;
        break;
    case IntFromString::http_jabber_amp_action_error:
        return Feature::amp_action_error;
        break;
    case IntFromString::http_jabber_amp_action_notify:
        return Feature::amp_action_notify;
        break;
    case IntFromString::http_jabber_amp_condition_deliver:
        return Feature::amp_condition_deliver;
        break;
    case IntFromString::http_jabber_amp_condition_expire_at:
        return Feature::amp_condition_expireat;
        break;
    case IntFromString::http_jabber_amp_condition_match_resources:
        return Feature::amp_condition_matchresource;
        break;
    case IntFromString::http_jabber_bytestreams:
        return Feature::bytestreams;
        break;
    case IntFromString::http_jabber_bytestreams_udp:
        return Feature::bytestreams_udp;
        break;
    case IntFromString::http_jabber_caps:
        return Feature::caps;
        break;
    case IntFromString::http_jabber_caps_optimize:
        return Feature::caps_optimize;
        break;
    case IntFromString::http_jabber_chatstates:
        return Feature::chatstates;
        break;
    case IntFromString::http_jabber_commands:
        return Feature::commands;
        break;
    case IntFromString::http_jabber_compress:
        return Feature::compress;
        break;
    case IntFromString::http_jabber_disco_info:
        return Feature::disco_info;
        break;
    case IntFromString::http_jabber_disco_items:
        return Feature::disco_items;
        break;
    case IntFromString::http_jabber_featureneg:
        return Feature::feature_neg;
        break;
    case IntFromString::http_jabber_geoloc:
        return Feature::geoloc;
        break;
    case IntFromString::http_jabber_httpauth:
        return Feature::httpauth;
        break;
    case IntFromString::http_jabber_httpbind:
        return Feature::httpbind;
        break;
    case IntFromString::http_jabber_ibb:
        return Feature::ibb;
        break;
    case IntFromString::http_jabber_mood:
        return Feature::mood;
        break;
    case IntFromString::http_jabber_muc:
        return Feature::muc;
        break;
    case IntFromString::http_jabber_muc_admin:
        return Feature::muc_admin;
        break;
    case IntFromString::http_jabber_muc_owner:
        return Feature::muc_owner;
        break;
    case IntFromString::http_jabber_muc_register:
        return Feature::muc_register;
        break;
    case IntFromString::http_jabber_muc_roomconfig:
        return Feature::muc_roomconfig;
        break;
    case IntFromString::http_jabber_muc_roominfo:
        return Feature::muc_roominfo;
        break;
    case IntFromString::http_jabber_muc_user:
        return Feature::muc_user;
        break;
    case IntFromString::http_jabber_offline:
        return Feature::offline;
        break;
    case IntFromString::http_jabber_pubsub_access_authorize:
        return Feature::pubsub_access_authorize;
        break;
    case IntFromString::http_jabber_pubsub_access_open:
        return Feature::pubsub_access_open;
        break;
    case IntFromString::http_jabber_pubsub_access_presence:
        return Feature::pubsub_access_presence;
        break;
    case IntFromString::http_jabber_pubsub_access_roster:
        return Feature::pubsub_access_roster;
        break;
    case IntFromString::http_jabber_pubsub_access_whitelist:
        return Feature::pubsub_access_whitelist;
        break;
    case IntFromString::http_jabber_pubsub_auto_create:
        return Feature::pubsub_auto_create;
        break;
    case IntFromString::http_jabber_pubsub_auto_subscribe:
        return Feature::pubsub_auto_subscribe;
        break;
    case IntFromString::http_jabber_pubsub_collections:
        return Feature::pubsub_collections;
        break;
    case IntFromString::http_jabber_pubsub_config_node:
        return Feature::pubsub_config_node;
        break;
    case IntFromString::http_jabber_pubsub_create_and_configure:
        return Feature::pubsub_create_and_configure;
        break;
    case IntFromString::http_jabber_pubsub_create_nodes:
        return Feature::pubsub_create_nodes;
        break;
    case IntFromString::http_jabber_pubsub_delete_any:
        return Feature::pubsub_delete_any;
        break;
    case IntFromString::http_jabber_pubsub_delete_nodes:
        return Feature::pubsub_delete_nodes;
        break;
    case IntFromString::http_jabber_pubsub_filtered_notifications:
        return Feature::pubsub_filtered_notifications;
        break;
    case IntFromString::http_jabber_pubsub_get_pending:
        return Feature::pubsub_get_pending;
        break;
    case IntFromString::http_jabber_pubsub_instant_nodes:
        return Feature::pubsub_instant_nodes;
        break;
    case IntFromString::http_jabber_pubsub_item_ids:
        return Feature::pubsub_item_ids;
        break;
    case IntFromString::http_jabber_pubsub_last_published:
        return Feature::pubsub_last_published;
        break;
    case IntFromString::http_jabber_pubsub_leased_subscription:
        return Feature::pubsub_leased_subscription;
        break;
    case IntFromString::http_jabber_pubsub_manage_subscription:
        return Feature::pubsub_manage_subscription;
        break;
    case IntFromString::http_jabber_pubsub_member_affiliation:
        return Feature::pubsub_member_affiliation;
        break;
    case IntFromString::http_jabber_pubsub_meta_data:
        return Feature::pubsub_metadata;
        break;
    case IntFromString::http_jabber_pubsub_modify_affiliations:
        return Feature::pubsub_modify_affiliations;
        break;
    case IntFromString::http_jabber_pubsub_multi_collection:
        return Feature::pubsub_multi_collection;
        break;
    case IntFromString::http_jabber_pubsub_multi_subscribe:
        return Feature::pubsub_multi_subscribe;
        break;
    case IntFromString::http_jabber_pubsub_outcast_affiliation:
        return Feature::pubsub_outcast_affiliation;
        break;
    case IntFromString::http_jabber_pubsub_persistent_items:
        return Feature::pubsub_persistent_items;
        break;
    case IntFromString::http_jabber_pubsub_presence_notifications:
        return Feature::pubsub_presence_notifications;
        break;
    case IntFromString::http_jabber_pubsub_presence_subscribe:
        return Feature::pubsub_presence_subscribe;
        break;
    case IntFromString::http_jabber_pubsub_publish:
        return Feature::pubsub_publish;
        break;
    case IntFromString::http_jabber_pubsub_publish_options:
        return Feature::pubsub_publish_options;
        break;
    case IntFromString::http_jabber_pubsub_publisher_affiliation:
        return Feature::pubsub_publisher_affiliation;
        break;
    case IntFromString::http_jabber_pubsub_purge_nodes:
        return Feature::pubsub_purge_nodes;
        break;
    case IntFromString::http_jabber_pubsub_retract_items:
        return Feature::pubsub_retract_items;
        break;
    case IntFromString::http_jabber_pubsub_retrieve_affiliations:
        return Feature::pubsub_retrieve_affiliations;
        break;
    case IntFromString::http_jabber_pubsub_retrieve_default:
        return Feature::pubsub_retrieve_default;
        break;
    case IntFromString::http_jabber_pubsub_retrieve_items:
        return Feature::pubsub_retrieve_items;
        break;
    case IntFromString::http_jabber_pubsub_retrieve_subscriptions:
        return Feature::pubsub_retrieve_subscriptions;
        break;
    case IntFromString::http_jabber_pubsub_subscribe:
        return Feature::pubsub_subscribe;
        break;
    case IntFromString::http_jabber_pubsub_subscription_options:
        return Feature::pubsub_subscription_options;
        break;
    case IntFromString::http_jabber_pubsub_subscription_notifications:
        return Feature::pubsub_subscription_notifications;
        break;
    case IntFromString::http_jabber_rosterx:
        return Feature::rosterx;
        break;
    case IntFromString::http_jabber_sipub:
        return Feature::sipub;
        break;
    case IntFromString::http_jabber_soap:
        return Feature::soap;
        break;
    case IntFromString::http_jabber_soap_fault:
        return Feature::soap_fault;
        break;
    case IntFromString::http_jabber_waitinglist:
        return Feature::waitinglist;
        break;
    case IntFromString::http_jabber_waitinglist_schemes_mailto:
        return Feature::waitinglist_mailto;
        break;
    case IntFromString::http_jabber_waitinglist_scheme_tel:
        return Feature::waitinglist_tel;
        break;
    case IntFromString::http_jabber_xhtml_im:
        return Feature::xhtml_im;
        break;
    case IntFromString::http_jabber_xdata_layout:
        return Feature::xdata_layout;
        break;
    case IntFromString::http_jabber_xdata_validate:
        return Feature::xdata_validate;
        break;
    case IntFromString::ipv6:
        return Feature::ipv6;
        break;
    case IntFromString::jabber_client:
        return Feature::jabber_client;
        break;
    case IntFromString::jabber_component_accept:
        return Feature::jabber_component_accept;
        break;
    case IntFromString::jabber_component_connect:
        return Feature::jabber_component_connect;
        break;
    case IntFromString::jabber_iq_auth:
        return Feature::jabber_iq_auth;
        break;
    case IntFromString::jabber_iq_gateway:
        return Feature::jabber_iq_gateway;
        break;
    case IntFromString::jabber_iq_last:
        return Feature::jabber_iq_last;
        break;
    case IntFromString::jabber_iq_oob:
        return Feature::jabber_iq_oob;
        break;
    case IntFromString::jabber_iq_privacy:
        return Feature::jabber_iq_privacy;
        break;
    case IntFromString::jabber_iq_private:
        return Feature::jabber_iq_private;
        break;
    case IntFromString::jabber_iq_register:
        return Feature::jabber_iq_register;
        break;
    case IntFromString::jabber_iq_roster:
        return Feature::jabber_iq_roster;
        break;
    case IntFromString::jabber_iq_rpc:
        return Feature::jabber_iq_rpc;
        break;
    case IntFromString::jabber_iq_search:
        return Feature::jabber_iq_search;
        break;
    case IntFromString::jabber_iq_version:
        return Feature::jabber_iq_version;
        break;
    case IntFromString::jabber_server:
        return Feature::jabber_server;
        break;
    case IntFromString::jabber_x_data:
        return Feature::jabber_x_data;
        break;
    case IntFromString::jabber_x_encrypted:
        return Feature::jabber_x_encrypted;
        break;
    case IntFromString::jabber_x_oob:
        return Feature::jabber_x_oob;
        break;
    case IntFromString::jabber_x_signed:
        return Feature::jabber_x_signed;
        break;
    case IntFromString::msglog:
        return Feature::msglog;
        break;
    case IntFromString::msgoffline:
        return Feature::msgoffline;
        break;
    case IntFromString::muc_hidden:
        return Feature::muc_hidden;
        break;
    case IntFromString::muc_membersonly:
        return Feature::muc_membersonly;
        break;
    case IntFromString::muc_moderated:
        return Feature::muc_moderated;
        break;
    case IntFromString::muc_nonanonymous:
        return Feature::muc_nonanonymous;
        break;
    case IntFromString::muc_open:
        return Feature::muc_open;
        break;
    case IntFromString::muc_passwordprotected:
        return Feature::muc_passwordprotected;
        break;
    case IntFromString::muc_persistent:
        return Feature::muc_persistent;
        break;
    case IntFromString::muc_public:
        return Feature::muc_public;
        break;
    case IntFromString::muc_rooms:
        return Feature::muc_rooms;
        break;
    case IntFromString::muc_semianonymous:
        return Feature::muc_semianonymous;
        break;
    case IntFromString::muc_temporary:
        return Feature::muc_temporary;
        break;
    case IntFromString::muc_unmoderated:
        return Feature::muc_unmoderated;
        break;
    case IntFromString::muc_unsecured:
        return Feature::muc_unsecured;
        break;
    case IntFromString::roster_delimiter:
        return Feature::roster_delimiter;
        break;
    case IntFromString::sslc2s:
        return Feature::sslc2s;
        break;
    case IntFromString::stringprep:
        return Feature::stringprep;
        break;
    case IntFromString::xmllang:
        return Feature::xmllang;
        break;
    case IntFromString::vcard_temp:
        return Feature::vcard_temp;
        break;
    default:
        break;
    }
}

discofeature_t Disco::feature2str(Feature feature){
    discofeature_t output;
    switch(feature){
    case Feature::bind:
        output.var = int2word(IntFromString::ns_xml_xmpp_bind);
        break;
    case Feature::e2e:
        output.var = int2word(IntFromString::ns_xml_xmpp_e2e);
        break;
    case Feature::sasl:
        output.var = int2word(IntFromString::ns_xml_xmpp_sasl);
        break;
    case Feature::sasl_c2s:
        output.var = int2word(IntFromString::ns_xml_xmpp_sasl_c2s);
        break;
    case Feature::sasl_s2s:
        output.var = int2word(IntFromString::ns_xml_xmpp_sasl_s2s);
        break;
    case Feature::session:
        output.var = int2word(IntFromString::ns_xml_xmpp_session);
        break;
    case Feature::stanzas:
        output.var = int2word(IntFromString::ns_xml_xmpp_stanzas);
        break;
    case Feature::streams:
        output.var = int2word(IntFromString::ns_xml_xmpp_streams);
        break;
    case Feature::tls:
        output.var = int2word(IntFromString::ns_xml_xmpp_tls);
        break;
    case Feature::tls_c2s:
        output.var = int2word(IntFromString::ns_xml_xmpp_tls_c2s);
        break;
    case Feature::tls_s2s:
        output.var = int2word(IntFromString::ns_xml_xmpp_tls_s2s);
        break;
    case Feature::sid:
        output.var = int2word(IntFromString::ns_xmpp_sid_0);
        break;
    case Feature::archive_auto:
        output.var = int2word(IntFromString::ns_xmpp_archive_auto);
        break;
    case Feature::archive_manage:
        output.var = int2word(IntFromString::ns_xmpp_archive_manage);
        break;
    case Feature::archive_manual:
        output.var = int2word(IntFromString::ns_xmpp_archive_manual);
        break;
    case Feature::archive_pref:
        output.var = int2word(IntFromString::ns_xmpp_archive_pref);
        break;
    case Feature::avatar_data:
        output.var = int2word(IntFromString::ns_xmpp_avatar_data);
        break;
    case Feature::avatar_metadata:
        output.var = int2word(IntFromString::ns_xmpp_avatar_metadata);
        break;
    case Feature::avatar_metadata_notify:
        output.var = int2word(IntFromString::ns_xmpp_avatar_metadata_notify);
        break;
    case Feature::delay:
        output.var = int2word(IntFromString::ns_xmpp_delay);
        break;
    case Feature::audio:
        output.var = int2word(IntFromString::ns_xmpp_jingle_apps_rtp_audio);
        break;
    case Feature::video:
        output.var = int2word(IntFromString::ns_xmpp_jingle_apps_rtp_video);
        break;
    case Feature::ping:
        output.var = int2word(IntFromString::ns_xmpp_ping);
        break;
    case Feature::receipts:
        output.var = int2word(IntFromString::ns_xmpp_receipts);
        break;
    case Feature::ssn:
        output.var = int2word(IntFromString::ns_xmpp_ssn);
        break;
    case Feature::time:
        output.var = int2word(IntFromString::ns_xmpp_time);
        break;
    case Feature::styling:
        output.var = int2word(IntFromString::ns_xmpp_styling_0);
        break;
    case Feature::caps2:
        output.var = int2word(IntFromString::ns_xmpp_caps);
        break;
    case Feature::caps2_optimize:
        output.var = int2word(IntFromString::ns_xmpp_caps_optimize);
        break;
    case Feature::rfc3264:
        output.var = int2word(IntFromString::ns_rfc_3264);
        break;
    case Feature::dnssrv:
        output.var = int2word(IntFromString::dnssrv);
        break;
    case Feature::fullunicode:
        output.var = int2word(IntFromString::fullunicode);
        break;
    case Feature::gc10:
        output.var = int2word(IntFromString::gc_10);
        break;
    case Feature::activity:
        output.var = int2word(IntFromString::http_jabber_activity);
        break;
    case Feature::address:
        output.var = int2word(IntFromString::http_jabber_address);
        break;
    case Feature::amp:
        output.var = int2word(IntFromString::http_jabber_amp);
        break;
    case Feature::amp_errors:
        output.var = int2word(IntFromString::http_jabber_amp_errors);
        break;
    case Feature::amp_action_alert:
        output.var = int2word(IntFromString::http_jabber_amp_action_alert);
        break;
    case Feature::amp_action_drop:
        output.var = int2word(IntFromString::http_jabber_amp_action_drop);
        break;
    case Feature::amp_action_error:
        output.var = int2word(IntFromString::http_jabber_amp_action_error);
        break;
    case Feature::amp_action_notify:
        output.var = int2word(IntFromString::http_jabber_amp_action_notify);
        break;
    case Feature::amp_condition_deliver:
        output.var = int2word(IntFromString::http_jabber_amp_condition_deliver);
        break;
    case Feature::amp_condition_expireat:
        output.var = int2word(IntFromString::http_jabber_amp_condition_expire_at);
        break;
    case Feature::amp_condition_matchresource:
        output.var = int2word(IntFromString::http_jabber_amp_condition_match_resources);
        break;
    case Feature::bytestreams:
        output.var = int2word(IntFromString::http_jabber_bytestreams);
        break;
    case Feature::bytestreams_udp:
        output.var = int2word(IntFromString::http_jabber_bytestreams_udp);
        break;
    case Feature::caps:
        output.var = int2word(IntFromString::http_jabber_caps);
        break;
    case Feature::caps_optimize:
        output.var = int2word(IntFromString::http_jabber_caps_optimize);
        break;
    case Feature::chatstates:
        output.var = int2word(IntFromString::http_jabber_chatstates);
        break;
    case Feature::commands:
        output.var = int2word(IntFromString::http_jabber_commands);
        break;
    case Feature::compress:
        output.var = int2word(IntFromString::http_jabber_compress);
        break;
    case Feature::disco_info:
        output.var = int2word(IntFromString::http_jabber_disco_info);
        break;
    case Feature::disco_items:
        output.var = int2word(IntFromString::http_jabber_disco_items);
        break;
    case Feature::feature_neg:
        output.var = int2word(IntFromString::http_jabber_featureneg);
        break;
    case Feature::geoloc:
        output.var = int2word(IntFromString::http_jabber_geoloc);
        break;
    case Feature::httpauth:
        output.var = int2word(IntFromString::http_jabber_httpauth);
        break;
    case Feature::httpbind:
        output.var = int2word(IntFromString::http_jabber_httpbind);
        break;
    case Feature::ibb:
        output.var = int2word(IntFromString::http_jabber_ibb);
        break;
    case Feature::mood:
        output.var = int2word(IntFromString::http_jabber_mood);
        break;
    case Feature::muc:
        output.var = int2word(IntFromString::http_jabber_muc);
        break;
    case Feature::muc_admin:
        output.var = int2word(IntFromString::http_jabber_muc_admin);
        break;
    case Feature::muc_owner:
        output.var = int2word(IntFromString::http_jabber_muc_owner);
        break;
    case Feature::muc_register:
        output.var = int2word(IntFromString::http_jabber_muc_register);
        break;
    case Feature::muc_roomconfig:
        output.var = int2word(IntFromString::http_jabber_muc_roomconfig);
        break;
    case Feature::muc_roominfo:
        output.var = int2word(IntFromString::http_jabber_muc_roominfo);
        break;
    case Feature::muc_user:
        output.var = int2word(IntFromString::http_jabber_muc_user);
        break;
    case Feature::offline:
        output.var = int2word(IntFromString::http_jabber_offline);
        break;
    case Feature::pubsub_access_authorize:
        output.var = int2word(IntFromString::http_jabber_pubsub_access_authorize);
        break;
    case Feature::pubsub_access_open:
        output.var = int2word(IntFromString::http_jabber_pubsub_access_open);
        break;
    case Feature::pubsub_access_presence:
        output.var = int2word(IntFromString::http_jabber_pubsub_access_presence);
        break;
    case Feature::pubsub_access_roster:
        output.var = int2word(IntFromString::http_jabber_pubsub_access_roster);
        break;
    case Feature::pubsub_access_whitelist:
        output.var = int2word(IntFromString::http_jabber_pubsub_access_whitelist);
        break;
    case Feature::pubsub_auto_create:
        output.var = int2word(IntFromString::http_jabber_pubsub_auto_create);
        break;
    case Feature::pubsub_auto_subscribe:
        output.var = int2word(IntFromString::http_jabber_pubsub_auto_subscribe);
        break;
    case Feature::pubsub_collections:
        output.var = int2word(IntFromString::http_jabber_pubsub_collections);
        break;
    case Feature::pubsub_config_node:
        output.var = int2word(IntFromString::http_jabber_pubsub_config_node);
        break;
    case Feature::pubsub_create_and_configure:
        output.var = int2word(IntFromString::http_jabber_pubsub_create_and_configure);
        break;
    case Feature::pubsub_create_nodes:
        output.var = int2word(IntFromString::http_jabber_pubsub_create_nodes);
        break;
    case Feature::pubsub_delete_any:
        output.var = int2word(IntFromString::http_jabber_pubsub_delete_any);
        break;
    case Feature::pubsub_delete_nodes:
        output.var = int2word(IntFromString::http_jabber_pubsub_delete_nodes);
        break;
    case Feature::pubsub_filtered_notifications:
        output.var = int2word(IntFromString::http_jabber_pubsub_filtered_notifications);
        break;
    case Feature::pubsub_get_pending:
        output.var = int2word(IntFromString::http_jabber_pubsub_get_pending);
        break;
    case Feature::pubsub_instant_nodes:
        output.var = int2word(IntFromString::http_jabber_pubsub_instant_nodes);
        break;
    case Feature::pubsub_item_ids:
        output.var = int2word(IntFromString::http_jabber_pubsub_item_ids);
        break;
    case Feature::pubsub_last_published:
        output.var = int2word(IntFromString::http_jabber_pubsub_last_published);
        break;
    case Feature::pubsub_leased_subscription:
        output.var = int2word(IntFromString::http_jabber_pubsub_leased_subscription);
        break;
    case Feature::pubsub_manage_subscription:
        output.var = int2word(IntFromString::http_jabber_pubsub_manage_subscription);
        break;
    case Feature::pubsub_member_affiliation:
        output.var = int2word(IntFromString::http_jabber_pubsub_member_affiliation);
        break;
    case Feature::pubsub_metadata:
        output.var = int2word(IntFromString::http_jabber_pubsub_meta_data);
        break;
    case Feature::pubsub_modify_affiliations:
        output.var = int2word(IntFromString::http_jabber_pubsub_modify_affiliations);
        break;
    case Feature::pubsub_multi_collection:
        output.var = int2word(IntFromString::http_jabber_pubsub_multi_collection);
        break;
    case Feature::pubsub_multi_subscribe:
        output.var = int2word(IntFromString::http_jabber_pubsub_multi_subscribe);
        break;
    case Feature::pubsub_outcast_affiliation:
        output.var = int2word(IntFromString::http_jabber_pubsub_outcast_affiliation);
        break;
    case Feature::pubsub_persistent_items:
        output.var = int2word(IntFromString::http_jabber_pubsub_persistent_items);
        break;
    case Feature::pubsub_presence_notifications:
        output.var = int2word(IntFromString::http_jabber_pubsub_presence_notifications);
        break;
    case Feature::pubsub_presence_subscribe:
        output.var = int2word(IntFromString::http_jabber_pubsub_presence_subscribe);
        break;
    case Feature::pubsub_publish:
        output.var = int2word(IntFromString::http_jabber_pubsub_publish);
        break;
    case Feature::pubsub_publish_options:
        output.var = int2word(IntFromString::http_jabber_pubsub_publish_options);
        break;
    case Feature::pubsub_publisher_affiliation:
        output.var = int2word(IntFromString::http_jabber_pubsub_publisher_affiliation);
        break;
    case Feature::pubsub_purge_nodes:
        output.var = int2word(IntFromString::http_jabber_pubsub_purge_nodes);
        break;
    case Feature::pubsub_retract_items:
        output.var = int2word(IntFromString::http_jabber_pubsub_retract_items);
        break;
    case Feature::pubsub_retrieve_affiliations:
        output.var = int2word(IntFromString::http_jabber_pubsub_retrieve_affiliations);
        break;
    case Feature::pubsub_retrieve_default:
        output.var = int2word(IntFromString::http_jabber_pubsub_retrieve_default);
        break;
    case Feature::pubsub_retrieve_items:
        output.var = int2word(IntFromString::http_jabber_pubsub_retrieve_items);
        break;
    case Feature::pubsub_retrieve_subscriptions:
        output.var = int2word(IntFromString::http_jabber_pubsub_retrieve_subscriptions);
        break;
    case Feature::pubsub_subscribe:
        output.var = int2word(IntFromString::http_jabber_pubsub_subscribe);
        break;
    case Feature::pubsub_subscription_options:
        output.var = int2word(IntFromString::http_jabber_pubsub_subscription_options);
        break;
    case Feature::pubsub_subscription_notifications:
        output.var = int2word(IntFromString::http_jabber_pubsub_subscription_notifications);
        break;
    case Feature::rosterx:
        output.var = int2word(IntFromString::http_jabber_rosterx);
        break;
    case Feature::sipub:
        output.var = int2word(IntFromString::http_jabber_sipub);
        break;
    case Feature::soap:
        output.var = int2word(IntFromString::http_jabber_soap);
        break;
    case Feature::soap_fault:
        output.var = int2word(IntFromString::http_jabber_soap_fault);
        break;
    case Feature::waitinglist:
        output.var = int2word(IntFromString::http_jabber_waitinglist);
        break;
    case Feature::waitinglist_mailto:
        output.var = int2word(IntFromString::http_jabber_waitinglist_schemes_mailto);
        break;
    case Feature::waitinglist_tel:
        output.var = int2word(IntFromString::http_jabber_waitinglist_scheme_tel);
        break;
    case Feature::xhtml_im:
        output.var = int2word(IntFromString::http_jabber_xhtml_im);
        break;
    case Feature::xdata_layout:
        output.var = int2word(IntFromString::http_jabber_xdata_layout);
        break;
    case Feature::xdata_validate:
        output.var = int2word(IntFromString::http_jabber_xdata_validate);
        break;
    case Feature::ipv6:
        output.var = int2word(IntFromString::ipv6);
        break;
    case Feature::jabber_client:
        output.var = int2word(IntFromString::jabber_client);
        break;
    case Feature::jabber_component_accept:
        output.var = int2word(IntFromString::jabber_component_accept);
        break;
    case Feature::jabber_component_connect:
        output.var = int2word(IntFromString::jabber_component_connect);
        break;
    case Feature::jabber_iq_auth:
        output.var = int2word(IntFromString::jabber_iq_auth);
        break;
    case Feature::jabber_iq_gateway:
        output.var = int2word(IntFromString::jabber_iq_gateway);
        break;
    case Feature::jabber_iq_last:
        output.var = int2word(IntFromString::jabber_iq_last);
        break;
    case Feature::jabber_iq_oob:
        output.var = int2word(IntFromString::jabber_iq_oob);
        break;
    case Feature::jabber_iq_privacy:
        output.var = int2word(IntFromString::jabber_iq_privacy);
        break;
    case Feature::jabber_iq_private:
        output.var = int2word(IntFromString::jabber_iq_private);
        break;
    case Feature::jabber_iq_register:
        output.var = int2word(IntFromString::jabber_iq_register);
        break;
    case Feature::jabber_iq_roster:
        output.var = int2word(IntFromString::jabber_iq_roster);
        break;
    case Feature::jabber_iq_rpc:
        output.var = int2word(IntFromString::jabber_iq_rpc);
        break;
    case Feature::jabber_iq_search:
        output.var = int2word(IntFromString::jabber_iq_search);
        break;
    case Feature::jabber_iq_version:
        output.var = int2word(IntFromString::jabber_iq_version);
        break;
    case Feature::jabber_server:
        output.var = int2word(IntFromString::jabber_server);
        break;
    case Feature::jabber_x_data:
        output.var = int2word(IntFromString::jabber_x_data);
        break;
    case Feature::jabber_x_encrypted:
        output.var = int2word(IntFromString::jabber_x_encrypted);
        break;
    case Feature::jabber_x_oob:
        output.var = int2word(IntFromString::jabber_x_oob);
        break;
    case Feature::jabber_x_signed:
        output.var = int2word(IntFromString::jabber_x_signed);
        break;
    case Feature::msglog:
        output.var = int2word(IntFromString::msglog);
        break;
    case Feature::msgoffline:
        output.var = int2word(IntFromString::msgoffline);
        break;
    case Feature::muc_hidden:
        output.var = int2word(IntFromString::muc_hidden);
        break;
    case Feature::muc_membersonly:
        output.var = int2word(IntFromString::muc_membersonly);
        break;
    case Feature::muc_moderated:
        output.var = int2word(IntFromString::muc_moderated);
        break;
    case Feature::muc_nonanonymous:
        output.var = int2word(IntFromString::muc_nonanonymous);
        break;
    case Feature::muc_open:
        output.var = int2word(IntFromString::muc_open);
        break;
    case Feature::muc_passwordprotected:
        output.var = int2word(IntFromString::muc_passwordprotected);
        break;
    case Feature::muc_persistent:
        output.var = int2word(IntFromString::muc_persistent);
        break;
    case Feature::muc_public:
        output.var = int2word(IntFromString::muc_public);
        break;
    case Feature::muc_rooms:
        output.var = int2word(IntFromString::muc_rooms);
        break;
    case Feature::muc_semianonymous:
        output.var = int2word(IntFromString::muc_semianonymous);
        break;
    case Feature::muc_temporary:
        output.var = int2word(IntFromString::muc_temporary);
        break;
    case Feature::muc_unmoderated:
        output.var = int2word(IntFromString::muc_unmoderated);
        break;
    case Feature::muc_unsecured:
        output.var = int2word(IntFromString::muc_unsecured);
        break;
    case Feature::roster_delimiter:
        output.var = int2word(IntFromString::roster_delimiter);
        break;
    case Feature::sslc2s:
        output.var = int2word(IntFromString::sslc2s);
        break;
    case Feature::stringprep:
        output.var = int2word(IntFromString::stringprep);
        break;
    case Feature::xmllang:
        output.var = int2word(IntFromString::xmllang);
        break;
    case Feature::vcard_temp:
        output.var = int2word(IntFromString::vcard_temp);
        break;
    default:
        break;
    }
    return output;
}
