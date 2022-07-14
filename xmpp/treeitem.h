#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVector>

template <class T>
class TreeItem
{
public:
    enum class Type{
        ROOT,
        SPACER,
        DATA
    };

    TreeItem(Type type, const T& data, TreeItem<T>* parentItem = nullptr);
    TreeItem(Type type, T&& data, TreeItem<T>* parentItem = nullptr);
    ~TreeItem();

    void appendChild(TreeItem<T>* child);

    TreeItem<T> *child(int row);
    TreeItem<T> *parent();
    int childCount() const;
    const T& data() const;
    T* dataPtr();
    void setData(const T& data);
    int row() const;
    Type type() const;
    bool isExpanded() const;
    void setExpanded(bool value);

private:
    QVector<TreeItem*> _childItems;
    T _itemData;
    TreeItem<T> *_parentItem;
    const Type _type;
    bool m_isExpanded = false;
};

template <class T>
TreeItem<T>::TreeItem(Type type, const T& data, TreeItem<T>* parentItem)
    : _itemData(data), _parentItem(parentItem), _type(type)
{}

template <class T>
TreeItem<T>::TreeItem(Type type, T&& data, TreeItem<T>* parentItem)
    : _itemData(std::move(data)), _parentItem(parentItem), _type(type)
{}

template <class T>
TreeItem<T>::~TreeItem(){
    qDeleteAll(_childItems);
}

template <class T>
void TreeItem<T>::appendChild(TreeItem<T>* item){
    _childItems.append(item);
}

template <class T>
TreeItem<T>* TreeItem<T>::child(int row){
    if(row < 0 || row >= _childItems.size()){
        return nullptr;
    }
    return _childItems.at(row);
}

template <class T>
int TreeItem<T>::childCount() const{
    return _childItems.count();
}

template <class T>
int TreeItem<T>::row() const{
    if(_parentItem){
        return _parentItem->_childItems.indexOf(const_cast<TreeItem<T>*>(this));
    }
    return 0;
}

template <class T>
const T& TreeItem<T>::data() const{
    return _itemData;
}

template <class T>
T* TreeItem<T>::dataPtr(){
    return &_itemData;
}

template <class T>
void TreeItem<T>::setData(const T& data){
    _itemData = data;
}

template <class T>
TreeItem<T>* TreeItem<T>::parent(){
    return _parentItem;
}

template <class T>
typename TreeItem<T>::Type TreeItem<T>::type() const{
    return _type;
}

template<class T>
bool TreeItem<T>::isExpanded() const
{
    return m_isExpanded;
}

template<class T>
void TreeItem<T>::setExpanded(bool value)
{
    m_isExpanded = value;
}

#endif // TREEITEM_H
