#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QTextEdit>
#include "../include/BlackFrameDetector.h"
#include <string>
#include <QtConcurrent>
#include <QFutureWatcher>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Black Frame Detector");
    window.resize(400, 600);
    window.setStyleSheet("background: #010202");


    QLabel *logoLabel = new QLabel();
    QPixmap logoPixmap("../photo/250x100.png");

    if (logoPixmap.isNull()) {
        logoLabel->setText("<center><b>Black Frame Detector</b></center>");
    } else {
        logoLabel->setPixmap(logoPixmap.scaledToHeight(125, Qt::SmoothTransformation));
    }
    logoLabel->setAlignment(Qt::AlignCenter);
    
    QLineEdit *pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText("Video file path....");
    pathEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 8px 12px;"               /* отступы внутри */
        "    border: 1.5px solid #515b5c;"        /* тонкая серая рамка */
        "    border-radius: 6px;"               /* скруглённые углы */
        "    background-color: #a5a5a5;"          /* белый фон */
        "    selection-background-color: #142838;" /* цвет выделения текста */
        "    selection-color: white;"
        "    font-size: 14px;"
        "    height: 30px;"
        "    color: #161824"
        "}"
    );

    QPushButton *analyzeButton = new QPushButton();
    analyzeButton->setFixedSize(250, 100); // чтобы не растягивалась

    QString styleAnalyzeBotton =   
    "QPushButton {"
        "    border: none;"
        "    background-image: url(../icons/black-frame-button.png);"
        "    background-repeat: no-repeat;"
        "    background-position: center;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-image: url(../icons/black-frame-button-hover.png);"
        "}"
        "QPushButton:pressed {"
        "    background-image: url(../icons/black-frame-button-analysis.png);"
        "}";

    analyzeButton->setStyleSheet(styleAnalyzeBotton);

    QPushButton *browseButton = new QPushButton("");
    browseButton->setFixedSize(120, 60);

    browseButton->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    background-image: url(../icons/browse-logo.png);"
        "    background-repeat: no-repeat;"
        "    background-position: center;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-image: url(../icons/browse-hover.png);"
        "}"
        "QPushButton:pressed {"
        "    background-image: url(../icons/browse-clicked.png);"
        "}"
    );

    QTextEdit *resultTextEdit = new QTextEdit();
    resultTextEdit->setReadOnly(true);
    resultTextEdit->setPlaceholderText("The result will appear here");
    resultTextEdit->setStyleSheet(
        "QTextEdit{"
        "    background-color: #1c7a8dff;"
        "    color: #e0e0e0;"
        "    border: 1.5px solid #a5a5a5;"
        "    border-radius: 6px;"
        "    padding: 10px;"
        "    font-family: Courier New, monospace;"
        "    font-size: 14px;"
        "}"
    );

    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);
    

    auto *mainLayout = new QVBoxLayout();
    //mainLayout->addWidget(logoLabel);
    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(analyzeButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(resultTextEdit);

    QObject::connect(analyzeButton, &QPushButton::clicked, [styleAnalyzeBotton, analyzeButton, pathEdit, resultTextEdit]() {
        resultTextEdit->clear();
        resultTextEdit->insertPlainText("Analyzing... Please wait.\n\n");
        analyzeButton->setEnabled(false);
        analyzeButton->setStyleSheet(
            "QPushButton {"
            "    background-image: url(../icons/black-frame-button-analysis.png);"
            "}"
        );


        std::string videoPath = pathEdit->text().trimmed().toStdString();
        std::vector<std::string> infoVideoFileVector = infoVideoFile(videoPath);
        QString info;

        if (infoVideoFileVector.size() != 4) {
            info = QString::fromStdString(infoVideoFileVector[0] + "\n");
            analyzeButton->setEnabled(true);
            analyzeButton->setStyleSheet(styleAnalyzeBotton);

        } else {
            info = "Filename: " + QString::fromStdString(infoVideoFileVector[0]) + "\n"
            + "Time: " + QString::fromStdString(infoVideoFileVector[1]) + "\n"
            + "FPS: " + QString::fromStdString(infoVideoFileVector[2]) + "\n"
            + "Frames: " + QString::fromStdString(infoVideoFileVector[3]) + "\n";
            
            QFuture<QString> future = QtConcurrent::run([videoPath]() -> QString {
                std::string result = findBlackFrames(videoPath, 5);
                return QString::fromStdString(result);
            });

            QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>();
            watcher->setFuture(future);

            QObject::connect(watcher, &QFutureWatcher<QString>::finished, 
                [watcher, analyzeButton, resultTextEdit, styleAnalyzeBotton]() {
                    analyzeButton->setStyleSheet(styleAnalyzeBotton);

                    QString result = watcher->result();
                    resultTextEdit->insertPlainText(result);
                    analyzeButton->setEnabled(true); 
                    watcher->deleteLater();
                }
            );
            }

        resultTextEdit->insertPlainText(info + "\n");
    });

    QObject::connect(browseButton, &QPushButton::clicked, [browseButton, pathEdit]() {
        QString filepath = QFileDialog::getOpenFileName(
            nullptr,
            "Select a video file",
            QDir::homePath(),
            "Video files (*.mp4 *.avi *.mov *.mkv);;All files (*)"
        );

        if (!filepath.isEmpty()) {
            pathEdit->setText(filepath);
        }

        browseButton->setChecked(false);
    });

    window.setLayout(mainLayout);
    window.show();

    return qApp->exec();
}