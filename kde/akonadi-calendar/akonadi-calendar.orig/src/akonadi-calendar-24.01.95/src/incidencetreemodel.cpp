/*
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akonadicalendar_debug.h"
#include "incidencetreemodel_p.h"

#include <Akonadi/EntityTreeModel>

using namespace Akonadi;
QDebug operator<<(QDebug s, const Node::Ptr &node);

static void calculateDepth(const Node::Ptr &node)
{
    Q_ASSERT(node);
    node->depth = node->parentNode ? 1 + node->parentNode->depth : 0;
    for (const Node::Ptr &child : std::as_const(node->directChilds)) {
        calculateDepth(child);
    }
}

// Desired ordering [...],3,2,1,0,-1
static bool reverseDepthLessThan(const Node::Ptr &node1, const Node::Ptr &node2)
{
    return node1->depth > node2->depth;
}

// Desired ordering 0,1,2,3,[...],-1
static bool depthLessThan(const PreNode::Ptr &node1, const PreNode::Ptr &node2)
{
    if (node1->depth == -1) {
        return false;
    }
    return node1->depth < node2->depth || node2->depth == -1;
}

static PreNode::List sortedPrenodes(const PreNode::List &nodes)
{
    const int count = nodes.count();
    QHash<QString, PreNode::Ptr> prenodeByUid;
    PreNode::List remainingNodes = nodes;

    while (prenodeByUid.count() < count) {
        const auto preSize = prenodeByUid.count(); // this saves us from infinite looping if the parent doesn't exist
        for (const PreNode::Ptr &node : nodes) {
            Q_ASSERT(node);
            const QString uid = node->incidence->instanceIdentifier();
            const QString parentUid = node->incidence->relatedTo();
            if (parentUid.isEmpty()) { // toplevel todo
                prenodeByUid.insert(uid, node);
                remainingNodes.removeAll(node);
                node->depth = 0;
            } else {
                if (prenodeByUid.contains(parentUid)) {
                    node->depth = 1 + prenodeByUid.value(parentUid)->depth;
                    remainingNodes.removeAll(node);
                    prenodeByUid.insert(uid, node);
                }
            }
        }

        if (preSize == prenodeByUid.count()) {
            break;
        }
    }

    PreNode::List sorted = nodes;
    std::sort(sorted.begin(), sorted.end(), depthLessThan);
    return sorted;
}

IncidenceTreeModelPrivate::IncidenceTreeModelPrivate(IncidenceTreeModel *qq, const QStringList &mimeTypes)
    : QObject()
    , m_mimeTypes(mimeTypes)
    , q(qq)
{
}

int IncidenceTreeModelPrivate::rowForNode(const Node::Ptr &node) const
{
    // Returns it's row number
    const int row = node->parentNode ? node->parentNode->directChilds.indexOf(node) : m_toplevelNodeList.indexOf(node);
    Q_ASSERT(row != -1);
    return row;
}

void IncidenceTreeModelPrivate::assert_and_dump(bool condition, const QString &message)
{
    if (!condition) {
        qCCritical(AKONADICALENDAR_LOG) << "This should never happen: " << message;
        dumpTree();
        Q_ASSERT(false);
    }
}

void IncidenceTreeModelPrivate::dumpTree()
{
    for (const Node::Ptr &node : std::as_const(m_toplevelNodeList)) {
        qCDebug(AKONADICALENDAR_LOG) << node;
    }
}

QModelIndex IncidenceTreeModelPrivate::indexForNode(const Node::Ptr &node) const
{
    if (!node) {
        return {};
    }
    const int row = node->parentNode ? node->parentNode->directChilds.indexOf(node) : m_toplevelNodeList.indexOf(node);

    Q_ASSERT(row != -1);
    return q->createIndex(row, 0, node.data());
}

void IncidenceTreeModelPrivate::reset(bool silent)
{
    if (!silent) {
        q->beginResetModel();
    }
    m_toplevelNodeList.clear();
    m_nodeMap.clear();
    m_itemByUid.clear();
    m_waitingForParent.clear();
    m_uidMap.clear();
    if (q->sourceModel()) {
        const int sourceCount = q->sourceModel()->rowCount();
        for (int i = 0; i < sourceCount; ++i) {
            PreNode::Ptr prenode = prenodeFromSourceRow(i);
            if (prenode && (m_mimeTypes.isEmpty() || m_mimeTypes.contains(prenode->incidence->mimeType()))) {
                insertNode(prenode, /**silent=*/true);
            }
        }
    }
    if (!silent) {
        q->endResetModel();
    }
}

