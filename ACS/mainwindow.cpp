#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>

#include "Core/Core.h"


QPoint startPoint;

qint32 id = 1;
QString s_time = "0";
QTimer *timer;
QTimer *timer2;
qint32 TimeDataInc;

Core *oCore;



typedef struct XY
{
   QVector<double> X;
   QVector<double> Y;
}XY;

QVector <XY> *vecPlots;


QVector <qint32> id_plot;
qint32 id_plot_cnt=0;
QVector <QString> time_plot;

//test
quint32 testTimeDataInc=0;






MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{

    id_plot.push_back(1);
    id_plot.push_back(2);
    id_plot.push_back(3);
    id_plot.push_back(4);
    id_plot.push_back(5); //magic ;)


    time_plot.push_back("0");
    time_plot.push_back("0");
    time_plot.push_back("0");
    time_plot.push_back("0");
    time_plot.push_back("0");//magic ;)


    XY sXY;

    vecPlots=new QVector <XY>;

    vecPlots->push_back(sXY);
    vecPlots->push_back(sXY);
    vecPlots->push_back(sXY);
    vecPlots->push_back(sXY);
    vecPlots->push_back(sXY);
    vecPlots->push_back(sXY);

    //qDebug()<<id_plot;
    //qDebug()<<time_plot;
    //qDebug()<<vecPlots;

    InitTimer();
    //InitPlot();


    srand(QDateTime::currentDateTime().toTime_t());

}

MainWindow::~MainWindow()
{
    delete ui;
}

Qt::CheckState CSFromBool(bool flg)
{
    if (flg)
        return  Qt::Checked;
    return  Qt::Unchecked;
}

QPixmap MainWindow::myPaint(QRgb color)
{
    QPixmap pixmap(16,16);
    QPainter painter;
    painter.begin(&pixmap);
    painter.drawRect(0,0,16,16);
    painter.fillRect(0,0,16,16,QBrush(QColor(color)));
    painter.end();
    return pixmap;
}

void MainWindow::SetColorLabel(QLabel *lb, bool flg)
{
    if (flg)
        lb->setPixmap(myPaint(qRgb(255,0,0)));
    else
        lb->setPixmap(myPaint(qRgb(0,255,0)));
}

void MainWindow::TestTimerTick()
{
   if (oCore->GetCountDFQueue()>2)
   {
       ui->checkBox->setChecked(oCore->GetFlgDataFlow(0));
       ui->checkBox_2->setChecked(oCore->GetFlgDataFlow(1));
   }
   ui->checkBox->setChecked(oCore->GetFlgDataFlow(0));

   DBSemaphores dbs=oCore->GetFlgDB();

   ui->listWidget->item(0)->setCheckState(CSFromBool(dbs.QueueWr));
   ui->listWidget->item(1)->setCheckState(CSFromBool(dbs.QueueRd));
   ui->listWidget->item(2)->setCheckState(CSFromBool(dbs.WrQueueDB));
   ui->listWidget->item(3)->setCheckState(CSFromBool(dbs.DBOutputWR));
   ui->listWidget->item(4)->setCheckState(CSFromBool(dbs.DBOutputRd));

   ui->checkBox_3->setChecked(!oCore->GetIsEmptyDFdb());
   SetColorLabel(ui->label_6,false);

   bool flg_u=false;
   for (quint32 i=0;i<oCore->GetCountDFQueue();i++)
       if (oCore->GetFlgDataFlow(i))
       {
           flg_u=true;
           break;
       }

    SetColorLabel(ui->label_7,flg_u);

    if (dbs.QueueRd || dbs.DBOutputWR)
        SetColorLabel(ui->label_8,true);
    else
        SetColorLabel(ui->label_8,false);

    ui->checkBox_4->setChecked(!oCore->GetSuspendStatusDB());

    if (!oCore->GetStatusOpenDB())
    {
        ui->pushButton_5->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->pushButton_4->setEnabled(false);
    }
    else
    {
        ui->pushButton_5->setEnabled(false);
        ui->lineEdit->setEnabled(false);
        ui->pushButton_4->setEnabled(true);
    }
}

