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
        "}"
    );

    QPushButton *analyzeButton = new QPushButton();
    analyzeButton->setFixedSize(250, 100); // чтобы не растягивалась

    // Устанавливаем стиль с фоновыми изображениями
    analyzeButton->setStyleSheet(
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
        "}"
    );

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

    QObject::connect(analyzeButton, &QPushButton::clicked, [analyzeButton, pathEdit, resultTextEdit]() {
        std::string videoPath = pathEdit->text().trimmed().toStdString();
        QString res = QString::fromStdString(findBlackFrames(videoPath, 5));
        analyzeButton->setChecked(false);
        resultTextEdit->setPlainText(res);

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