#include "streampool.h"

StreamPool& StreamPool::instance(){
    static StreamPool instance;
    return instance;
}

Stream* StreamPool::newStream(const Account& account, const Server& server){
    jidbare_t jid = account.jid();
    QThread* thread = new QThread();
    Stream* stream = new Stream(account, server);
    stream->moveToThread(thread);
    connect(this,   &StreamPool::disconnectAll,
            stream, &Stream::initDisconnect);
    connect(thread, &QThread::started,
            stream, &Stream::connectInsecure);
    connect(stream, &Stream::disconnected,
            thread, &QThread::quit);
    connect(stream, &Stream::disconnected,
            stream, &Stream::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    thread->start();
    m_umapStreams.insert({jid, stream});
    m_ptrLastStream = stream;
    return stream;
}

Stream* StreamPool::getStream(const jidbare_t& jid) const{
    Stream* stream;
    try{
        stream = m_umapStreams.at(jid);
    }catch(const std::out_of_range&){
        stream = nullptr;
    }
    return stream;
}

StreamPool::~StreamPool(){
    for(auto it = m_umapStreams.cbegin(); it != m_umapStreams.cend(); ){
        QThread* thread = it->second->thread();
        delete(it->second);
        delete(thread);
    }
}

Stream *StreamPool::lastStream() const{
    return m_ptrLastStream;
}