void MainWindow::InitCore()
{

    timer2 = new QTimer();
    timer2->start(2);
    connect (timer2, SIGNAL(timeout()), this, SLOT(TestTimerTick()));

    oCore=new Core(1,ui->tab_2,ui->plainTextEdit);
}



//!!!!/////////////////////// TODO: DEL /////////////////////!!!!//
void MainWindow::InitTimer()
{
    TimeDataInc=0;
    timer = new QTimer();
    timer->start(1);
    connect (timer, SIGNAL(timeout()), this, SLOT(timerTick()));


}

void MainWindow::InitPlot()
{
    ui->setupUi(this);

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->customPlot->xAxis->setLabel("Время, мс");
    ui->customPlot->xAxis->setRange(0, 50);
    ui->customPlot->xAxis->setTickLength(0, 5);
    ui->customPlot->xAxis->setSubTickLength(0, 2);
    ui->customPlot->yAxis->setLabel("Значение");
    ui->customPlot->yAxis->setRange(0, 110);
    ui->customPlot->yAxis->setTickLength(0, 5);
    ui->customPlot->yAxis->setSubTickLength(0, 2);
    ui->customPlot->axisRect()->setupFullAxesBox();

    ui->customPlot->xAxis->grid()->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    ui->customPlot->xAxis->grid()->setVisible(true);
    ui->customPlot->yAxis->grid()->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    ui->customPlot->yAxis->grid()->setVisible(true);
    ui->customPlot->xAxis->grid()->setSubGridPen(QPen(Qt::black, 1, Qt::DashLine));
    ui->customPlot->xAxis->grid()->setSubGridVisible(true);
    ui->customPlot->yAxis->grid()->setSubGridPen(QPen(Qt::black, 1, Qt::DashLine));
    ui->customPlot->yAxis->grid()->setSubGridVisible(true);

    ui->customPlot->plotLayout()->insertRow(0);
    ui->customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->customPlot, ""));

    ui->customPlot->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->customPlot->legend->setFont(legendFont);
    ui->customPlot->legend->setSelectedFont(legendFont);
    ui->customPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

//    addRandomGraph();

    // connect slot that ties some axis selections together (especially opposite axes)
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect some interaction slots:
    connect(ui->customPlot, SIGNAL(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)), this, SLOT(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)));
    connect(ui->customPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));

    // setup policy and connect slot for context menu popup:
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

    QVector <QColor> cl;
    cl.push_back(Qt::red);
    cl.push_back(Qt::green);
    cl.push_back(Qt::magenta);
    cl.push_back(Qt::black);
/*
    for (quint32 i=0;i<CurrentSessionConfig->size();i++)
    {
    ui->customPlot->addGraph();
    QString str=qu->CurrentSessionConfig->operator [](i).param_name+","+
            qu->CurrentSessionConfig->operator [](i).unit;
    ui->customPlot->graph()->setName(str);
    QPen graphPen;
    graphPen.setColor(cl[i]);
    graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
    ui->customPlot->graph(i)->setPen(graphPen);
    }

    str=qu->CurrentSessionConfig->operator [](1).param_name+","+
                qu->CurrentSessionConfig->operator [](1).unit;
    ui->customPlot->addGraph();
    ui->customPlot->graph()->setName(str);
    graphPen.setColor(Qt::blue);
    graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
    ui->customPlot->graph(1)->setPen(graphPen);

    str=qu->CurrentSessionConfig->operator [](2).param_name+","+
                qu->CurrentSessionConfig->operator [](2).unit;
    ui->customPlot->addGraph();
    ui->customPlot->graph()->setName(str);
    graphPen.setColor(Qt::green);
    graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
    ui->customPlot->graph(2)->setPen(graphPen);

    str=qu->CurrentSessionConfig->operator [](3).param_name+","+
                qu->CurrentSessionConfig->operator [](3).unit;
    ui->customPlot->addGraph();
    ui->customPlot->graph()->setName(str);
    graphPen.setColor(Qt::black);
    graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
    ui->customPlot->graph(3)->setPen(graphPen);
*/

}

