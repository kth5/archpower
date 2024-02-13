/*
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akonadi-calendar_export.h"
#include <Akonadi/Item>

#include <QAbstractProxyModel>

#include <memory>

namespace Akonadi
{

class IncidenceTreeModelPrivate;

/** Hierarchical incidence model.
 *  Useful as a source for TodoModel for example.
 */
class AKONADI_CALENDAR_EXPORT IncidenceTreeModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    /**
     * Constructs a new IncidenceTreeModel.
     */
    explicit IncidenceTreeModel(QObject *parent = nullptr);

    /**
     * Constructs a new IncidenceTreeModel which will only show incidences of
     * type @p mimeTypes. Common use case is a to-do tree.
     *
     * This constructor is offered for performance reasons. The filtering has
     * zero overhead, and we avoid stacking mime type filter proxy models.
     *
     * If you're more concerned about clean design than performance, use the default
     * constructor and stack a Akonadi::EntityMimeTypeFilterModel on top of this one.
     */
    explicit IncidenceTreeModel(const QStringList &mimeTypes, QObject *parent = nullptr);

    ~IncidenceTreeModel() override;

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    [[nodiscard]] QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    [[nodiscard]] QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    [[nodiscard]] QModelIndex parent(const QModelIndex &child) const override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    [[nodiscard]] bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Returns the akonadi item containing the incidence with @p incidenceUid.
     */
    [[nodiscard]] Akonadi::Item item(const QString &incidenceUid) const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever an index changes parent.
     * The view can then expand the parent if desired.
     * This is better than the view waiting for "rows moved" signals because those
     * signals are also sent when the model is initially populated.
     */
    void indexChangedParent(const QModelIndex &index);

    /**
     * Signals that we finished doing a batch of insertions.
     *
     * One rowsInserted() signal from the ETM, will make IncidenceTreeModel generate
     * several rowsInserted(), layoutChanged() or rowsMoved() signals.
     *
     * A tree view can use this signal to know when to call KConfigViewStateSaver::restore()
     * to restore expansion states. Listening to rowsInserted() signals would be a
     * performance problem.
     */
    void batchInsertionFinished();

private:
    friend class IncidenceTreeModelPrivate;
    std::unique_ptr<IncidenceTreeModelPrivate> const d;
};

}
