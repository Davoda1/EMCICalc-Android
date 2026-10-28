// SPDX-License-Identifier: MIT
#pragma once
#include "graphmodel.h"
#include <QWidget>
class QAction;class QCheckBox;class QComboBox;class QDoubleSpinBox;class QLabel;class QLineEdit;class QPushButton;class QScrollArea;class QTabWidget;class QTableWidget;class QTimer;class GraphWorkbenchCanvas;
// Native Qt surface: all geometry is immutable worker-produced plain data.
class GraphWorkbench final:public QWidget {
public:
    explicit GraphWorkbench(QWidget *parent=nullptr);
    ~GraphWorkbench() override;
    std::function<void(EmciGraph::Request,std::shared_ptr<std::atomic_bool>)> onSampleRequested;
    std::function<void(QString)> onStatusMessage;
    std::function<void()> onChanged;
    void acceptResult(const EmciGraph::Result&);
    void requestPlot();
    void cancelWork();
    QJsonObject saveState() const;
    bool restoreState(const QJsonObject&,QString *error=nullptr);
    void addExpression(QString expression,std::shared_ptr<const CalcContextSnapshot> context={});
    void retranslate(const std::function<QString(const char*)>&translator);
    const EmciGraph::Result& result()const{return m_result;}
    void resetView();
    void clearGraphs();
protected:
    void changeEvent(QEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    friend class GraphWorkbenchCanvas;
    void addSeries();void removeSeries();void selectSeries(int);void commitEditor();void refreshList();void refreshRanges();void changed();void fit();void rebuildTable();void tableEdited(int,int);void exportFile(const QString&);
    void status(const QString&);void updateType();void updateButtons();
    void beginCurve();void focusExpression();void updateTablePresentation();
    QString text(const char*)const;
    EmciGraph::Request m_request;
    EmciGraph::Result m_result;
    std::shared_ptr<std::atomic_bool> m_cancel;
    quint64 m_generation=0;bool m_loading=false,m_pending=false,m_accepted=false,m_grid=true,m_equal=false,m_3d=false;
    double m_yaw=.7,m_pitch=.65,m_3dZoom=1;
    int m_selected=-1;
    std::function<QString(const char*)>m_translator;
    QTabWidget*m_tabs=nullptr;GraphWorkbenchCanvas*m_canvas=nullptr;QComboBox*m_seriesSelector=nullptr;QScrollArea*m_seriesScroll=nullptr;
    QLineEdit*m_name=nullptr,*m_expression=nullptr,*m_expressionY=nullptr;
    QComboBox*m_kind=nullptr,*m_angle=nullptr,*m_style=nullptr,*m_resolution=nullptr,*m_tableSeries=nullptr;
    QCheckBox*m_enabled=nullptr,*m_gridCheck=nullptr,*m_equalCheck=nullptr,*m_3dCheck=nullptr,*m_annotations=nullptr,*m_shade=nullptr;
    QDoubleSpinBox*m_bounds[4]{},*m_parameter[2]{},*m_integral[2]{},*m_tableBounds[3]{};
    QLabel*m_status=nullptr,*m_help=nullptr,*m_contextLabel=nullptr;QTableWidget*m_table=nullptr;
    QPushButton*m_color=nullptr,*m_stop=nullptr,*m_plot=nullptr,*m_edit=nullptr,*m_view=nullptr,*m_rotate=nullptr,*m_keyboard=nullptr,*m_add=nullptr,*m_remove=nullptr,*m_tableInputsToggle=nullptr;
    QAction*m_png=nullptr,*m_svg=nullptr,*m_csv=nullptr,*m_fit=nullptr,*m_rotateAction=nullptr;
    QWidget*m_empty=nullptr,*m_plotTools=nullptr,*m_parameterSection=nullptr,*m_integralSection=nullptr,*m_parametricField=nullptr;
    QTimer*m_panTimer=nullptr;
};