void IncidenceTreeModelPrivate::onHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_EMIT q->headerDataChanged(orientation, first, last);
}

void IncidenceTreeModelPrivate::onDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    Q_ASSERT(begin.isValid());
    Q_ASSERT(end.isValid());
    Q_ASSERT(q->sourceModel());
    Q_ASSERT(!begin.parent().isValid());
    Q_ASSERT(!end.parent().isValid());
    Q_ASSERT(begin.row() <= end.row());
    const int first_row = begin.row();
    const int last_row = end.row();

    for (int i = first_row; i <= last_row; ++i) {
        QModelIndex sourceIndex = q->sourceModel()->index(i, 0);
        Q_ASSERT(sourceIndex.isValid());
        QModelIndex index = q->mapFromSource(sourceIndex);
        // Index might be invalid if we filter by incidence type.
        if (index.isValid()) {
            Q_ASSERT(index.internalPointer());

            // Did we this node change parent? If no, just Q_EMIT dataChanged(), if
            // yes, we must Q_EMIT rowsMoved(), so we see a visual effect in the view.
            Node *rawNode = reinterpret_cast<Node *>(index.internalPointer());
            Node::Ptr node = m_uidMap.value(rawNode->uid); // Looks hackish but it's safe
            Q_ASSERT(node);
            Node::Ptr oldParentNode = node->parentNode;
            auto item = q->data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
            Q_ASSERT(item.isValid());
            KCalendarCore::Incidence::Ptr incidence =
                !item.hasPayload<KCalendarCore::Incidence::Ptr>() ? KCalendarCore::Incidence::Ptr() : item.payload<KCalendarCore::Incidence::Ptr>();
            if (!incidence) {
                qCCritical(AKONADICALENDAR_LOG) << "Incidence shouldn't be invalid." << item.hasPayload() << item.id();
                Q_ASSERT(false);
                return;
            }

            // An UID could have changed, update hashes!
            if (node->uid != incidence->instanceIdentifier()) {
                qCDebug(AKONADICALENDAR_LOG) << "Incidence UID has changed" << node->uid << incidence->instanceIdentifier();
                m_itemByUid.remove(node->uid);
                m_uidMap.remove(node->uid);
                node->uid = incidence->instanceIdentifier();
                m_uidMap.insert(node->uid, node);
            }
            m_itemByUid.insert(incidence->instanceIdentifier(), item);

            Node::Ptr newParentNode;
            const QString newParentUid = incidence->relatedTo();
            if (!newParentUid.isEmpty()) {
                Q_ASSERT(m_uidMap.contains(newParentUid));
                newParentNode = m_uidMap.value(newParentUid);
                Q_ASSERT(newParentNode);
            }

            const bool parentChanged = newParentNode.data() != oldParentNode.data();

            if (parentChanged) {
                const int fromRow = rowForNode(node);
                int toRow = -1;
                QModelIndex newParentIndex;

                // Calculate parameters for beginMoveRows()
                if (newParentNode) {
                    newParentIndex = q->mapFromSource(newParentNode->sourceIndex);
                    Q_ASSERT(newParentIndex.isValid());
                    toRow = newParentNode->directChilds.count();
                } else {
                    // New parent is 0, it's son of root now
                    newParentIndex = QModelIndex();
                    toRow = m_toplevelNodeList.count();
                }

                const bool res = q->beginMoveRows(/**fromParent*/ index.parent(), fromRow, fromRow, newParentIndex, toRow);
                Q_ASSERT(res);
                Q_UNUSED(res)

                // Now that beginmoveRows() was called, we can do the actual moving:
                if (newParentNode) {
                    newParentNode->directChilds.append(node); // Add to new parent
                    node->parentNode = newParentNode;

                    if (oldParentNode) {
                        oldParentNode->directChilds.remove(fromRow); // Remove from parent
                        Q_ASSERT(oldParentNode->directChilds.indexOf(node) == -1);
                    } else {
                        m_toplevelNodeList.remove(fromRow); // Remove from root
                        Q_ASSERT(m_toplevelNodeList.indexOf(node) == -1);
                    }
                } else {
                    // New parent is 0, it's son of root now
                    m_toplevelNodeList.append(node);
                    node->parentNode = Node::Ptr();
                    oldParentNode->directChilds.remove(fromRow);
                    Q_ASSERT(oldParentNode->directChilds.indexOf(node) == -1);
                }

                q->endMoveRows();

                // index is rotten after the move, retrieve it again
                index = indexForNode(node);
                Q_ASSERT(index.isValid());

                if (newParentNode) {
                    Q_EMIT q->indexChangedParent(index.parent());
                }
            } else {
                Q_EMIT q->dataChanged(index, index);
            }
        }
    }
}

