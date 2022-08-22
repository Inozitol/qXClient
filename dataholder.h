#ifndef DATAHOLDER_H
#define DATAHOLDER_H

#include <unordered_map>
#include <QString>
#include <QIcon>

class DataHolder
{
public:

    const QIcon& getIcon(const QString& name);
    const QImage& getAvatar(const QString& id);
    void insertAvatar(const QImage& img, const QString& id);

    static DataHolder& instance();

private:
    DataHolder();
    DataHolder(const DataHolder&) = delete;
    DataHolder& operator=(const DataHolder&) = delete;

    std::unordered_map<QString, QIcon>  m_umapIconReference;
    std::unordered_map<QString, QImage> m_umapAvatarReference;
    QImage m_nullImg;
    QIcon  m_defIcon;
};

#endif // DATAHOLDER_H