void MainWindow::timerTick()
{
//    if (id_plot_cnt>id_plot.size()-1) id_plot_cnt=0;

//    if (!sem->WrQueueDB && qu->DBOutput->empty())
//    {
//        dbc->GetParameters(id_plot[id_plot_cnt], time_plot[id_plot_cnt]);
//        id_plot_cnt++;
//     }

//    if (qu->DBOutput->isEmpty() || sem->DBOutputWR) return;

//    structNewDataFlow buff;
//    bool flagAddGraph = false;

//    if (id_plot_cnt-1<0) return;

//    while (!qu->DBOutput->empty())
//    {
//    buff = qu->DBOutput->dequeue();
//    vecPlots->operator [](id_plot_cnt-1).X.push_back(buff.time.toDouble());
//    vecPlots->operator [](id_plot_cnt-1).Y.push_back(buff.data.toInt());
//    //X.push_back(buff.time.toDouble());
//    //Y.push_back(buff.data);
//    flagAddGraph = true;
//    }


//    if (flagAddGraph)
//    {

//        DrawMultiGraph(id_plot_cnt-1);
//        //addGraph();
//        time_plot[id_plot_cnt-1]=buff.time;
//    }

//   // time = buff.time;
}

void MainWindow::titleDoubleClick(QMouseEvent* event, QCPPlotTitle* title)
{// Set the plot title by double clicking on it
    Q_UNUSED(event)

    bool ok;
    QString newTitle = QInputDialog::getText(this, "QCustomPlot example", "New plot title:", QLineEdit::Normal, title->text(), &ok);

    if (ok)
    {
        title->setText(newTitle);
        ui->customPlot->replot();
    }
}

void MainWindow::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{  // Set an axis label by double clicking on it
    if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "QCustomPlot example", "New axis label:", QLineEdit::Normal, axis->label(), &ok);

        if (ok)
        {
            axis->setLabel(newLabel);
            ui->customPlot->replot();
        }
    }
}

void MainWindow::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{  // Rename a graph by double clicking on its legend item
    Q_UNUSED(legend)
    if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            ui->customPlot->replot();
        }
    }
}

void MainWindow::selectionChanged()
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
    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) ||
        ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
        ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) ||
        ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of graphs with selection of corresponding legend items:
    for (int i=0; i<ui->customPlot->graphCount(); ++i)
    {
        QCPGraph *graph = ui->customPlot->graph(i);
        QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);

        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelected(true);
        }
    }
}

void MainWindow::mousePress(QMouseEvent* event)
{
    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
    else
    {
        ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
        startPoint = event->pos();
    }
}

void MainWindow::mouseRelease(QMouseEvent* event)
{
    QPoint finishPoint = event->pos();

    if (startPoint.x() != finishPoint.x() && startPoint.y() != finishPoint.y())
    {

    }
}

