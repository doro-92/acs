#include "qplot.h"


//! Флаг отображения графиков, разнесенных друг от друга
bool flagSeparatedMultiGraph;
//! Векторы данных
QVector <vectorData> vData;

QPlot::QPlot(QWidget *parent) : QWidget(parent)
{
    InitPlot(parent);
}

QPlot::~QPlot()
{

}

void QPlot::InitPlot(QWidget *parent)
{
    //if (parent->layout()==NULL)
    //QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    //else
    //bicycle iept, robit
    QLayout *verticalLayout=parent->layout();

    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    cPlot = new QCustomPlot(this);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(cPlot->sizePolicy().hasHeightForWidth());
    cPlot->setSizePolicy(sizePolicy);
    verticalLayout->addWidget(cPlot);

    cPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    cPlot->setBackground(QBrush(Qt::black));
    cPlot->axisRect()->setBackground(QBrush(Qt::black));
    cPlot->legend->setBrush(Qt::NoBrush);

    cPlot->xAxis->setLabel("Время, мс");
    cPlot->xAxis->setLabelColor(Qt::white);
    cPlot->xAxis->setBasePen(QPen(Qt::white));
    cPlot->xAxis->setSelectedBasePen(QPen(Qt::white));
    cPlot->xAxis->setTickLength(0, 5);
    cPlot->xAxis->setTickLabelColor(Qt::white);
    cPlot->xAxis->setSelectedTickLabelColor(Qt::white);
    cPlot->xAxis->setTickPen(QPen(Qt::white));
    cPlot->xAxis->setSelectedTickPen(QPen(Qt::white));
    cPlot->xAxis->setSubTickLength(0, 2);
    cPlot->xAxis->setSubTickPen(QPen(Qt::white));
    cPlot->xAxis->setSelectedSubTickPen(QPen(Qt::white));
    cPlot->xAxis2->setBasePen(QPen(Qt::gray));
    cPlot->xAxis2->setSelectedBasePen(QPen(Qt::gray));
    cPlot->yAxis->setTickLength(0, 5);
    cPlot->yAxis->setSubTickLength(0, 2);
    cPlot->yAxis2->setBasePen(QPen(Qt::gray));
    cPlot->yAxis2->setSelectedBasePen(QPen(Qt::gray));
    cPlot->axisRect()->setupFullAxesBox();

    cPlot->xAxis->grid()->setPen(QPen(Qt::gray, 1, Qt::SolidLine));
    cPlot->xAxis->grid()->setVisible(true);
    cPlot->yAxis->grid()->setPen(QPen(Qt::gray, 1, Qt::SolidLine));
    cPlot->yAxis->grid()->setVisible(true);
    cPlot->xAxis->grid()->setSubGridPen(QPen(Qt::gray, 1, Qt::DashLine));
    cPlot->xAxis->grid()->setSubGridVisible(true);
    cPlot->yAxis->grid()->setSubGridPen(QPen(Qt::gray, 1, Qt::DashLine));
    cPlot->yAxis->grid()->setSubGridVisible(true);

    cPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(8);
    cPlot->legend->setFont(legendFont);
    cPlot->legend->setSelectedFont(legendFont);
    cPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

    addRandomGraph();

    // connect slot that ties some axis selections together (especially opposite axes)
    connect(cPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(cPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(cPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
    connect(cPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(cPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), cPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(cPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), cPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(cPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));

    // setup policy and connect slot for context menu popup:
    cPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(cPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

void QPlot::selectionChanged()
{
    /*
     *    normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
     *    the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
     *    and the axis base line together. However, the axis label shall be selectable individually.
     *
     *    The selection state of the left and right axes shall be synchronized as well as the state of the
     *    bottom and top axes.
     *
     *    Further, we want to synchronize the selection of the graphs with the selection state of the respective
     *    legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
     *    or on its legend item.
  */

    // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (cPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        cPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        cPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        cPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        cPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        cPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (cPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        cPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        cPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        cPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        cPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        cPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<cPlot->graphCount(); ++i)
    {
        QCPGraph *graph = cPlot->graph(i);
        QCPPlottableLegendItem *item = cPlot->legend->itemWithPlottable(graph);

        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelected(true);
            ReDrawPlots(i);
        }
    }
}

void QPlot::mousePress(QMouseEvent* event)
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (cPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        cPlot->axisRect()->setRangeDrag(cPlot->xAxis->orientation());
    else if (cPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        cPlot->axisRect()->setRangeDrag(cPlot->yAxis->orientation());
    else
        startPoint = event->pos();
}

void QPlot::mouseRelease(QMouseEvent* event)
{
    QPoint finishPoint = event->pos();
    double w = abs(startPoint.x()-finishPoint.x());
    double h = abs(startPoint.y()-finishPoint.y());

    if (w > 0 && h > 0)
    {
        if (event->button() == Qt::LeftButton)
        {
            double deltax = w/cPlot->axisRect()->width();
            double deltay = h/cPlot->axisRect()->height();
            cPlot->axisRect()->zoomEvent(deltax, deltay, QPoint((startPoint.x() + finishPoint.x()) / 2,(startPoint.y() + finishPoint.y())/2)); //deltax и deltay - коэффициенты масштабирования, QPoint((p1.x()+p2.x())/2,(p2.y()+p2.y())/2) - позиция масштабирования.
        }
    }
}

void QPlot::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (cPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        cPlot->axisRect()->setRangeZoom(cPlot->xAxis->orientation());
    else if (cPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        cPlot->axisRect()->setRangeZoom(cPlot->yAxis->orientation());
    else
        cPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void QPlot::addGraph(QString name, QVector <double> x, QVector <double> y)
{
    int grNum = 0;
    while (grNum < cPlot->graphCount() &&
           cPlot->graph(grNum++)->name() != name) {}

    if (grNum == cPlot->graphCount())
    {
        cPlot->addGraph();
        cPlot->graph()->setName(name);
        cPlot->graph()->setLineStyle((QCPGraph::LineStyle)1);
        cPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)1));
        QPen graphPen;
        graphPen.setWidthF(2);
        switch (cPlot->graphCount())
        {
        case 1:
            graphPen.setColor(Qt::red);
            break;
        case 2:
            graphPen.setColor(Qt::yellow);
            break;
        case 3:
            graphPen.setColor(Qt::green);
            break;
        case 4:
            graphPen.setColor(Qt::magenta);
            break;
        default:
            graphPen.setColor(Qt::red);
            break;
        }
        cPlot->graph()->setPen(graphPen);

        cPlot->legend->item(cPlot->graphCount() - 1)->setTextColor(cPlot->graph(cPlot->graphCount() - 1)->pen().color());
        cPlot->legend->item(cPlot->graphCount() - 1)->setSelectedTextColor(cPlot->graph(cPlot->graphCount() - 1)->pen().color());

        cPlot->graph()->addData(x, y);
        cPlot->xAxis->setRange(cPlot->graph()->data()->keys().operator [](0),
                               cPlot->graph()->data()->keys().last());
    }
    else
    {
        cPlot->graph(grNum - 1)->addData(x, y);
        cPlot->xAxis->setRange(cPlot->graph(grNum - 1)->data()->keys().operator [](0),
                               cPlot->graph(grNum - 1)->data()->keys().last());
    }

    ReDrawPlots(getGraphNum());
}

void QPlot::addRandomGraph()
{
    int n = 50;
    int step;
    vectorData data;
    int graphnum = cPlot->graphCount() + 1;
    data.Name = "Параметр" + QString::number(graphnum) + ", ЕИ" + QString::number(graphnum);

    switch (graphnum)
    {
    case 1:
        step = 1;
        for (int i = 0; i < n; i+=step)
        {
            data.T.push_back(i);
            data.Y.push_back(rand()%110);
        }
        vData.push_back(data);
        break;
    case 2:
        step = 5;
        for (int i = 0; i < n; i+=step)
        {
            data.T.push_back(i);
            data.Y.push_back((rand()%130) - 50);
        }
        vData.push_back(data);
        break;
    case 3:
        step = 3;
        for (int i = 0; i < n; i+=step)
        {
            data.T.push_back(i);
            data.Y.push_back((rand()%5) - 1);
        }
        vData.push_back(data);
        break;
    case 4:
        step = 1;
        for (int i = 0; i < n; i+=step)
        {
            data.T.push_back(i);
            data.Y.push_back((rand()%10) - 3);
        }
        vData.push_back(data);
        break;
    default:
        step = 1;
        for (int i = 0; i < n; i+=step)
        {
            data.T.push_back(i);
            data.Y.push_back(rand()%10);
        }
        vData.push_back(data);
        break;
    }

    addGraph(data.Name, data.T, data.Y);
}

void QPlot::plusRandomGraph()
{
    vectorData data;
    data.Name = "Параметр1, ЕИ1";

    for (int i = cPlot->graph(0)->data()->keys().last(); i < cPlot->graph(0)->data()->keys().last() + 50; i++)
    {
        data.T.push_back(i);
        vData[0].T.push_back(data.T.last());
        data.Y.push_back(rand()%110);
        vData[0].Y.push_back(data.Y.last());
    }

    addGraph(data.Name, data.T, data.Y);
}

void QPlot::removeSelectedGraph()
{
    if (cPlot->selectedGraphs().size() > 0)
    {
        cPlot->removeGraph(cPlot->selectedGraphs().first());
        cPlot->replot();
    }
}

void QPlot::removeAllGraphs()
{
    cPlot->clearGraphs();
    cPlot->replot();
}

void QPlot::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (cPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        menu->addAction("Передвинуть вверх и влево", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Передвинуть вверх и в центр", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Передвинуть вверх и вправо", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Передвинуть вниз и вправо", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Передвинуть вниз и влево", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
    }
    else  // general context menu on graphs requested
    {
        menu->addAction("Добавить график случайных значений", this, SLOT(addRandomGraph()));
        menu->addAction("Дополнить график случайными значениями", this, SLOT(plusRandomGraph()));

        if (cPlot->selectedGraphs().size() > 0)
            menu->addAction("Удалить выбранный график", this, SLOT(removeSelectedGraph()));
        if (cPlot->graphCount() > 0)
            menu->addAction("Удалить все графики", this, SLOT(removeAllGraphs()));

        // Пункт меню для отрисовки всех графиков вместе или разнесенно друг от друга:
        if (flagSeparatedMultiGraph)
            menu->addAction("Отобразить графики вместе", this, SLOT(ShowTogether()));
        else
            menu->addAction("Разнести графики", this, SLOT(ShowSeparate()));
    }

    menu->popup(cPlot->mapToGlobal(pos));
}

void QPlot::moveLegend()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);

        if (ok)
        {
            cPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            cPlot->replot();
        }
    }
}

void QPlot::graphClicked(QCPAbstractPlottable *plottable)
{
//    ui->statusBar->showMessage(QString("Clicked on graph '%1'.").arg(plottable->name()), 1000);
    ReDrawPlots(cPlot->axisRect()->graphs().indexOf(plottable->parentPlot()->selectedGraphs().at(0)));
}

void QPlot::ShowSeparate()
{
    setSeparate(getGraphNum());
}

void QPlot::ShowTogether()
{
    setTogether(getGraphNum());
}

void QPlot::setSeparate(int graphNum)
{
    flagSeparatedMultiGraph = true;

    double maxNew = *std::max_element(vData[graphNum].Y.constBegin(), vData[graphNum].Y.constEnd());
    double minNew = *std::min_element(vData[graphNum].Y.constBegin(), vData[graphNum].Y.constEnd());
    double dltNew = maxNew - minNew;
    double maxOld, minOld, dltOld;
    QVector <double> yNew;

    for (int i = 0; i < cPlot->graphCount(); i++)
    {
        maxOld = *std::max_element(vData[i].Y.constBegin(), vData[i].Y.constEnd());
        minOld = *std::min_element(vData[i].Y.constBegin(), vData[i].Y.constEnd());
        dltOld = maxOld - minOld;
        yNew.clear();

        for (int k = 0; k < vData[i].Y.size(); k++)
            yNew.push_back(vData[i].Y.at(k) * dltNew / dltOld +
                           minNew - minOld * dltNew / dltOld +
                           dltNew * (graphNum - i));

        cPlot->graph(i)->clearData();
        cPlot->graph(i)->addData(vData[i].T, yNew);
    }
    cPlot->yAxis->setRange(dltNew * (graphNum - cPlot->graphCount()) + maxNew, dltNew * graphNum + maxNew);
    setAxisColor(cPlot->graph(graphNum)->pen().color(), graphNum);
    cPlot->replot();
}

void QPlot::setTogether(int graphNum)
{
    flagSeparatedMultiGraph = false;

    double maxNew = *std::max_element(vData[graphNum].Y.constBegin(), vData[graphNum].Y.constEnd());
    double minNew = *std::min_element(vData[graphNum].Y.constBegin(), vData[graphNum].Y.constEnd());
    double dltNew = maxNew - minNew;
    double maxOld, minOld, dltOld;
    QVector <double> yNew;

    for (int i = 0; i < cPlot->graphCount(); i++)
    {
        maxOld = *std::max_element(vData[i].Y.constBegin(), vData[i].Y.constEnd());
        minOld = *std::min_element(vData[i].Y.constBegin(), vData[i].Y.constEnd());
        dltOld = maxOld - minOld;
        yNew.clear();

        for (int k = 0; k < vData[i].Y.size(); k++)
            yNew.push_back(vData[i].Y.at(k) * dltNew / dltOld +
                           minNew - minOld * dltNew / dltOld);

        cPlot->graph(i)->clearData();
        cPlot->graph(i)->addData(vData[i].T, yNew);
    }
    cPlot->yAxis->setRange(minNew, maxNew);
    setAxisColor(cPlot->graph(graphNum)->pen().color(), graphNum);
    cPlot->replot();
}

void QPlot::ReDrawPlots(int graphNum)
{
    if (flagSeparatedMultiGraph)
        setSeparate(graphNum);
    else
        setTogether(graphNum);
}

void QPlot::setAxisColor(QColor color, int graphNum)
{
    if (cPlot->selectedGraphs().isEmpty() ||
        cPlot->graph(graphNum)->selected())
    {
        cPlot->yAxis->setTickLabelColor(QColor(color));
        cPlot->yAxis->setBasePen(QPen(QColor(color)));
        cPlot->yAxis->setTickPen(QPen(QColor(color)));
        cPlot->yAxis->setSubTickPen(QPen(QColor(color)));
        cPlot->yAxis->setLabel(vData[graphNum].Name);
        cPlot->yAxis->setLabelColor(color);

        cPlot->yAxis->setSelectedTickLabelColor(QColor(color));
        cPlot->yAxis->setSelectedBasePen(QPen(QColor(color)));
        cPlot->yAxis->setSelectedTickPen(QPen(QColor(color)));
        cPlot->yAxis->setSelectedSubTickPen(QPen(QColor(color)));
        cPlot->yAxis->setSelectedLabelColor(color);
        cPlot->legend->setSelectedIconBorderPen(QPen(QColor(color)));
    }

    QPen graphPenSel;
    graphPenSel.setColor(cPlot->graph(graphNum)->pen().color());
    graphPenSel.setWidth(3);
    cPlot->graph(graphNum)->setSelectedPen(graphPenSel);
}

int QPlot::getGraphNum()
{
    if (cPlot->selectedGraphs().isEmpty())
        return (cPlot->graphCount() - 1);
    else
        return cPlot->axisRect()->graphs().indexOf(cPlot->selectedGraphs().at(0));
}
