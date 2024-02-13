/*
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "incidencetreemodel.h"

#include <Akonadi/Item>

#include <KCalendarCore/Incidence>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QPersistentModelIndex>
#include <QSharedPointer>
#include <QStringList>

namespace Akonadi
{
using Uid = QString;
using ParentUid = QString;

struct Node {
    using Ptr = QSharedPointer<Node>;
    using Map = QMap<Akonadi::Item::Id, Ptr>;
    using List = QList<Ptr>;

    QPersistentModelIndex sourceIndex; // because ETM::modelIndexesForItem is so slow
    Akonadi::Item::Id id;
    Node::Ptr parentNode;
    QString parentUid;
    QString uid;
    List directChilds;
    int depth;
};

/** Just a struct to contain some data before we create the node */
struct PreNode {
    using Ptr = QSharedPointer<PreNode>;
    using List = QList<Ptr>;
    KCalendarCore::Incidence::Ptr incidence;
    QPersistentModelIndex sourceIndex;
    Akonadi::Item item;
    int depth;
    PreNode()
        : depth(-1)
    {
    }
};

class IncidenceTreeModelPrivate : public QObject
{
    Q_OBJECT
public:
    IncidenceTreeModelPrivate(IncidenceTreeModel *qq, const QStringList &mimeTypes);
    void reset(bool silent = false);
    void insertNode(const PreNode::Ptr &node, bool silent = false);
    void insertNode(const QModelIndex &sourceIndex, bool silent = false);
    void removeNode(const Node::Ptr &node);
    QModelIndex indexForNode(const Node::Ptr &node) const;
    int rowForNode(const Node::Ptr &node) const;
    bool indexBeingRemoved(const QModelIndex &) const; // Is it being removed?
    void dumpTree();
    void assert_and_dump(bool condition, const QString &message);
    Node::List sorted(const Node::List &nodes) const;
    PreNode::Ptr prenodeFromSourceRow(int sourceRow) const;
    void setSourceModel(QAbstractItemModel *model);

public:
    Node::Map m_nodeMap;
    Node::List m_toplevelNodeList;
    QHash<Uid, Node::Ptr> m_uidMap;
    QHash<Uid, Akonadi::Item> m_itemByUid;
    QMultiHash<ParentUid, Node::Ptr> m_waitingForParent;
    QList<Node *> m_removedNodes;
    const QStringList m_mimeTypes;

private Q_SLOTS:
    void onHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    void onDataChanged(const QModelIndex &begin, const QModelIndex &end);

    void onRowsAboutToBeInserted(const QModelIndex &parent, int begin, int end);
    void onRowsInserted(const QModelIndex &parent, int begin, int end);
    void onRowsAboutToBeRemoved(const QModelIndex &parent, int begin, int end);
    void onRowsRemoved(const QModelIndex &parent, int begin, int end);
    void onRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);

    void onModelAboutToBeReset();
    void onModelReset();
    void onLayoutAboutToBeChanged();
    void onLayoutChanged();

private:
    IncidenceTreeModel *const q;
};
}
