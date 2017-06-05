#ifndef TREEITEMACTION_H
#define TREEITEMACTION_H

#include "gui/treeitem.h"

#include <QAction>

class TreeItemAction : public QAction
{
    Q_OBJECT

public:
    explicit TreeItemAction(const QString & text, TreeItem *item, QObject *parent = 0);

signals:
    void itemClicked(TreeItem *);

public slots:

private slots:
     void clicked();

private:
     TreeItem * treeItem;

};

#endif // TREEITEMACTION_H