void IncidenceTreeModelPrivate::onRowsAboutToBeInserted(const QModelIndex &parent, int, int)
{
    // We are a reparenting proxy, the source proxy is flat
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent)
    // Nothing to do yet. We don't know if all the new incidences in this range belong to the same
    // parent yet.
}

PreNode::Ptr IncidenceTreeModelPrivate::prenodeFromSourceRow(int row) const
{
    PreNode::Ptr node = PreNode::Ptr(new PreNode());
    node->sourceIndex = q->sourceModel()->index(row, 0, QModelIndex());
    Q_ASSERT(node->sourceIndex.isValid());
    Q_ASSERT(node->sourceIndex.model() == q->sourceModel());
    const auto item = node->sourceIndex.data(EntityTreeModel::ItemRole).value<Akonadi::Item>();

    if (!item.isValid()) {
        // It's a Collection, ignore that, we only want items.
        return {};
    }

    node->item = item;
    node->incidence = item.payload<KCalendarCore::Incidence::Ptr>();
    Q_ASSERT(node->incidence);

    return node;
}

void IncidenceTreeModelPrivate::onRowsInserted(const QModelIndex &parent, int begin, int end)
{
    // QElapsedTimer timer;
    // timer.start();
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent)
    Q_ASSERT(begin <= end);
    PreNode::List nodes;
    for (int i = begin; i <= end; ++i) {
        PreNode::Ptr node = prenodeFromSourceRow(i);
        // if m_mimeTypes is empty, we ignore this feature
        if (!node || (!m_mimeTypes.isEmpty() && !m_mimeTypes.contains(node->incidence->mimeType()))) {
            continue;
        }
        nodes << node;
    }

    const PreNode::List sortedNodes = sortedPrenodes(nodes);

    for (const PreNode::Ptr &node : sortedNodes) {
        insertNode(node);
    }

    // view can now call KConfigViewStateSaver::restoreState(), to expand nodes.
    if (end > begin) {
        Q_EMIT q->batchInsertionFinished();
    }
    // qCDebug(AKONADICALENDAR_LOG) << "Took " << timer.elapsed() << " to insert " << end-begin+1;
}

