/*
  SPDX-FileCopyrightText: 2008 Thomas Thrainer <tom_t@gmx.at>
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "todomodel.h"

#include <KCalendarCore/Attachment>
#include <KCalendarCore/Event>
#include <KEmailAddress>
#include <KLocalizedString>

#include <Akonadi/CalendarUtils>
#include <Akonadi/IncidenceTreeModel>
#include <Akonadi/TagCache>

#include <KCalUtils/DndFactory>
#include <KCalUtils/ICalDrag>
#include <KCalUtils/IncidenceFormatter>
#include <KCalUtils/VCalDrag>

#include "akonadicalendar_debug.h"

#include <QIcon>
#include <QMimeData>


namespace Akonadi
{
class TodoModelPrivate
{
public:
    TodoModelPrivate(TodoModel *qq);

    Akonadi::EntityTreeModel *etm()
    {
        if (!mEtm) {
            auto *model = q->sourceModel();
            while (model) {
                if (auto *proxy = qobject_cast<QAbstractProxyModel *>(model); proxy) {
                    model = proxy->sourceModel();
                } else if (auto *etm = qobject_cast<Akonadi::EntityTreeModel *>(model); etm) {
                    mEtm = etm;
                    break;
                } else {
                    return nullptr;
                }
            }
        }

        return mEtm;
    }

    // TODO: O(N) complexity, see if the profiler complains about this
    Akonadi::Item findItemByUid(const QString &uid, const QModelIndex &parent) const;

    Akonadi::IncidenceChanger *m_changer = nullptr;

    void onDataChanged(const QModelIndex &begin, const QModelIndex &end);

    TodoModel *const q;
    Akonadi::EntityTreeModel *mEtm = nullptr;
};
}

using namespace Akonadi;

TodoModelPrivate::TodoModelPrivate(TodoModel *qq)
    : q(qq)
{
}

Akonadi::Item TodoModelPrivate::findItemByUid(const QString &uid, const QModelIndex &parent) const
{
    Q_ASSERT(!uid.isEmpty());
    auto treeModel = qobject_cast<Akonadi::IncidenceTreeModel *>(q->sourceModel());
    if (treeModel) { // O(1) Shortcut
        return treeModel->item(uid);
    }

    Akonadi::Item item;
    const int count = q->rowCount(parent);
    for (int i = 0; i < count; ++i) {
        const QModelIndex currentIndex = q->index(i, 0, parent);
        Q_ASSERT(currentIndex.isValid());
        item = q->data(currentIndex, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        if (item.isValid()) {
            return item;
        } else {
            item = findItemByUid(uid, currentIndex);
            if (item.isValid()) {
                return item;
            }
        }
    }

    return item;
}

void TodoModelPrivate::onDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    Q_ASSERT(begin.isValid());
    Q_ASSERT(end.isValid());
    const QModelIndex proxyBegin = q->mapFromSource(begin);
    Q_ASSERT(proxyBegin.column() == 0);
    const QModelIndex proxyEnd = q->mapFromSource(end);
    Q_EMIT q->dataChanged(proxyBegin, proxyEnd.sibling(proxyEnd.row(), TodoModel::ColumnCount - 1));
}

TodoModel::TodoModel(QObject *parent)
    : KExtraColumnsProxyModel(parent)
    , d(new TodoModelPrivate(this))
{
    setObjectName(QLatin1StringView("TodoModel"));
}

TodoModel::~TodoModel() = default;

QVariant TodoModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.isValid());
    if (!index.isValid()) {
        return {};
    }

    const QModelIndex sourceIndex = mapToSource(index.sibling(index.row(), 0));
    if (!sourceIndex.isValid()) {
        return {};
    }
    Q_ASSERT(sourceIndex.isValid());
    const auto item = sourceIndex.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid()) {
        qCWarning(AKONADICALENDAR_LOG) << "Invalid index: " << sourceIndex;
        // Q_ASSERT( false );
        return {};
    }
    const KCalendarCore::Todo::Ptr todo = Akonadi::CalendarUtils::todo(item);
    if (!todo) {
        qCCritical(AKONADICALENDAR_LOG) << "item.hasPayload()" << item.hasPayload();
        if (item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
            auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();
            if (incidence) {
                qCCritical(AKONADICALENDAR_LOG) << "It's actually " << incidence->type();
            }
        }

        Q_ASSERT(!"There's no to-do.");
        return {};
    }

    switch (role) {
    case SummaryRole:
        return todo->summary();
    case RecurRole:
        if (todo->recurs()) {
            if (todo->hasRecurrenceId()) {
                return i18nc("yes, an exception to a recurring to-do", "Exception");
            } else {
                return i18nc("yes, recurring to-do", "Yes");
            }
        } else {
            return i18nc("no, not a recurring to-do", "No");
        }
    case PriorityRole:
        if (todo->priority() == 0) {
            return QStringLiteral("--");
        }
        return todo->priority();
    case PercentRole:
        return todo->percentComplete();
    case StartDateRole:
        return todo->hasStartDate() ? QLocale().toString(todo->dtStart().toLocalTime().date(), QLocale::ShortFormat) : QString();
    case DueDateRole:
        return todo->hasDueDate() ? QLocale().toString(todo->dtDue().toLocalTime().date(), QLocale::ShortFormat) : QString();
    case CategoriesRole:
        return todo->categories().join(i18nc("delimiter for joining category/tag names", ","));
    case DescriptionRole:
        return todo->description();
    case CalendarRole:
        return Akonadi::CalendarUtils::displayName(d->etm(), item.parentCollection());
    default:
        break; // column based model handling
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case SummaryColumn:
            return QVariant(todo->summary());
        case RecurColumn:
            if (todo->recurs()) {
                if (todo->hasRecurrenceId()) {
                    return i18nc("yes, an exception to a recurring to-do", "Exception");
                } else {
                    return i18nc("yes, recurring to-do", "Yes");
                }
            } else {
                return i18nc("no, not a recurring to-do", "No");
            }
        case PriorityColumn:
            if (todo->priority() == 0) {
                return QVariant(QStringLiteral("--"));
            }
            return {todo->priority()};
        case PercentColumn:
            return {todo->percentComplete()};
        case StartDateColumn:
            return todo->hasStartDate() ? QLocale().toString(todo->dtStart().toLocalTime().date(), QLocale::ShortFormat) : QVariant(QString());
        case DueDateColumn:
            return todo->hasDueDate() ? QLocale().toString(todo->dtDue().toLocalTime().date(), QLocale::ShortFormat) : QVariant(QString());
        case CompletedDateColumn:
            return todo->hasCompletedDate() ? QLocale().toString(todo->completed().toLocalTime().date(), QLocale::ShortFormat) : QVariant(QString());
        case CategoriesColumn: {
            QString categories = todo->categories().join(i18nc("delimiter for joining category/tag names", ","));
            return QVariant(categories);
        }
        case DescriptionColumn:
            return QVariant(todo->description());
        case CalendarColumn:
            return QVariant(Akonadi::CalendarUtils::displayName(d->etm(), item.parentCollection()));
        }
        return {};
    }

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case SummaryColumn:
            return QVariant(todo->summary());
        case RecurColumn:
            return {todo->recurs()};
        case PriorityColumn:
            return {todo->priority()};
        case PercentColumn:
            return {todo->percentComplete()};
        case StartDateColumn:
            return QVariant(todo->dtStart().date());
        case DueDateColumn:
            return QVariant(todo->dtDue().date());
        case CompletedDateColumn:
            return QVariant(todo->completed().date());
        case CategoriesColumn:
            return QVariant(todo->categories());
        case DescriptionColumn:
            return QVariant(todo->description());
        case CalendarColumn:
            return QVariant(Akonadi::CalendarUtils::displayName(d->etm(), item.parentCollection()));
        }
        return {};
    }

    // indicate if a row is checked (=completed) only in the first column
    if (role == Qt::CheckStateRole && index.column() == 0) {
        if (hasChildren(index) && !index.parent().isValid()) {
            return {};
        }

        if (todo->isCompleted()) {
            return {Qt::Checked};
        } else {
            return {Qt::Unchecked};
        }
    }

    // icon for recurring todos
    // It's in the summary column so you don't accidentally click
    // the checkbox ( which increments the next occurrence date ).
    // category colour
    if (role == Qt::DecorationRole && index.column() == SummaryColumn) {
        if (todo->recurs()) {
            return QVariant(QIcon::fromTheme(QStringLiteral("task-recurring")));
        }
        const QStringList categories = todo->categories();
        return categories.isEmpty() ? QVariant() : QVariant(Akonadi::TagCache::instance()->tagColor(categories.first()));
    } else if (role == Qt::DecorationRole) {
        return {};
    }

    if (role == TodoRole) {
        return QVariant::fromValue(item);
    }

    if (role == TodoPtrRole) {
        return QVariant::fromValue(todo);
    }

    if (role == IsRichTextRole) {
        if (index.column() == SummaryColumn) {
            return {todo->summaryIsRich()};
        } else if (index.column() == DescriptionColumn) {
            return {todo->descriptionIsRich()};
        } else {
            return {};
        }
    }

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        // If you change this, change headerData() too.
        case RecurColumn:
        case PriorityColumn:
        case PercentColumn:
        case StartDateColumn:
        case DueDateColumn:
        case CategoriesColumn:
        case CalendarColumn:
            return {Qt::AlignHCenter | Qt::AlignVCenter};
        }
        return {Qt::AlignLeft | Qt::AlignVCenter};
    }

    if (sourceModel()) {
        return sourceModel()->data(mapToSource(index.sibling(index.row(), 0)), role);
    }

    return {};
}

QVariant TodoModel::extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role) const
{
    // we customize all columns, not just the extra ones, and thus do all that in ::data()
    Q_UNUSED(parent);
    Q_UNUSED(row);
    Q_UNUSED(extraColumn);
    Q_UNUSED(role);
    return {};
}

bool TodoModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_ASSERT(index.isValid());
    if (!d->m_changer) {
        return false;
    }
    const QVariant oldValue = data(index, role);

    if (oldValue == value) {
        // Nothing changed, the user used one of the QStyledDelegate's editors but seted the old value
        // Lets just skip this then and avoid a roundtrip to akonadi, and avoid sending invitations
        return true;
    }

    const auto item = data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    const KCalendarCore::Todo::Ptr todo = Akonadi::CalendarUtils::todo(item);

    if (!item.isValid() || !todo) {
        qCWarning(AKONADICALENDAR_LOG) << "TodoModel::setData() called, bug item is invalid or doesn't have payload";
        Q_ASSERT(false);
        return false;
    }

    const auto parentCol = Akonadi::EntityTreeModel::updatedCollection(d->etm(), item.parentCollection());
    if (parentCol.rights() & Akonadi::Collection::CanChangeItem) {
        KCalendarCore::Todo::Ptr oldTodo(todo->clone());
        if (role == Qt::CheckStateRole && index.column() == 0) {
            const bool checked = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
            if (checked) {
                todo->setCompleted(QDateTime::currentDateTimeUtc()); // Because it calls Todo::recurTodo()
            } else {
                todo->setCompleted(false);
            }
        }

        if (role == Qt::EditRole) {
            switch (index.column()) {
            case SummaryColumn:
                if (!value.toString().isEmpty()) {
                    todo->setSummary(value.toString());
                }
                break;
            case PriorityColumn:
                todo->setPriority(value.toInt());
                break;
            case PercentColumn:
                todo->setPercentComplete(value.toInt());
                break;
            case StartDateColumn: {
                QDateTime tmp = todo->dtStart();
                tmp.setDate(value.toDate());
                todo->setDtStart(tmp);
                break;
            }
            case DueDateColumn: {
                QDateTime tmp = todo->dtDue();
                tmp.setDate(value.toDate());
                todo->setDtDue(tmp);
                break;
            }
            case CategoriesColumn:
                todo->setCategories(value.toStringList());
                break;
            case DescriptionColumn:
                todo->setDescription(value.toString());
                break;
            }
        }

        if (!todo->dirtyFields().isEmpty()) {
            d->m_changer->modifyIncidence(item, oldTodo);
            // modifyIncidence will eventually call the view's
            // changeIncidenceDisplay method, which in turn
            // will call processChange. processChange will then emit
            // dataChanged to the view, so we don't have to
            // do it here
        }

        return true;
    } else {
        if (!(role == Qt::CheckStateRole && index.column() == 0)) {
            // KOHelper::showSaveIncidenceErrorMsg( 0, todo ); //TODO pass parent
            qCCritical(AKONADICALENDAR_LOG) << "Unable to modify incidence";
        }
        return false;
    }
}

int TodoModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

void TodoModel::setSourceModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }

    beginResetModel();

    if (sourceModel()) {
        disconnect(sourceModel(), &QAbstractItemModel::dataChanged, this, nullptr);
    }

    KExtraColumnsProxyModel::setSourceModel(model);

    if (sourceModel()) {
        connect(sourceModel(), &QAbstractItemModel::dataChanged, this, [this](const auto &begin, const auto &end) {
            d->onDataChanged(begin, end);
        });
    }

    endResetModel();
}

void TodoModel::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    d->m_changer = changer;
}

QVariant TodoModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        switch (column) {
        case SummaryColumn:
            return QVariant(i18n("Summary"));
        case RecurColumn:
            return QVariant(i18n("Recurs"));
        case PriorityColumn:
            return QVariant(i18n("Priority"));
        case PercentColumn:
            return QVariant(i18nc("@title:column percent complete", "Complete"));
        case StartDateColumn:
            return QVariant(i18n("Start Date"));
        case DueDateColumn:
            return QVariant(i18n("Due Date"));
        case CompletedDateColumn:
            return QVariant(i18nc("@title:column date completed", "Completed"));
        case CategoriesColumn:
            return QVariant(i18n("Tags"));
        case DescriptionColumn:
            return QVariant(i18n("Description"));
        case CalendarColumn:
            return QVariant(i18n("Calendar"));
        }
    }

    if (role == Qt::TextAlignmentRole) {
        switch (column) {
        // If you change this, change data() too.
        case RecurColumn:
        case PriorityColumn:
        case PercentColumn:
        case StartDateColumn:
        case DueDateColumn:
        case CategoriesColumn:
        case CalendarColumn:
            return {Qt::AlignHCenter};
        }
        return {};
    }
    return {};
}

void TodoModel::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    Q_UNUSED(calendar);
    // Deprecated, no longer does anything
}

Qt::DropActions TodoModel::supportedDropActions() const
{
    // Qt::CopyAction not supported yet
    return Qt::MoveAction;
}

QStringList TodoModel::mimeTypes() const
{
    static QStringList list;
    if (list.isEmpty()) {
        list << KCalUtils::ICalDrag::mimeType() << KCalUtils::VCalDrag::mimeType();
    }
    return list;
}

QMimeData *TodoModel::mimeData(const QModelIndexList &indexes) const
{
    Akonadi::Item::List items;
    for (const QModelIndex &index : indexes) {
        const auto item = this->data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        if (item.isValid() && !items.contains(item)) {
            items.push_back(item);
        }
    }
    return Akonadi::CalendarUtils::createMimeData(items);
}

bool TodoModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(column)

    if (action != Qt::MoveAction) {
        qCWarning(AKONADICALENDAR_LOG) << "No action other than MoveAction currently supported!"; // TODO
        return false;
    }

    if (d->m_changer && (KCalUtils::ICalDrag::canDecode(data) || KCalUtils::VCalDrag::canDecode(data))) {
        // DndFactory only needs a valid calendar for drag event, not for drop event.
        KCalUtils::DndFactory dndFactory(KCalendarCore::Calendar::Ptr{});
        KCalendarCore::Todo::Ptr t = dndFactory.createDropTodo(data);
        KCalendarCore::Event::Ptr e = dndFactory.createDropEvent(data);

        if (t) {
            // we don't want to change the created todo, but the one which is already
            // stored in our calendar / tree
            const Akonadi::Item item = d->findItemByUid(t->uid(), QModelIndex());
            KCalendarCore::Todo::Ptr todo = Akonadi::CalendarUtils::todo(item);
            KCalendarCore::Todo::Ptr destTodo;
            if (parent.isValid()) {
                const auto parentItem = this->data(parent, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
                if (parentItem.isValid()) {
                    destTodo = Akonadi::CalendarUtils::todo(parentItem);
                }

                auto tmpParent = parent;
                while (tmpParent.isValid()) {
                    const auto parentItem = this->data(parent, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
                    if (!parentItem.isValid()) {
                        break;
                    }
                    const auto parentTodo = Akonadi::CalendarUtils::todo(parentItem);
                    if (!parentTodo) {
                        break;
                    }

                    if (parentTodo->uid() == todo->uid()) { // correct, don't use instanceIdentifier() here
                        Q_EMIT dropOnSelfRejected();
                        return false;
                    }

                    tmpParent = tmpParent.parent();
                }
            }

            if (!destTodo || !destTodo->hasRecurrenceId()) {
                KCalendarCore::Todo::Ptr oldTodo = KCalendarCore::Todo::Ptr(todo->clone());
                // destTodo is empty when we drag a to-do out of a relationship
                todo->setRelatedTo(destTodo ? destTodo->uid() : QString());
                d->m_changer->modifyIncidence(item, oldTodo);

                // again, no need to Q_EMIT dataChanged, that's done by processChange
                return true;
            } else {
                qCDebug(AKONADICALENDAR_LOG) << "Todo's with recurring id can't have child todos yet.";
                return false;
            }
        } else if (e) {
            // TODO: Implement dropping an event onto a to-do: Generate a relationship to the event!
        } else {
            if (!parent.isValid()) {
                // TODO we should create a new todo with the data in the drop object
                qCDebug(AKONADICALENDAR_LOG) << "TODO: Create a new todo with the given data";
                return false;
            }

            const auto parentItem = this->data(parent, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
            KCalendarCore::Todo::Ptr destTodo = Akonadi::CalendarUtils::todo(parentItem);

            if (data->hasText()) {
                QString text = data->text();

                KCalendarCore::Todo::Ptr oldTodo = KCalendarCore::Todo::Ptr(destTodo->clone());

                if (text.startsWith(QLatin1String("file:"))) {
                    destTodo->addAttachment(KCalendarCore::Attachment(text));
                } else {
                    QStringList emails = KEmailAddress::splitAddressList(text);
                    for (QStringList::ConstIterator it = emails.constBegin(); it != emails.constEnd(); ++it) {
                        QString name;
                        QString email;
                        QString comment;
                        if (KEmailAddress::splitAddress(*it, name, email, comment) == KEmailAddress::AddressOk) {
                            destTodo->addAttendee(KCalendarCore::Attendee(name, email));
                        }
                    }
                }
                d->m_changer->modifyIncidence(parentItem, oldTodo);
                return true;
            }
        }
    }

    return false;
}

Qt::ItemFlags TodoModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags ret = KExtraColumnsProxyModel::flags(index);

    const auto item = data(index, Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();

    if (!item.isValid()) {
        Q_ASSERT(mapToSource(index).isValid());
        qCWarning(AKONADICALENDAR_LOG) << "Item is invalid " << index;
        Q_ASSERT(false);
        return Qt::NoItemFlags;
    }

    ret |= Qt::ItemIsDragEnabled;

    const KCalendarCore::Todo::Ptr todo = Akonadi::CalendarUtils::todo(item);

    const auto parentCol = Akonadi::EntityTreeModel::updatedCollection(d->etm(), item.parentCollection());
    if (parentCol.rights() & Akonadi::Collection::CanChangeItem) {
        // the following columns are editable:
        switch (index.column()) {
        case SummaryColumn:
        case PriorityColumn:
        case PercentColumn:
        case StartDateColumn:
        case DueDateColumn:
        case CategoriesColumn:
            ret |= Qt::ItemIsEditable;
            break;
        case DescriptionColumn:
            if (!todo->descriptionIsRich()) {
                ret |= Qt::ItemIsEditable;
            }
            break;
        }
    }

    if (index.column() == 0) {
        // whole rows should have checkboxes, so append the flag for the
        // first item of every row only. Also, only the first item of every
        // row should be used as a target for a drag and drop operation.
        ret |= Qt::ItemIsUserCheckable | Qt::ItemIsDropEnabled;
    }
    return ret;
}

QHash<int, QByteArray> TodoModel::roleNames() const
{
    return {
        {SummaryRole, QByteArrayLiteral("summary")},
        {RecurRole, QByteArrayLiteral("recur")},
        {PriorityRole, QByteArrayLiteral("priority")},
        {PercentRole, QByteArrayLiteral("percent")},
        {StartDateRole, QByteArrayLiteral("startDate")},
        {DueDateRole, QByteArrayLiteral("dueDate")},
        {CategoriesRole, QByteArrayLiteral("categories")},
        {DescriptionRole, QByteArrayLiteral("description")},
        {CalendarRole, QByteArrayLiteral("calendar")},
        {Qt::CheckStateRole, QByteArrayLiteral("checked")},
    };
}

#include "moc_todomodel.cpp"
