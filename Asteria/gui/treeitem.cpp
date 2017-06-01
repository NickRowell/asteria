#include "treeitem.h"

TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parentItem) {
    m_parentItem = parentItem;
    m_itemData = data;
}

TreeItem::~TreeItem() {
    qDeleteAll(m_childItems);
}

void TreeItem::appendChild(TreeItem *item) {
    m_childItems.append(item);
}

TreeItem *TreeItem::child(unsigned int row) {
    return m_childItems.value(row);
}

int TreeItem::childCount() const {
    return m_childItems.count();
}

int TreeItem::row() const {
    if(m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
    }
    // Is the root item
    return 0;
}

int TreeItem::columnCount() const {
    return m_itemData.count();
}

QVariant TreeItem::data(unsigned int column) const {
    return m_itemData.value(column);
}

TreeItem *TreeItem::parentItem() {
    return m_parentItem;
}

QIcon TreeItem::getIcon() {
    return icon;
}

void TreeItem::setIcon(QIcon iconToUse) {
    icon = iconToUse;
}