void IncidenceTreeModelPrivate::insertNode(const PreNode::Ptr &prenode, bool silent)
{
    KCalendarCore::Incidence::Ptr incidence = prenode->incidence;
    Akonadi::Item item = prenode->item;
    Node::Ptr node(new Node());
    node->sourceIndex = prenode->sourceIndex;
    node->id = item.id();
    node->uid = incidence->instanceIdentifier();
    m_itemByUid.insert(node->uid, item);
    // qCDebug(AKONADICALENDAR_LOG) << "New node " << node.data() << node->uid << node->id;
    node->parentUid = incidence->relatedTo();
    if (node->uid == node->parentUid) {
        qCWarning(AKONADICALENDAR_LOG) << "Incidence with itself as parent!" << node->uid << "Akonadi item" << item.id() << "remoteId=" << item.remoteId();
        node->parentUid.clear();
    }

    if (m_uidMap.contains(node->uid)) {
        qCWarning(AKONADICALENDAR_LOG) << "Duplicate incidence detected:"
                                       << "uid=" << node->uid << ". File a bug against the resource. collection=" << item.storageCollectionId();
        return;
    }

    Q_ASSERT(!m_nodeMap.contains(node->id));
    m_uidMap.insert(node->uid, node);
    m_nodeMap.insert(item.id(), node);

    int rowToUse = -1;
    bool mustInsertIntoParent = false;

    const bool hasParent = !node->parentUid.isEmpty();
    if (hasParent) {
        // We have a parent, did he arrive yet ?
        if (m_uidMap.contains(node->parentUid)) {
            node->parentNode = m_uidMap.value(node->parentUid);

            // We can only insert after beginInsertRows(), because it affects rowCounts
            mustInsertIntoParent = true;
            rowToUse = node->parentNode->directChilds.count();
        } else {
            // Parent unknown, we are orphan for now
            Q_ASSERT(!m_waitingForParent.contains(node->parentUid, node));
            m_waitingForParent.insert(node->parentUid, node);
        }
    }

    if (!node->parentNode) {
        rowToUse = m_toplevelNodeList.count();
    }

    // Lets insert the row:
    const QModelIndex &parent = indexForNode(node->parentNode);
    if (!silent) {
        q->beginInsertRows(parent, rowToUse, rowToUse);
    }

    if (!node->parentNode) {
        m_toplevelNodeList.append(node);
    }

    if (mustInsertIntoParent) {
        node->parentNode->directChilds.append(node);
    }

    if (!silent) {
        q->endInsertRows();
    }

    // Are we a parent?
    if (m_waitingForParent.contains(node->uid)) {
        Q_ASSERT(m_waitingForParent.count(node->uid) > 0);
        const QList<Node::Ptr> children = m_waitingForParent.values(node->uid);
        m_waitingForParent.remove(node->uid);
        Q_ASSERT(!children.isEmpty());

        for (const Node::Ptr &child : children) {
            const int fromRow = m_toplevelNodeList.indexOf(child);
            Q_ASSERT(fromRow != -1);
            const QModelIndex toParent = indexForNode(node);
            Q_ASSERT(toParent.isValid());
            Q_ASSERT(toParent.model() == q);
            // const int toRow = node->directChilds.count();

            if (!silent) {
                // const bool res = q->beginMoveRows( /**fromParent*/QModelIndex(), fromRow,
                //                                 fromRow, toParent, toRow );
                // Q_EMIT q->layoutAboutToBeChanged();
                q->beginResetModel();
                // Q_ASSERT( res );
            }
            child->parentNode = node;
            node->directChilds.append(child);
            m_toplevelNodeList.remove(fromRow);

            if (!silent) {
                // q->endMoveRows();
                q->endResetModel();
                // Q_EMIT q->layoutChanged();
            }
        }
    }
}

// Sorts children first parents last
Node::List IncidenceTreeModelPrivate::sorted(const Node::List &nodes) const
{
    if (nodes.isEmpty()) {
        return nodes;
    }

    // Initialize depths
    for (const Node::Ptr &topLevelNode : std::as_const(m_toplevelNodeList)) {
        calculateDepth(topLevelNode);
    }

    Node::List sorted = nodes;
    std::sort(sorted.begin(), sorted.end(), reverseDepthLessThan);

    return sorted;
}

