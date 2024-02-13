/*
    SPDX-FileCopyrightText: 2004, 2005, 2006 Carsten Niehaus <cniehaus@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "elementdataviewer.h"

#include <element.h>

#include "prefs.h"

// Qt-Includes
#include "kalzium_debug.h"
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QPen>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KHelpClient>
#include <KPlotAxis>
#include <KPlotObject>
#include <KStandardAction>
#include <KUnitConversion/Converter>

AxisData::AxisData(AXISTYPE type)
    : currentDataType(-1)
{
    m_type = type;
}

double AxisData::value(int element) const
{
    if ((element < 1) || (element > dataList.count())) {
        return 0.0;
    }

    return dataList[element - 1];
}

ElementDataViewer::ElementDataViewer(QWidget *parent)
    : QDialog(parent)
    , m_yData(new AxisData(AxisData::Y))
    , m_xData(new AxisData(AxisData::X))
{
    setWindowTitle(i18nc("@title:window", "Plot Data"));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Close);
    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ElementDataViewer::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ElementDataViewer::reject);
    mainLayout->addWidget(buttonBox);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    KalziumDataObject *kdo = KalziumDataObject::instance();

    ui.setupUi(mainWidget);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);

    // setup the list of names
    foreach (Element *e, kdo->ElementList) {
        names << e->dataAsString(ChemicalDataObject::name);
        symbols << e->dataAsString(ChemicalDataObject::symbol);
        elecConfig << e->dataAsString(ChemicalDataObject::electronicConfiguration);
        block << e->dataAsString(ChemicalDataObject::periodTableBlock);
    }

    m_actionCollection = new KActionCollection(this);
    KStandardAction::quit(this, SLOT(close()), m_actionCollection);

    connect(m_timer, &QTimer::timeout, this, &ElementDataViewer::drawPlot);
    connect(ui.KCB_y, QOverload<int>::of(&KComboBox::activated), this, &ElementDataViewer::rangeChanged);
    connect(ui.KCB_x, QOverload<int>::of(&KComboBox::activated), this, &ElementDataViewer::rangeChanged);
    connect(ui.comboElementLabels, QOverload<int>::of(&KComboBox::activated), this, &ElementDataViewer::rangeChanged);
    connect(ui.comboElementType, QOverload<int>::of(&KComboBox::activated), this, &ElementDataViewer::rangeChanged);
    connect(ui.from, QOverload<int>::of(&QSpinBox::valueChanged), this, &ElementDataViewer::rangeChanged);
    connect(ui.to, QOverload<int>::of(&QSpinBox::valueChanged), this, &ElementDataViewer::rangeChanged);
    connect(buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &ElementDataViewer::slotHelp);
    connect(ui.full, &QPushButton::clicked, this, &ElementDataViewer::fullRange);
    connect(ui.swapXYAxis, &QPushButton::clicked, this, &ElementDataViewer::swapXYAxis);
    drawPlot();

    resize(650, 500);
}

ElementDataViewer::~ElementDataViewer()
{
    delete m_yData;
    delete m_xData;
}

void ElementDataViewer::slotHelp()
{
    KHelpClient::invokeHelp(QStringLiteral("tools.html#plot_data"), QStringLiteral("kalzium"));
}

void ElementDataViewer::rangeChanged()
{
    if (ui.from->value() > ui.to->value()) {
        ui.to->setValue(ui.from->value());
    }

    m_timer->stop();
    m_timer->start(500);
}

void ElementDataViewer::fullRange()
{
    ui.from->setValue(1);
    ui.to->setValue(116);
}

void ElementDataViewer::swapXYAxis()
{
    int x = ui.KCB_x->currentIndex();
    int y = ui.KCB_y->currentIndex();

    ui.KCB_x->setCurrentIndex(y);
    ui.KCB_y->setCurrentIndex(x);

    rangeChanged();
}

void ElementDataViewer::setLimits()
{
    qCDebug(KALZIUM_LOG) << "ElementDataViewer::setLimits()";

    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;

    getMinMax(x1, x2, m_xData);
    getMinMax(y1, y2, m_yData);

    qCDebug(KALZIUM_LOG) << x1 << " :: " << x2 << " ----- " << y1 << " :: " << y2;

    // JH: add some padding to show all points
    double dx = 0.05 * (x2 - x1);
    double dy = 0.05 * (y2 - y1);
    x1 -= dx;
    x2 += dx;
    y1 -= dy;
    y2 += dy;

    // X     // try to put a small padding to make the points on the axis visible
    // X     double dx = (to - from + 1) / 25 + 1.0;
    // X     double dy = (maxY - minY) / 10.0;
    // X     // in case that dy is quite small (for example, when plotting a single
    // X     // point)
    // X     if (dy < 1e-7)
    // X         dy = maxY / 10.0;
    // X     ui.plotwidget->setLimits(from - dx, to + dx, minY - dy, maxY + dy);

    ui.plotwidget->setLimits(x1, x2, y1, y2);
}

void ElementDataViewer::getMinMax(double &min, double &max, AxisData *data)
{
    int firstElement = ui.from->value();
    int lastElement = ui.to->value();

    double minValue = data->value(firstElement);
    double maxValue = data->value(firstElement);

    qCDebug(KALZIUM_LOG) << "Taking elements from " << firstElement << " to " << lastElement;

    for (int _currentVal = firstElement; _currentVal <= lastElement; ++_currentVal) { // go over all selected elements
        double v = data->value(_currentVal);

        if (minValue > v) {
            minValue = v;
        }
        if (maxValue < v) {
            maxValue = v;
        }
    }

    qCDebug(KALZIUM_LOG) << "The value are ]" << minValue << " , " << maxValue << "[.";

    min = minValue;
    max = maxValue;
}

void ElementDataViewer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        slotZoomIn();
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        slotZoomOut();
        break;
    case Qt::Key_Escape:
        close();
        break;
    }
}

void ElementDataViewer::slotZoomIn()
{
}
void ElementDataViewer::slotZoomOut()
{
}

void ElementDataViewer::setupAxisData(AxisData *data)
{
    int selectedData = 0;
    if (data->type() == AxisData::X) {
        selectedData = ui.KCB_x->currentIndex();
    } else {
        selectedData = ui.KCB_y->currentIndex();
    }

    data->currentDataType = selectedData;

    // init to _something_
    ChemicalDataObject::BlueObelisk kind = ChemicalDataObject::mass;
    QString caption;
    int unit = KUnitConversion::NoUnit;

    switch (selectedData) {
    case AxisData::NUMBER: {
        kind = ChemicalDataObject::atomicNumber;
        caption = i18n("Atomic Number");
        break;
    }
    case AxisData::MASS: {
        kind = ChemicalDataObject::mass;
        caption = i18n("Atomic Mass");
        break;
    }
    case AxisData::EN: {
        kind = ChemicalDataObject::electronegativityPauling;
        caption = i18n("Electronegativity");
        break;
    }
    case AxisData::MELTINGPOINT: {
        kind = ChemicalDataObject::meltingpoint;
        caption = i18n("Melting Point");
        unit = Prefs::temperatureUnit();
        break;
    }
    case AxisData::BOILINGPOINT: {
        kind = ChemicalDataObject::boilingpoint;
        caption = i18n("Boiling Point");
        unit = Prefs::temperatureUnit();
        break;
    }
    case AxisData::ATOMICRADIUS: {
        kind = ChemicalDataObject::radiusVDW;
        caption = i18n("Atomic Radius");
        unit = Prefs::lengthUnit();
        break;
    }
    case AxisData::COVALENTRADIUS: {
        kind = ChemicalDataObject::radiusCovalent;
        caption = i18n("Covalent Radius");
        unit = Prefs::lengthUnit();
        break;
    }
    }
    KalziumDataObject *kdo = KalziumDataObject::instance();

    DoubleList dblDataList;
    foreach (Element *element, kdo->ElementList) {
        dblDataList << element->dataAsVariant(kind, unit).toDouble();
    }

    data->dataList.clear();
    data->dataList << dblDataList;
    data->kind = kind;

    if (unit != KUnitConversion::NoUnit) {
        QString stringUnit = KalziumDataObject::instance()->unitAsString(unit);
        data->unit = QString(' ' + stringUnit);

        caption.append(" [");
        caption.append(stringUnit);
        caption.append(']');
    }

    if (data->type() == AxisData::X) {
        ui.plotwidget->axis(KPlotWidget::BottomAxis)->setLabel(caption);
    } else {
        ui.plotwidget->axis(KPlotWidget::LeftAxis)->setLabel(caption);
        ui.plotwidget->axis(KPlotWidget::RightAxis)->setLabel(caption);
    }
}

void ElementDataViewer::drawPlot()
{
    /*
     * to be 100% safe delete the old list
     */
    ui.plotwidget->removeAllPlotObjects();

    /*
     * spare the next step in case everything is already set and done
     */
    if (m_yData->currentDataType != ui.KCB_y->currentIndex()) {
        initData();
    }

    if (m_xData->currentDataType != ui.KCB_x->currentIndex()) {
        initData();
    }

    /*
     * if the user selected the elements 20 to 30 the list-values are 19 to 29!!!
     */
    const int tmpfrom = ui.from->value();
    const int tmpto = ui.to->value();
    const int from = qMin(tmpfrom, tmpto);
    const int to = qMax(tmpfrom, tmpto);

    /*
     * The number of elements. #20 to 30 are 30-20+1=11 Elements
     */
    int num = to - from + 1;

    setLimits();

    QSet<int> metals, metalloids, nonMetals;

    metals << 3 << 4 << 11 << 12 << 13;
    for (int i = 19; i <= 31; ++i) {
        metals << i;
    }
    for (int i = 37; i <= 50; ++i) {
        metals << i;
    }
    for (int i = 55; i <= 83; ++i) {
        metals << i;
    }
    for (int i = 87; i <= 116; ++i) {
        metals << i;
    }

    metalloids << 5 << 14 << 32 << 33 << 51 << 52 << 84 << 85;

    nonMetals << 1 << 2 << 6 << 7 << 8 << 9 << 10 << 15 << 16;
    nonMetals << 17 << 18 << 34 << 35 << 36 << 53 << 54 << 86;

    /*
     * check if the users wants to see the elementnames or not
     */
    int whatShow = ui.comboElementLabels->currentIndex();

    /*
     * Checks what type of element, the user wants to plot.
     * example, metal, non-metal.
     */
    int whichType = ui.comboElementType->currentIndex();

    KPlotObject *dataPointGreen = nullptr;
    KPlotObject *dataPointRed = nullptr;

    double av_x = 0.0;
    double max_x = m_xData->value(from);
    double min_x = m_xData->value(from);
    double av_y = 0.0;
    double max_y = m_yData->value(from);
    double min_y = m_yData->value(from);

    /*
     * iterate for example from element 20 to 30 and construct
     * the KPlotObjects
     */
    dataPointGreen = new KPlotObject(Qt::green, KPlotObject::Points, 4, KPlotObject::Star);
    dataPointGreen->setLabelPen(QPen(Qt::blue));

    dataPointRed = new KPlotObject(Qt::red, KPlotObject::Points, 4,
                                   KPlotObject::Star); // Star can be replaced with a cross
    dataPointRed->setLabelPen(QPen(Qt::blue));

    for (int i = from; i < to + 1; ++i) {
        double value_y = m_yData->value(i);
        double value_x = m_xData->value(i);

        bool known = ((value_y) > 0.0) ? true : false;
        // The element is know if its value is not zero
        bool belongs = true;
        // The value of belongs is one if it belongs to the particular group

        // See if the particular element belongs to the selected set or not.
        // If a particular group of elements is selected,
        if (whichType > 0) {
            belongs = false;
            switch (whichType) {
            case 1: // Plot only metals
                belongs = metals.contains(i);
                break;
            case 2: // plot only nonmetals and metalloids
                belongs = (nonMetals.contains(i) || metalloids.contains(i));
                break;
            case 3: // Plot s block elements
                belongs = (block[i - 1] == QLatin1String("s"));
                break;
            case 4: // Plot p block elements
                belongs = (block[i - 1] == QLatin1String("p"));
                break;
            case 5: // Plot d block elements
                belongs = (block[i - 1] == QLatin1String("d"));
                break;
            case 6: // plot f block elements
                belongs = (block[i - 1] == QLatin1String("f"));
                break;
            case 7: // Noble gases
                belongs = ((elecConfig[i - 1]).endsWith(QLatin1String("p6")));
                belongs |= (i == 2); // Include Helium
                break;
            case 8: // Alkalie metals
                belongs = ((elecConfig[i - 1]).endsWith(QLatin1String("s1")));
                belongs &= (block[i - 1] == QLatin1String("s")); // exclude chromium
                belongs &= (i != 1); // exclude Hydrogen
                break;
            case 9: // Alkaline earth metals
                belongs = ((elecConfig[i - 1]).endsWith(QLatin1String("s2")));
                belongs &= (block[i - 1] == QLatin1String("s")); // exclude chromium
                belongs &= (i != 2); // exclude Helium
                break;
            case 10: // Lanthanides
                // If element i is an f block element, with
                // electronic configuration containing "f4" in it
                // or the element is Lanthanum
                belongs = ((block[i - 1] == QLatin1String("f")) && ((elecConfig[i - 1]).contains(QLatin1String("4f")))) || (i == 57); // Lanthanum 57
                break;
            case 11: // Actinides
                // If element i is an f block element, with
                //  electronic configuration containing "f5" in it
                //  or the element is Actinium
                belongs = (((block[i - 1] == QLatin1String("f"))) && ((elecConfig[i - 1]).contains(QLatin1String("5f")))) || (i == 89); // Actinium 89
                break;
            case 12: // Radio active
                belongs = ((i == 43) || (i == 61) || (i > 84));
                // Technitium prothomium and then polonium onwards.
                break;
            default:
                whichType = 0;
                belongs = true;
            }
        }
        if (belongs) {
            if (known) {
                av_x += value_x;
                av_y += value_y;

                if (value_x > max_x) {
                    max_x = value_x;
                }
                if (value_y > max_y) {
                    max_y = value_y;
                }
                if (value_x < min_x) {
                    min_x = value_x;
                }
                if (value_y < min_y) {
                    min_y = value_y;
                }

                QString lbl;
                if (whatShow > 0) { // The users wants to see the labels
                    lbl = whatShow == 1 ? names[i - 1] : symbols[i - 1];
                }

                dataPointGreen->addPoint(value_x, value_y, lbl);
            } else { // unknown value
                // num is required while finding the average, if an element is not
                // known it should not contribute to the average.
                --num;

                QString lbl;
                if (whatShow > 0) { // The user wants to see the labels
                    lbl = whatShow == 1 ? names[i - 1] : symbols[i - 1];
                }

                dataPointRed->addPoint(value_x, value_y, lbl);
                // For an Unknown value, use a red point to mark the data-point.
            }
        } else { // The element does not belong to the set
            // num is required while finding average, if an element is
            // not in the selected set, it should not contribute to the avg.
            --num;
        }
    }

    ui.plotwidget->addPlotObject(dataPointGreen);
    ui.plotwidget->addPlotObject(dataPointRed);

    if (num > 0) {
        // now set the values for the min, max and average value
        ui.av_x->setText(QString::number(av_x / num).append(m_xData->unit));
        ui.minimum_x->setText(QString::number(min_x).append(m_xData->unit));
        ui.maximum_x->setText(QString::number(max_x).append(m_xData->unit));

        ui.av_y->setText(QString::number(av_y / num).append(m_yData->unit));
        ui.minimum_y->setText(QString::number(min_y).append(m_yData->unit));
        ui.maximum_y->setText(QString::number(max_y).append(m_yData->unit));
    } else {
        ui.av_x->setText(QString::number(0.0));
        ui.minimum_x->setText(QString::number(0.0));
        ui.maximum_x->setText(QString::number(0.0));
        ui.av_y->setText(QString::number(0.0));
        ui.minimum_y->setText(QString::number(0.0));
        ui.maximum_y->setText(QString::number(0.0));
    }
}

void ElementDataViewer::initData()
{
    setupAxisData(m_xData);
    setupAxisData(m_yData);
}

#include "moc_elementdataviewer.cpp"
