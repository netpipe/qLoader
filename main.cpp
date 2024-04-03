#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Application Manager");

        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        centralWidget->setLayout(mainLayout);

        QHBoxLayout *inputLayout = new QHBoxLayout;
        mainLayout->addLayout(inputLayout);

        nameLabel = new QLabel("Name:", this);
        inputLayout->addWidget(nameLabel);

        nameEdit = new QLineEdit(this);
        inputLayout->addWidget(nameEdit);

        pathLabel = new QLabel("Path:", this);
        inputLayout->addWidget(pathLabel);

        pathEdit = new QLineEdit(this);
        inputLayout->addWidget(pathEdit);

        addButton = new QPushButton("Add", this);
        connect(addButton, &QPushButton::clicked, this, &MainWindow::addEntry);
        inputLayout->addWidget(addButton);

        removeButton = new QPushButton("Remove", this);
        connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeEntry);
        inputLayout->addWidget(removeButton);

        updateButton = new QPushButton("Update", this);
        connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateEntry);
        inputLayout->addWidget(updateButton);

        applicationsList = new QListWidget(this);
        mainLayout->addWidget(applicationsList);

        initializeDatabase();
        loadApplications();

        connect(applicationsList, &QListWidget::itemClicked, this, &MainWindow::loadSelectedApplication);
    }

    ~MainWindow() {
        m_database.close();
    }

private slots:
    void addEntry() {
        QString name = nameEdit->text();
        QString path = pathEdit->text();

        if (!name.isEmpty() && !path.isEmpty()) {
            QSqlQuery query;
            query.prepare("INSERT INTO applications (name, path) VALUES (?, ?)");
            query.addBindValue(name);
            query.addBindValue(path);
            query.exec();

            loadApplications();
            nameEdit->clear();
            pathEdit->clear();
        }
    }

    void removeEntry() {
        QListWidgetItem *item = applicationsList->currentItem();
        if (item) {
            QString name = item->text();
            QSqlQuery query;
            query.prepare("DELETE FROM applications WHERE name = ?");
            query.addBindValue(name);
            query.exec();

            loadApplications();
        }
    }

    void updateEntry() {
        QListWidgetItem *item = applicationsList->currentItem();
        if (item) {
            QString name = item->text();
            QString newName = nameEdit->text();
            QString newPath = pathEdit->text();

            QSqlQuery query;
            query.prepare("UPDATE applications SET name = ?, path = ? WHERE name = ?");
            query.addBindValue(newName);
            query.addBindValue(newPath);
            query.addBindValue(name);
            query.exec();

            loadApplications();
            nameEdit->clear();
            pathEdit->clear();
        }
    }

    void loadSelectedApplication(QListWidgetItem *item) {
        if (item) {
            QString name = item->text();
            QSqlQuery query;
            query.prepare("SELECT path FROM applications WHERE name = ?");
            query.addBindValue(name);
            query.exec();

            if (query.next()) {
                QString path = query.value(0).toString();
                nameEdit->setText(name);
                pathEdit->setText(path);
            }
        }
    }

private:
    void initializeDatabase() {
        m_database = QSqlDatabase::addDatabase("QSQLITE");
        m_database.setDatabaseName("applications.db");

        if (!m_database.open()) {
            qDebug() << "Error: Failed to open database:" << m_database.lastError();
        }

        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS applications (name TEXT, path TEXT)");
    }

    void loadApplications() {
        applicationsList->clear();
        QSqlQuery query("SELECT * FROM applications", m_database);
        while (query.next()) {
            QString name = query.value(0).toString();
            applicationsList->addItem(name);
        }
    }

    QWidget *centralWidget;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *pathLabel;
    QLineEdit *pathEdit;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *updateButton;
    QListWidget *applicationsList;
    QSqlDatabase m_database;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#include "main.moc"
