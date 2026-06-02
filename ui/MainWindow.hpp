#pragma once
#include <QMainWindow>
#include <memory>

class QTreeView;
class QTabWidget;
class QLabel;
class QTextEdit;
class QStandardItemModel;

namespace slate::storage {
    class Database;
}

namespace slate::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setup_ui();
    void open_project();
    void scan_project(const QString &path);
    void show_stats();

    QTreeView *project_tree_;
    QTabWidget *center_tabs_;
    QLabel *status_label_;
    QTextEdit *metrics_view_;
    QStandardItemModel *tree_model_;

    std::unique_ptr<slate::storage::Database> db_;
    QString current_project_path_;
};

} // namespace slate::ui
