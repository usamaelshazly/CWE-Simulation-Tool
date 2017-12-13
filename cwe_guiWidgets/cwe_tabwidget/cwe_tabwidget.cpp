/*
 * the CWE_TabWidget is an extended version of a tab widget where
 * Tabs display a label AND a state
 * the data display area itself hold a standard QTabWidget, one tab per
 * variable group (as defined in the JSon config file)
 */

#include "cwe_tabwidget.h"
#include "ui_cwe_tabwidget.h"

#include "cwe_parampanel.h"
#include "cwe_stagestatustab.h"
#include "cwe_groupswidget.h"
#include "CFDanalysis/CFDcaseInstance.h"

#include "cwe_guiWidgets/cwe_parameters.h"

#include "qdebug.h"

#include "../CFDClientProgram/vwtinterfacedriver.h"

CWE_TabWidget::CWE_TabWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CWE_TabWidget)
{
    ui->setupUi(this);

    //groupWidgetList = new QMap<QString, CWE_GroupsWidget *>();
    //stageTabList    = new QMap<QString, CWE_StageStatusTab *>();

    this->setButtonMode(CWE_BTN_NONE);
    this->setViewState(SimCenterViewState::visible);
}

CWE_TabWidget::~CWE_TabWidget()
{
    delete ui;
}

void CWE_TabWidget::setController(CWE_Parameters * newController)
{
    myController = newController;
}

void CWE_TabWidget::setViewState(SimCenterViewState state)
{
    switch (state)
    {
    case SimCenterViewState::editable:
        m_viewState = SimCenterViewState::editable;
        break;
    case SimCenterViewState::hidden:
        m_viewState = SimCenterViewState::hidden;
        break;
    case SimCenterViewState::visible:
    default:
        m_viewState = SimCenterViewState::visible;
    }
}

SimCenterViewState CWE_TabWidget::viewState()
{
    return m_viewState;
}

void CWE_TabWidget::resetView()
{
    //TODO: clear underlying widgets
}
int CWE_TabWidget::addVarTab(QString key, const QString &label, QJsonArray *varList, QJsonObject *varsInfo, QMap<QString,QString> * setVars)
{
    /*
    int index = addVarTab(key, label);
    if (index >= 0)
    {
        addVarsToTab(key, label, varList, varsInfo, setVars);
    }
    return index;
    */
    return -1;
}

void CWE_TabWidget::addVarsData(QJsonObject JSONgroup, QJsonObject JSONvars)
{

}

