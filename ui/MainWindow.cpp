#include "MainWindow.hpp"

#include <QAction>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>

#include "ApplicationContext.hpp"
#include "Database.hpp"
#include "Scanner.hpp"
#include "Graph.hpp"

namespace slate::ui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), db_(std::make_unique<slate::storage::Database>()) {
    setup_ui();
}

MainWindow::~MainWindow() {
    if (db_) db_->close();
}

void MainWindow::setup_ui() {
    // Paksa menu bar selalu terlihat (non-native)
    menuBar()->setNativeMenuBar(false);

    QMenu *file_menu = menuBar()->addMenu("&File");

    // Open Project
    QAction *open_action = file_menu->addAction("&Open Project...");
    connect(open_action, &QAction::triggered, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Open Project Directory");
        if (!dir.isEmpty()) {
            current_project_path_ = dir;
            scan_project(dir);
        }
    });

    // Scan Project
    QAction *scan_action = file_menu->addAction("&Scan Project");
    connect(scan_action, &QAction::triggered, this, [this]() {
        if (!current_project_path_.isEmpty()) {
            scan_project(current_project_path_);
        } else {
            status_label_->setText("No project path set. Use Open Project first.");
        }
    });

    file_menu->addSeparator();
    QAction *exit_action = file_menu->addAction("E&xit");
    connect(exit_action, &QAction::triggered, this, &QWidget::close);

    // Central widget
    QWidget *central = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(central);

    // Project tree
    project_tree_ = new QTreeView(this);
    tree_model_ = new QStandardItemModel(this);
    tree_model_->setHorizontalHeaderLabels({"Project"});
    project_tree_->setModel(tree_model_);
    hbox->addWidget(project_tree_, 1);

    // Center tabs
    center_tabs_ = new QTabWidget(this);
    metrics_view_ = new QTextEdit(this);
    metrics_view_->setReadOnly(true);
    center_tabs_->addTab(metrics_view_, "Metrics");
    hbox->addWidget(center_tabs_, 3);

    setCentralWidget(central);

    // Status bar
    status_label_ = new QLabel("Ready");
    statusBar()->addWidget(status_label_);
}

void MainWindow::scan_project(const QString &path) {
    status_label_->setText("Scanning: " + path);

    QString db_path = path + "/slate.db";
    db_->close();
    if (!db_->open(db_path.toStdString())) {
        status_label_->setText("Failed to open database");
        return;
    }

    slate::scanner::Scanner scanner;
    auto files = scanner.scan_directory(path.toStdString());

    int session_id = db_->start_indexing_session();

    tree_model_->clear();
    tree_model_->setHorizontalHeaderLabels({"Project"});

    int total_syms = 0;
    int total_deps = 0;

    for (const auto& file : files) {
        auto *file_item = new QStandardItem(QString::fromStdString(file.path.filename().string()));
        file_item->setEditable(false);

        int file_id = db_->insert_file(file.path.string(), file.language);
        auto symbols = scanner.extract_symbols(file);
        for (const auto& sym : symbols) {
            auto *sym_item = new QStandardItem(QString("%1 (%2:%3)")
                .arg(QString::fromStdString(sym.name))
                .arg(QString::fromStdString(sym.kind))
                .arg(sym.line));
            sym_item->setEditable(false);
            file_item->appendRow(sym_item);

            db_->insert_symbol(sym.name, sym.kind, file_id, sym.line, sym.column);
            total_syms++;
        }

        total_deps += static_cast<int>(file.includes.size());
        for (const auto& inc : file.includes) {
            for (const auto& f2 : files) {
                if (f2.path.string().ends_with(inc) ||
                    f2.path.string().ends_with(inc + ".h") ||
                    f2.path.string().ends_with(inc + ".hpp")) {
                    int target_id = db_->insert_file(f2.path.string(), f2.language);
                    auto src_syms = scanner.extract_symbols(file);
                    auto tgt_syms = scanner.extract_symbols(f2);
                    if (!src_syms.empty() && !tgt_syms.empty()) {
                        int src_sym_id = db_->insert_symbol(src_syms[0].name, src_syms[0].kind,
                                                            file_id, src_syms[0].line, src_syms[0].column);
                        int tgt_sym_id = db_->insert_symbol(tgt_syms[0].name, tgt_syms[0].kind,
                                                            target_id, tgt_syms[0].line, tgt_syms[0].column);
                        db_->insert_dependency(src_sym_id, tgt_sym_id, "include");
                    }
                    break;
                }
            }
        }

        tree_model_->appendRow(file_item);
    }

    db_->finish_indexing_session(session_id, "completed");
    project_tree_->expandAll();
    show_stats();
    status_label_->setText(QString("Scanned %1 files, %2 symbols, %3 dependencies")
                               .arg(files.size()).arg(total_syms).arg(total_deps));
}

void MainWindow::show_stats() {
    if (!db_) return;

    int files = db_->count_files();
    int symbols = db_->count_symbols();
    int deps = db_->count_dependencies();

    QString stats;
    stats += QString("Files: %1\n").arg(files);
    stats += QString("Symbols: %1\n").arg(symbols);
    stats += QString("Dependencies: %1\n").arg(deps);

    int circular = 0;
    if (deps > 0) {
        auto deps_list = db_->load_all_dependencies();
        slate::graph::Graph g;
        for (const auto& d : deps_list) {
            slate::graph::Node sn; sn.id = std::to_string(d.source_symbol_id);
            sn.name = ""; sn.type = slate::graph::NodeType::Symbol;
            g.add_node(sn);
            slate::graph::Node tn; tn.id = std::to_string(d.target_symbol_id);
            tn.name = ""; tn.type = slate::graph::NodeType::Symbol;
            g.add_node(tn);
            slate::graph::Edge e; e.from_id = std::to_string(d.source_symbol_id);
            e.to_id = std::to_string(d.target_symbol_id); e.kind = d.kind;
            g.add_edge(e);
        }
        circular = static_cast<int>(g.find_all_cycles().size());
    }
    stats += QString("Circular Dependencies: %1\n").arg(circular);

    metrics_view_->setPlainText(stats);
}

} // namespace slate::ui
