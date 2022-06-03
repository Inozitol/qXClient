#include "streampool.h"

StreamPool& StreamPool::instance(){
    static StreamPool instance;
    return instance;
}

Stream* StreamPool::newStream(std::shared_ptr<Account> acc, std::shared_ptr<Server> srv){
    jidbare_t jid = acc->jid();
    QThread* thread = new QThread();
    Stream* stream = new Stream(acc, srv);
    stream->moveToThread(thread);
    connect(thread, &QThread::started,
            stream, &Stream::connectInsecure);
    connect(stream, &Stream::finished,
            thread, &QThread::quit);
    connect(stream, &Stream::finished,
            stream, &Stream::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    thread->start();
    _streams.insert(jid, stream);
    _last_inserted = stream;
    return stream;
}

Stream* StreamPool::getStream(const jidbare_t& jid) const{
    return _streams.value(jid);
}

StreamPool::~StreamPool(){
    for(auto it = _streams.cbegin(); it != _streams.cend(); ){
        QThread* thread = it.value()->thread();
        delete(it.value());
        delete(thread);
    }
}

Stream *StreamPool::getLast() const{
    return _last_inserted;
}