void CWE_TabWidget::setParameterConfig(QJsonObject &obj)
{
    QVBoxLayout *tablayout = (QVBoxLayout *)ui->tabsBar->layout();
    delete tablayout;
    tablayout = new QVBoxLayout(this);
    tablayout->setMargin(0);
    tablayout->setSpacing(0);
    ui->tabsBar->setLayout(tablayout);

    QJsonArray  sequence = obj.value(QString("sequence")).toArray();
    QJsonObject stages   = obj.value(QString("stages")).toObject();

    foreach (QJsonValue theStage, sequence)
    {
        QString stageName = theStage.toString();

        /* create a CWE_StageStatusTab */
        CWE_StageStatusTab *tab = new CWE_StageStatusTab(stageName, this);
        tablayout->addWidget(tab);
        //QVBoxLayout *layout = (QVBoxLayout *)ui->tabsBar->layout();

        /* create a CWE_GroupsWidget */
        CWE_GroupsWidget *groupWidget = new CWE_GroupsWidget(this);
        ui->stagePanels->addWidget(groupWidget);

        /* link tab and groupWidget */
        tab->linkWidget(groupWidget);
        groupWidget->linkWidget(tab);

        /* set the parameter information for the CWE_GroupsWidget */
        groupWidget->setParameterConfig(stageName, obj);

        /* connect signals and slots */
        connect(tab,SIGNAL(btn_pressed(CWE_GroupsWidget *,QString)),this,SLOT(on_groupTabSelected(CWE_GroupsWidget *, QString)));
        //connect(tab,SIGNAL(btn_released(CWE_GroupsWidget *)),this,SLOT(on_groupTabSelected(CWE_GroupsWidget *)));


    }

    tablayout->addSpacerItem(new QSpacerItem(10,40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

/* *** moved to SCtrDataWidget ***

QWidget * CWE_TabWidget::addStd(QJsonObject JSONvar, QWidget *parent, QString *setVal)
{
    QVariant defaultOption = JSONvar["default"].toVariant();
    QString unit           = JSONvar["unit"].toString();
    // QJson fails to convert "1" to int, thus: QString::toInt( QJson::toString() )
    int precision          = JSONvar["precision"].toString().toInt();

    QLabel *theName = new QLabel(parent);
    QString displayname = JSONvar["displayname"].toString();
    theName->setText(displayname);
    theName->setMinimumHeight(16);
    //theName->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);

    QWidget * theValue;

    if (precision > 0) {
        theValue = new QDoubleSpinBox(parent);
        ((QDoubleSpinBox *)theValue)->setValue(defaultOption.toDouble());
        ((QDoubleSpinBox *)theValue)->setDecimals(precision);
    }
    else {
        theValue = new QSpinBox(parent);
        ((QSpinBox *)theValue)->setValue(defaultOption.toInt());
    }

    QLabel *theUnit = new QLabel(parent);
    theUnit->setText(unit);

    QGridLayout *layout = (QGridLayout*)(parent->layout());
    int row = layout->rowCount();
    layout->addWidget(theName, row,0);
    layout->addWidget(theValue,row,1);
    layout->addWidget(theUnit, row,2);

    return theValue;
}

QWidget * CWE_TabWidget::addBool(QJsonObject JSONvar, QWidget *parent, QString * setVal)
{
    QLabel *theName = new QLabel(parent);
    QString displayname = JSONvar["displayname"].toString();
    theName->setText(displayname);

    QCheckBox *theBox = new QCheckBox(parent);
    if (setVal == NULL)
    {
        theBox->setChecked(JSONvar["default"].toBool());
    }
    else
    {
        theBox->setChecked(setVal);
    }

    QGridLayout *layout = (QGridLayout*)(parent->layout());
    int row = layout->rowCount();
    layout->addWidget(theName,row,0);
    layout->addWidget(theBox, row,1);

    return theBox;
}

QWidget * CWE_TabWidget::addFile(QJsonObject JSONvar, QWidget *parent, QString * setVal)
{
    QLabel *theName = new QLabel(parent);
    QString displayname = JSONvar["displayname"].toString();
    theName->setText(displayname);

    QLineEdit *theFileName = new QLineEdit(parent);
    theFileName->setText("unknown file name");

    QGridLayout *layout = (QGridLayout*)(parent->layout());
    int row = layout->rowCount();
    layout->addWidget(theName,row,0);
    layout->addWidget(theFileName,row,1,1,2);

    return theFileName;
}

QWidget * CWE_TabWidget::addChoice(QJsonObject JSONvar, QWidget *parent, QString * setVal)
{
    QLabel *theName = new QLabel(parent);
    QString displayname = JSONvar["displayname"].toString();
    theName->setText(displayname);

    QComboBox *theSelection = new QComboBox(parent);
    QJsonObject combo_options = JSONvar["options"].toObject();

    QStandardItemModel *theModel = new QStandardItemModel();
    foreach (const QString &theKey, combo_options.keys())
    {
        //QStandardItem *itm = new QStandardItem(theKey);
        QStandardItem *itm = new QStandardItem(combo_options[theKey].toString());
        theModel->appendRow(itm);
    }
    theSelection->setModel(theModel);
    if ((setVal == NULL) || (!combo_options.contains(*setVal)))
    {
        theSelection->setCurrentText(combo_options[JSONvar["default"].toString()].toString());
    }
    else
    {
        theSelection->setCurrentText(combo_options[*setVal].toString());
    }

    QGridLayout *layout = (QGridLayout*)(parent->layout());
    int row = layout->rowCount();
    layout->addWidget(theName, row,0);
    layout->addWidget(theSelection,row,1,1,2);

    return theSelection;
}


QWidget * CWE_TabWidget::addVector3D(QJsonObject JSONvar, QWidget *parent, QString *setVal )
{
    return NULL;
}

QWidget * CWE_TabWidget::addVector2D(QJsonObject JSONvar, QWidget *parent, QString *setVal )
{
    return NULL;
}

QWidget * CWE_TabWidget::addTensor3D(QJsonObject JSONvar, QWidget *parent, QString *setVal )
{
    return NULL;
}

QWidget * CWE_TabWidget::addTensor2D(QJsonObject JSONvar, QWidget *parent, QString *setVal )
{
    return NULL;
}

QWidget * CWE_TabWidget::addUnknown(QJsonObject JSONvar, QWidget *parent, QString *setVal)
{
    QLabel *theName = new QLabel(parent);
    QString displayname = JSONvar["displayname"].toString();
    theName->setText(displayname);

    QGridLayout *layout = (QGridLayout*)(parent->layout());
    int row = layout->rowCount();
    layout->addWidget(theName,row,0);

    return NULL;
}

void CWE_TabWidget::addType(const QString &varName, const QString &type, QJsonObject JSONvar, QWidget *parent, QString *setVal)
{
    QWidget *widget;

    widget = NULL;

    if      (type == "std")      { widget = this->addStd(JSONvar, parent, setVal); }
    else if (type == "bool")     { widget = this->addBool(JSONvar, parent, setVal); }
    else if (type == "file")     { widget = this->addFile(JSONvar, parent, setVal); }
    else if (type == "choose")   { widget = this->addChoice(JSONvar, parent, setVal); }
    else if (type == "vector2D") { widget = this->addVector2D(JSONvar, parent, setVal); }
    else if (type == "tensor2D") { widget = this->addTensor2D(JSONvar, parent, setVal); }
    else if (type == "vector3D") { widget = this->addVector3D(JSONvar, parent, setVal); }
    else if (type == "tensor3D") { widget = this->addTensor3D(JSONvar, parent, setVal); }
    else                         { widget = this->addUnknown(JSONvar, parent, setVal); }

    // store information for reset operations, data collection, and validation
    InputDataType *varData = new InputDataType;
    varData->name        = varName;
    varData->displayName = JSONvar["displayname"].toString();
    varData->type        = type;
    varData->defValue    = JSONvar["default"].toString();
    varData->widget      = widget;
    if (type == "choose") {
        varData->options = new QJsonObject(JSONvar["options"].toObject());
    }
    else {
        varData->options = NULL;
    }
    variableWidgets->insert(varName, varData);
}

*/

void CWE_TabWidget::on_pbtn_run_clicked()
{
    myController->performCaseCommand(currentSelectedStage, CaseCommand::RUN);
}

QMap<QString, QString> CWE_TabWidget::collectParamData()
{
    /*
     * TODO:
     * -- loop through groupsWidgetList and collect information fromCWE_GroupsWidgets
     */

    QString val;
    QMap<QString, QString> currentParameters;

    /*
    // collect parameter values from all groupWidgets in groupWidgetList
    foreach (const CWE_GroupsWidget *itm, groupWidgetList->values())
    {
        QMap<QString, QString> groupParams = itm->collectParamData();

        // add to output
        foreach (QString varName, groupParams.keys())
        {
            val = groupParams.value(varName);
            currentParameters.insert(varName, val);
        }
    }
    */

    return currentParameters;
}

void CWE_TabWidget::on_pbtn_cancel_clicked()
{
    myController->performCaseCommand(currentSelectedStage, CaseCommand::CANCEL);
}

void CWE_TabWidget::on_pbtn_results_clicked()
{
    myController->switchToResults();
}

void CWE_TabWidget::on_pbtn_rollback_clicked()
{
    myController->performCaseCommand(currentSelectedStage, CaseCommand::ROLLBACK);
}

QString CWE_TabWidget::getStateText(StageState theState)
{
    if (theState == StageState::ERROR)
        return "*** ERROR ***";
    if (theState == StageState::FINISHED)
        return "Task Finished";
    if (theState == StageState::LOADING)
        return "Loading Data ...";
    if (theState == StageState::RUNNING)
        return "Task Running";
    if (theState == StageState::UNRUN)
        return "Not Yet Run";
    return "*** TOTAL ERROR ***";
}

void CWE_TabWidget::setButtonMode(uint mode)
{
    bool btnState;

    btnState = (mode & CWE_BTN_RUN)?true:false;
    ui->pbtn_run->setEnabled(btnState);

    btnState = (mode & CWE_BTN_CANCEL)?true:false;
    ui->pbtn_cancel->setEnabled(btnState);

    btnState = (mode & CWE_BTN_RESULTS)?true:false;
    ui->pbtn_results->setEnabled(btnState);

    btnState = (mode & CWE_BTN_ROLLBACK)?true:false;
    ui->pbtn_rollback->setEnabled(btnState);
}

void CWE_TabWidget::addStageTab(QString key, QJsonObject &obj)
{
    /*
     * create a stage tab for a stage identified by key
    */

    CWE_GroupsWidget *newPanel = new CWE_GroupsWidget(ui->stagePanels);
    CWE_StageStatusTab *newTab = new CWE_StageStatusTab(key, this);
    newPanel->setCorrespondingTab(newTab);
    newTab->setCorrespondingPanel(newPanel);
    ui->tabsBar->layout()->addWidget(newTab);
    ui->stagePanels->addWidget(newPanel);
}


/* *** SLOTS *** */

void CWE_TabWidget::on_groupTabSelected(CWE_GroupsWidget *groupWidget, QString s)
{
    //ui->stagePanels->setCurrentWidget();
}