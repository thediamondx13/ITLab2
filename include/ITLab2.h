#pragma once

#include <LFSR.h>

#include <QtWidgets/QMainWindow>
#include "ui_ITLab2.h"

class ITLab2 : public QMainWindow
{
    Q_OBJECT

public:
    ITLab2(QWidget* parent = nullptr);
    ~ITLab2();

private:
    Ui::ITLab2Class ui;

    QString filePath;
    QByteArray fileData;
    QByteArray resultData;

    /* Converts raw bytes to a binary string, capped at maxBytes for display */
    static QString BytesToBitString(const QByteArray& data, qsizetype maxBytes = 512);

    /* Parses the 37-char '0'/'1' string from the register input field */
    static std::array<uint8_t, LFSR::REG_SIZE> ParseRegisterState(const QString& stateStr);

private slots:
    void on_btnLoad_clicked();
    void on_btnProcess_clicked();
    void on_btnSave_clicked();
    void on_leRegState_textEdited(const QString& text);
};