void IncidenceTreeModelPrivate::onRowsAboutToBeRemoved(const QModelIndex &parent, int begin, int end)
{
    // QElapsedTimer timer;
    // timer.start();
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent)
    Q_ASSERT(begin <= end);

    // First, gather nodes to remove
    Node::List nodesToRemove;
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceIndex = q->sourceModel()->index(i, 0, QModelIndex());
        Q_ASSERT(sourceIndex.isValid());
        Q_ASSERT(sourceIndex.model() == q->sourceModel());
        const Akonadi::Item::Id id = sourceIndex.data(EntityTreeModel::ItemIdRole).toLongLong();
        Q_ASSERT(id != -1);
        if (!m_nodeMap.contains(id)) {
            // We don't know about this one because we're ignoring it's mime type.
            Q_ASSERT(m_mimeTypes.count() != 3);
            continue;
        }
        Node::Ptr node = m_nodeMap.value(id);
        Q_ASSERT(node->id == id);
        nodesToRemove << node;
    }

    // We want to remove children first, to avoid row moving
    const Node::List nodesToRemoveSorted = sorted(nodesToRemove);

    for (const Node::Ptr &node : nodesToRemoveSorted) {
        // Go ahead and remove it now. We don't do it in ::onRowsRemoved(), because
        // while unparenting children with moveRows() the view might call data() on the
        // item that is already removed from ETM.
        removeNode(node);
        // qCDebug(AKONADICALENDAR_LOG) << "Just removed a node, here's the tree";
        // dumpTree();
    }

    m_removedNodes.clear();
    // qCDebug(AKONADICALENDAR_LOG) << "Took " << timer.elapsed() << " to remove " << end-begin+1;
}

void IncidenceTreeModelPrivate::removeNode(const Node::Ptr &node)
{
    Q_ASSERT(node);
    // qCDebug(AKONADICALENDAR_LOG) << "Dealing with parent: " << node->id << node.data()
    //         << node->uid << node->directChilds.count() << indexForNode( node );

    // First, unparent the children
    if (!node->directChilds.isEmpty()) {
        const Node::List children = node->directChilds;
        const QModelIndex fromParent = indexForNode(node);
        Q_ASSERT(fromParent.isValid());
        //    const int firstSourceRow = 0;
        //  const int lastSourceRow  = node->directChilds.count() - 1;
        // const int toRow = m_toplevelNodeList.count();
        // q->beginMoveRows( fromParent, firstSourceRow, lastSourceRow,
        //                  /**toParent is root*/QModelIndex(), toRow );
        q->beginResetModel();
        node->directChilds.clear();
        for (const Node::Ptr &child : children) {
            // qCDebug(AKONADICALENDAR_LOG) << "Dealing with child: " << child.data() << child->uid;
            m_toplevelNodeList.append(child);
            child->parentNode = Node::Ptr();
            m_waitingForParent.insert(node->uid, child);
        }
        // q->endMoveRows();
        q->endResetModel();
    }

    const QModelIndex parent = indexForNode(node->parentNode);

    const int rowToRemove = rowForNode(node);

    // Now remove the row
    Q_ASSERT(!(parent.isValid() && parent.model() != q));
    q->beginRemoveRows(parent, rowToRemove, rowToRemove);
    m_itemByUid.remove(node->uid);

    if (parent.isValid()) {
        node->parentNode->directChilds.remove(rowToRemove);
        node->parentNode = Node::Ptr();
    } else {
        m_toplevelNodeList.remove(rowToRemove);
    }

    if (!node->parentUid.isEmpty()) {
        m_waitingForParent.remove(node->parentUid, node);
    }

    m_uidMap.remove(node->uid);
    m_nodeMap.remove(node->id);

    q->endRemoveRows();
    m_removedNodes << node.data();
}

void IncidenceTreeModelPrivate::onRowsRemoved(const QModelIndex &parent, int begin, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(begin)
    Q_UNUSED(end)
    // Nothing to do here, see comment on ::onRowsAboutToBeRemoved()
}

void IncidenceTreeModelPrivate::onModelAboutToBeReset()
{
    q->beginResetModel();
}

void IncidenceTreeModelPrivate::onModelReset()
{
    reset(/**silent=*/false);
    q->endResetModel();
}

void IncidenceTreeModelPrivate::onLayoutAboutToBeChanged()
{
    Q_ASSERT(q->persistentIndexList().isEmpty());
    Q_EMIT q->layoutAboutToBeChanged();
}

