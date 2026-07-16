#include <QApplication>
#include <QFont>
#include "MainWindow.h"

static const char *kStyleSheet = R"(
QWidget {
    background-color: #12161b;
    color: #dbe1e6;
    font-family: "Segoe UI", sans-serif;
    font-size: 10pt;
}

QMainWindow {
    background-color: #0a0d10;
}

QGroupBox {
    background-color: #12161b;
    border: 1px solid #232a31;
    border-radius: 8px;
    margin-top: 14px;
    padding-top: 12px;
    font-weight: 600;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
    color: #52d9c9;
}

QLineEdit, QComboBox, QPlainTextEdit {
    background-color: #161b21;
    border: 1px solid #232a31;
    border-radius: 6px;
    padding: 6px 8px;
    selection-background-color: #2c8478;
}
QLineEdit:focus, QComboBox:focus {
    border: 1px solid #52d9c9;
}
QLineEdit:disabled, QComboBox:disabled {
    color: #6b7580;
    background-color: #12161b;
}
QLineEdit::placeholder {
    color: #6b7580;
}

QComboBox::drop-down {
    border: none;
    width: 22px;
}
QComboBox QAbstractItemView {
    background-color: #161b21;
    border: 1px solid #232a31;
    selection-background-color: #2c8478;
    outline: none;
}

QPushButton {
    background-color: #1c2229;
    border: 1px solid #232a31;
    border-radius: 6px;
    padding: 8px 16px;
    font-weight: 600;
}
QPushButton:hover {
    background-color: #232a31;
    border-color: #3a444d;
}
QPushButton:pressed {
    background-color: #161b21;
}
QPushButton:disabled {
    color: #4a545c;
    background-color: #161b21;
}

QPushButton#downloadButton {
    background-color: #52d9c9;
    color: #03211d;
    border: none;
    font-size: 11pt;
}
QPushButton#downloadButton:hover {
    background-color: #68e3d4;
}
QPushButton#downloadButton:disabled {
    background-color: #2c8478;
    color: #0a0d10;
}

QPushButton#cancelButton {
    color: #f2555a;
    border: 1px solid #3a2226;
}
QPushButton#cancelButton:hover {
    background-color: #241417;
}

QCheckBox {
    spacing: 8px;
}
QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid #3a444d;
    border-radius: 4px;
    background-color: #161b21;
}
QCheckBox::indicator:checked {
    background-color: #52d9c9;
    border-color: #52d9c9;
}

QProgressBar {
    background-color: #161b21;
    border: 1px solid #232a31;
    border-radius: 6px;
    text-align: center;
    color: #dbe1e6;
    height: 18px;
}
QProgressBar::chunk {
    background-color: #52d9c9;
    border-radius: 5px;
}

QPlainTextEdit {
    font-family: "Cascadia Mono", "Consolas", monospace;
    font-size: 9.5pt;
}

QScrollBar:vertical {
    background: #12161b;
    width: 10px;
}
QScrollBar::handle:vertical {
    background: #2a323a;
    border-radius: 5px;
    min-height: 24px;
}
QScrollBar::handle:vertical:hover {
    background: #3a444d;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}
)";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("YtDlpGui");
    QApplication::setOrganizationName("Necto");
    app.setStyleSheet(kStyleSheet);

    MainWindow window;
    window.resize(760, 660);
    window.show();

    return app.exec();
}