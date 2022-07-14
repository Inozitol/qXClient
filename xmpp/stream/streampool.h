#ifndef STREAMPOOL_H
#define STREAMPOOL_H

#include "stream.h"

class StreamPool : public QObject
{
    Q_OBJECT
public:
    Stream* newStream(const Account& account, const Server& server);
    Stream* getStream(const jidbare_t& jid) const;
    Stream* lastStream() const;

    static StreamPool& instance();

private:
    StreamPool() = default;
    StreamPool(const StreamPool&) = delete;
    StreamPool& operator=(const StreamPool&) = delete;

    ~StreamPool();

    std::unordered_map<jidbare_t, Stream*> m_umapStreams;
    Stream* m_ptrLastStream = nullptr;

signals:
    void disconnectAll();
};

#endif // STREAMPOOL_H