void IncidenceTreeModelPrivate::onLayoutChanged()
{
    reset(/**silent=*/true);
    Q_ASSERT(q->persistentIndexList().isEmpty());
    Q_EMIT q->layoutChanged();
}

void IncidenceTreeModelPrivate::onRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)
{
    // Not implemented yet
    Q_ASSERT(false);
}

void IncidenceTreeModelPrivate::setSourceModel(QAbstractItemModel *model)
{
    q->beginResetModel();

    if (q->sourceModel()) {
        disconnect(q->sourceModel(), &IncidenceTreeModel::dataChanged, this, &IncidenceTreeModelPrivate::onDataChanged);

        disconnect(q->sourceModel(), &IncidenceTreeModel::headerDataChanged, this, &IncidenceTreeModelPrivate::onHeaderDataChanged);

        disconnect(q->sourceModel(), &IncidenceTreeModel::rowsInserted, this, &IncidenceTreeModelPrivate::onRowsInserted);

        disconnect(q->sourceModel(), &IncidenceTreeModel::rowsRemoved, this, &IncidenceTreeModelPrivate::onRowsRemoved);

        disconnect(q->sourceModel(), &IncidenceTreeModel::rowsMoved, this, &IncidenceTreeModelPrivate::onRowsMoved);

        disconnect(q->sourceModel(), &IncidenceTreeModel::rowsAboutToBeInserted, this, &IncidenceTreeModelPrivate::onRowsAboutToBeInserted);

        disconnect(q->sourceModel(), &IncidenceTreeModel::rowsAboutToBeRemoved, this, &IncidenceTreeModelPrivate::onRowsAboutToBeRemoved);

        disconnect(q->sourceModel(), &IncidenceTreeModel::modelAboutToBeReset, this, &IncidenceTreeModelPrivate::onModelAboutToBeReset);

        disconnect(q->sourceModel(), &IncidenceTreeModel::modelReset, this, &IncidenceTreeModelPrivate::onModelReset);

        disconnect(q->sourceModel(), &IncidenceTreeModel::layoutAboutToBeChanged, this, &IncidenceTreeModelPrivate::onLayoutAboutToBeChanged);

        disconnect(q->sourceModel(), &IncidenceTreeModel::layoutChanged, this, &IncidenceTreeModelPrivate::onLayoutChanged);
    }

    q->QAbstractProxyModel::setSourceModel(model);

    if (q->sourceModel()) {
        connect(q->sourceModel(), &IncidenceTreeModel::dataChanged, this, &IncidenceTreeModelPrivate::onDataChanged);

        connect(q->sourceModel(), &IncidenceTreeModel::headerDataChanged, this, &IncidenceTreeModelPrivate::onHeaderDataChanged);

        connect(q->sourceModel(), &IncidenceTreeModel::rowsAboutToBeInserted, this, &IncidenceTreeModelPrivate::onRowsAboutToBeInserted);

        connect(q->sourceModel(), &IncidenceTreeModel::rowsInserted, this, &IncidenceTreeModelPrivate::onRowsInserted);

        connect(q->sourceModel(), &IncidenceTreeModel::rowsAboutToBeRemoved, this, &IncidenceTreeModelPrivate::onRowsAboutToBeRemoved);

        connect(q->sourceModel(), &IncidenceTreeModel::rowsRemoved, this, &IncidenceTreeModelPrivate::onRowsRemoved);

        connect(q->sourceModel(), &IncidenceTreeModel::rowsMoved, this, &IncidenceTreeModelPrivate::onRowsMoved);

        connect(q->sourceModel(), &IncidenceTreeModel::modelAboutToBeReset, this, &IncidenceTreeModelPrivate::onModelAboutToBeReset);

        connect(q->sourceModel(), &IncidenceTreeModel::modelReset, this, &IncidenceTreeModelPrivate::onModelReset);

        connect(q->sourceModel(), &IncidenceTreeModel::layoutAboutToBeChanged, this, &IncidenceTreeModelPrivate::onLayoutAboutToBeChanged);

        connect(q->sourceModel(), &IncidenceTreeModel::layoutChanged, this, &IncidenceTreeModelPrivate::onLayoutChanged);
    }

    reset(/**silent=*/true);
    q->endResetModel();
}

