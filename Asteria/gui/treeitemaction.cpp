#include "treeitemaction.h"

TreeItemAction::TreeItemAction(const QString & text, TreeItem * item, QObject *parent) : QAction(text, parent) {
    treeItem = item;
    connect(this, SIGNAL (triggered()), this, SLOT (clicked()));
}

void TreeItemAction::clicked() {
    emit itemClicked(treeItem);
}
