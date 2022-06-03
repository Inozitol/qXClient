#ifndef STREAMPOOL_H
#define STREAMPOOL_H

#include "stream.h"

class StreamPool : public QObject
{
    Q_OBJECT
public:
    Stream* newStream(std::shared_ptr<Account> acc, std::shared_ptr<Server> srv);
    Stream* getStream(const jidbare_t& jid) const;
    Stream* getLast() const;

    static StreamPool& instance();


private:
    StreamPool() = default;
    StreamPool(const StreamPool&) = delete;
    StreamPool& operator=(const StreamPool&) = delete;

    ~StreamPool();

    QMap<jidbare_t, Stream*> _streams;
    Stream* _last_inserted = nullptr;
};

#endif // STREAMPOOL_H