IncidenceTreeModel::IncidenceTreeModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , d(new IncidenceTreeModelPrivate(this, QStringList()))
{
    setObjectName(QLatin1StringView("IncidenceTreeModel"));
}

IncidenceTreeModel::IncidenceTreeModel(const QStringList &mimeTypes, QObject *parent)
    : QAbstractProxyModel(parent)
    , d(new IncidenceTreeModelPrivate(this, mimeTypes))
{
    setObjectName(QLatin1StringView("IncidenceTreeModel"));
}

IncidenceTreeModel::~IncidenceTreeModel() = default;

QVariant IncidenceTreeModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());
    if (!index.isValid() || !sourceModel()) {
        return {};
    }

    QModelIndex sourceIndex = mapToSource(index);
    Q_ASSERT(sourceIndex.isValid());

    return sourceModel()->data(sourceIndex, role);
}

int IncidenceTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Q_ASSERT(parent.model() == this);
        Node *parentNode = reinterpret_cast<Node *>(parent.internalPointer());
        Q_ASSERT(parentNode);
        d->assert_and_dump(!d->m_removedNodes.contains(parentNode), QString::number((quintptr)parentNode, 16) + QLatin1String(" was already deleted"));

        const int count = parentNode->directChilds.count();
        return count;
    }

    return d->m_toplevelNodeList.count();
}

int IncidenceTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Q_ASSERT(parent.model() == this);
    }
    return sourceModel() ? sourceModel()->columnCount() : 1;
}

void IncidenceTreeModel::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }
    d->setSourceModel(model);
}

QModelIndex IncidenceTreeModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        qCWarning(AKONADICALENDAR_LOG) << "IncidenceTreeModel::mapFromSource() source index is invalid";
        // Q_ASSERT( false );
        return {};
    }

    if (!sourceModel()) {
        return {};
    }
    Q_ASSERT(sourceIndex.column() < sourceModel()->columnCount());
    Q_ASSERT(sourceModel() == sourceIndex.model());
    const Akonadi::Item::Id id = sourceIndex.data(Akonadi::EntityTreeModel::ItemIdRole).toLongLong();

    if (id == -1 || !d->m_nodeMap.contains(id)) {
        return {};
    }

    const Node::Ptr node = d->m_nodeMap.value(id);
    Q_ASSERT(node);

    return d->indexForNode(node);
}

QModelIndex IncidenceTreeModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid() || !sourceModel()) {
        return {};
    }

    Q_ASSERT(proxyIndex.column() < columnCount());
    Q_ASSERT(proxyIndex.internalPointer());
    Q_ASSERT(proxyIndex.model() == this);
    Node *node = reinterpret_cast<Node *>(proxyIndex.internalPointer());

    /*
     This code is slow, using a persistent model index instead.
    QModelIndexList indexes = EntityTreeModel::modelIndexesForItem( sourceModel(), Akonadi::Item( node->id ) );
    if ( indexes.isEmpty() ) {
      Q_ASSERT( sourceModel() );
      qCCritical(AKONADICALENDAR_LOG) << "IncidenceTreeModel::mapToSource() no indexes."
               << proxyIndex << node->id << "; source.rowCount() = "
               << sourceModel()->rowCount() << "; source=" << sourceModel()
               << "rowCount()" << rowCount();
      Q_ASSERT( false );
      return QModelIndex();
    }
    QModelIndex index = indexes.first();*/
    QModelIndex index = node->sourceIndex;
    if (!index.isValid()) {
        qCWarning(AKONADICALENDAR_LOG) << "IncidenceTreeModel::mapToSource(): sourceModelIndex is invalid";
        Q_ASSERT(false);
        return {};
    }
    Q_ASSERT(index.model() == sourceModel());

    return index.sibling(index.row(), proxyIndex.column());
}