void MainWindow::mouseWheel()
{
    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::addGraph()
{
    /*
    ui->customPlot->removeGraph(0);
    ui->customPlot->addGraph();
    ui->customPlot->graph()->setName(QString("График %1").arg(ui->customPlot->graphCount()-1));
    ui->customPlot->graph()->setData(X, Y);
    ui->customPlot->xAxis->setRange(X[0], X[X.size() - 1]);
    ui->customPlot->graph()->setLineStyle((QCPGraph::LineStyle)1);
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)1));
    QPen graphPen;
    graphPen.setColor(Qt::red);
    graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
    ui->customPlot->graph()->setPen(graphPen);
    ui->customPlot->replot();*/
}

void MainWindow::DrawMultiGraph(quint32 id)
{
    //ui->customPlot->removeGraph(id);

    //ui->customPlot->graph(0)->addData(
    //ui->customPlot->graph(id)->setName(QString("График "+QString::number(id)));
    ui->customPlot->graph(id)->setData(vecPlots->operator [](id).X, vecPlots->operator [](id).Y);
    ui->customPlot->xAxis->setRange(vecPlots->operator [](id).X[0], vecPlots->operator [](id).X[vecPlots->operator [](id).X.size() - 1]);
    ui->customPlot->graph(id)->setLineStyle((QCPGraph::LineStyle)1);
    ui->customPlot->graph(id)->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)1));

    ui->customPlot->replot();
}
/*
void MainWindow::addRandomGraph()
{
    int n = 50; // number of points in graph
    QVector <structNewDataFlow> vectr;
    structNewDataFlow curr_data;

    for (int i=0; i<n; i++)
    {
        if (i == 25)
            curr_data.time = "0";
        else
            curr_data.time = (((i + 1) / 2) * 2);
        curr_data.data = rand()%110;
        vectr.push_back(curr_data);
    }
    addGraph(vectr);
}
*/

void MainWindow::contextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
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
        if (ui->customPlot->selectedGraphs().size() > 0)
            menu->addAction("Удалить выбранный график", this, SLOT(removeSelectedGraph()));
        if (ui->customPlot->graphCount() > 0)
            menu->addAction("Удалить все графики", this, SLOT(removeAllGraphs()));
    }

    menu->popup(ui->customPlot->mapToGlobal(pos));
}

void MainWindow::moveLegend()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);

        if (ok)
        {
            ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            ui->customPlot->replot();
        }
    }
}

void MainWindow::graphClicked(QCPAbstractPlottable *plottable)
{
    ui->statusBar->showMessage(QString("Clicked on graph '%1'.").arg(plottable->name()), 1000);
}



//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Test functions //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

//радом вектор для внутренних тестов
void MainWindow::on_pushButton_clicked()
{
        QTime midnight(0,0,0);
        qsrand(midnight.secsTo(QTime::currentTime()));
        //ui->textEdit->append("test rnd 1="+QString::number((qrand() % 100)));
        //ui->textEdit->append("test rnd 2="+QString::number((qrand() % 100)));


        QVector <structDataFlow> vecDataFlow;

        structDataFlow strtDF;

       for (int i=0;i<ui->lineEdit_2->text().toInt();i++)
       {
           strtDF.data=QString::number((qrand() % 100));
           strtDF.numDevice=1;
           strtDF.numParam=1;
           strtDF.time=QString::number(testTimeDataInc++);

           vecDataFlow.push_back(strtDF);
       }

    oCore->TestSetDataFlow(&vecDataFlow);
}


//занимает очередь 1
void MainWindow::on_pushButton_2_clicked()
{
    oCore->TestSetFlgDataFlow(0,!oCore->GetFlgDataFlow(0));
}

//занимает очередь 2
void MainWindow::on_pushButton_3_clicked()
{
    oCore->TestSetFlgDataFlow(1,!oCore->GetFlgDataFlow(1));
}

//запуск и приостановка потока
void MainWindow::on_pushButton_7_clicked()
{
    oCore->TestControlDBThread();
}

//последняя ошибка БД
void MainWindow::on_pushButton_8_clicked()
{
  QMessageBox::critical(NULL,"Ошибка",oCore->TestGetLastErrorDB());
}

//создает или открывает БД
void MainWindow::on_pushButton_5_clicked()
{
    oCore->OpenCreateDBFromSerialNumber(ui->lineEdit->text());
}

//закрывает БД
void MainWindow::on_pushButton_4_clicked()
{
    oCore->CloseDB();
}

//запуск теста
void MainWindow::on_pushButton_9_clicked()
{
    if (!oCore->StartTest())
        QMessageBox::critical(NULL,"Ошибка",oCore->TestGetLastErrorDB());
}

// вызывает парсинг кода
void MainWindow::on_pushButton_6_clicked()
{
    oCore->Parse(ui->plainTextEdit_2);
}

//////////////////////////////////////////////////////////////////////////
