#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QIcon>

/**
 * @brief The TreeItem class
 *
 * Follows the example at http://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
 */
class TreeItem
{
public:
    explicit TreeItem(const QList<QVariant> &data, TreeItem *parentItem=0);
    ~TreeItem();

    void appendChild(TreeItem *item);

    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem *parentItem();
    QIcon getIcon();
    void setIcon(QIcon iconToUse);

private:
    // Child items (links to subdirectories, video clips)
    QList<TreeItem*> m_childItems;
    // The data held by this item (nothing, if it's a directory)
    QList<QVariant> m_itemData;
    // The parent item (e.g. the directory containing the clip/subdirectory represented by this TreeItem)
    TreeItem *m_parentItem;
    // Icon displayed next to the item in the tree viewer
    QIcon icon;
};

#endif // TREEITEM_H