QModelIndex IncidenceTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        qCWarning(AKONADICALENDAR_LOG) << "IncidenceTreeModel::parent(): child is invalid";
        Q_ASSERT(false);
        return {};
    }

    Q_ASSERT(child.model() == this);
    Q_ASSERT(child.internalPointer());
    Node *childNode = reinterpret_cast<Node *>(child.internalPointer());
    if (d->m_removedNodes.contains(childNode)) {
        qCWarning(AKONADICALENDAR_LOG) << "IncidenceTreeModel::parent() Node already removed.";
        return {};
    }

    if (!childNode->parentNode) {
        return {};
    }

    const QModelIndex parentIndex = d->indexForNode(childNode->parentNode);

    if (!parentIndex.isValid()) {
        qCWarning(AKONADICALENDAR_LOG) << "IncidenceTreeModel::parent(): proxyModelIndex is invalid.";
        Q_ASSERT(false);
        return {};
    }

    Q_ASSERT(parentIndex.model() == this);
    Q_ASSERT(childNode->parentNode.data());

    // Parent is always at row 0
    return parentIndex;
}

QModelIndex IncidenceTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= rowCount(parent)) {
        // This is ok apparently
        /*qCWarning(AKONADICALENDAR_LOG) << "IncidenceTreeModel::index() parent.isValid()" << parent.isValid()
                   << "; row=" << row << "; column=" << column
                   << "; rowCount() = " << rowCount( parent ); */
        // Q_ASSERT( false );
        return {};
    }

    Q_ASSERT(column >= 0);
    Q_ASSERT(column < columnCount());

    if (parent.isValid()) {
        Q_ASSERT(parent.model() == this);
        Q_ASSERT(parent.internalPointer());
        Node *parentNode = reinterpret_cast<Node *>(parent.internalPointer());

        if (row >= parentNode->directChilds.count()) {
            qCCritical(AKONADICALENDAR_LOG) << "IncidenceTreeModel::index() row=" << row << "; column=" << column;
            Q_ASSERT(false);
            return {};
        }

        return createIndex(row, column, parentNode->directChilds.at(row).data());
    } else {
        Q_ASSERT(row < d->m_toplevelNodeList.count());
        Node::Ptr node = d->m_toplevelNodeList.at(row);
        Q_ASSERT(node);
        return createIndex(row, column, node.data());
    }
}

bool IncidenceTreeModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Q_ASSERT(parent.column() < columnCount());
        if (parent.column() != 0) {
            // Indexes at column >0 don't have parent, says Qt documentation
            return false;
        }
        Node *parentNode = reinterpret_cast<Node *>(parent.internalPointer());
        Q_ASSERT(parentNode);
        return !parentNode->directChilds.isEmpty();
    } else {
        return !d->m_toplevelNodeList.isEmpty();
    }
}

Akonadi::Item IncidenceTreeModel::item(const QString &uid) const
{
    Akonadi::Item item;
    if (uid.isEmpty()) {
        qCWarning(AKONADICALENDAR_LOG) << "Called with an empty uid";
    } else {
        if (d->m_itemByUid.contains(uid)) {
            item = d->m_itemByUid.value(uid);
        } else {
            qCWarning(AKONADICALENDAR_LOG) << "There's no incidence with uid " << uid;
        }
    }

    return item;
}

QDebug operator<<(QDebug s, const Node::Ptr &node)
{
    Q_ASSERT(node);
    static int level = 0;
    ++level;
    QString padding = QString(level - 1, QLatin1Char(' '));
    s << padding + QLatin1String("node") << node.data() << QStringLiteral(";uid=") << node->uid << QStringLiteral(";id=") << node->id
      << QStringLiteral(";parentUid=") << node->parentUid << QStringLiteral(";parentNode=") << (void *)(node->parentNode.data()) << '\n';

    for (const Node::Ptr &child : std::as_const(node->directChilds)) {
        s << child;
    }

    --level;
    return s;
}

#include "moc_incidencetreemodel_p.cpp"

#include "moc_incidencetreemodel.cpp"
