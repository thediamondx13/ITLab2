#include <ITLab2.h>

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

#include <QMessageBox>
#include <QApplication>

ITLab2::ITLab2(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.progressBar->setValue(0);
    ui.progressBar->setVisible(false);
}

ITLab2::~ITLab2() {}

QString ITLab2::BytesToBitString(const QByteArray& data, qsizetype maxBytes)
{
    qsizetype bytesToShow = qMin(data.size(), maxBytes);
    QString bits;
    bits.reserve(bytesToShow * 8);

    for (qsizetype i = 0; i < bytesToShow; i++)
    {
        uint8_t byte = static_cast<uint8_t>(data[i]);
        for (int b = 7; b >= 0; b--)
            bits.append((byte >> b) & 1 ? '1' : '0');
    }

    if (data.size() > maxBytes)
        bits.append(QString("\n... (показаны первые %1 из %2 байт)").arg(maxBytes).arg(data.size()));

    return bits;
}

std::array<uint8_t, LFSR::REG_SIZE> ITLab2::ParseRegisterState(const QString& stateStr)
{
    assert(stateStr.length() == LFSR::REG_SIZE && "Register state must be exactly REG_SIZE bits");

    std::array<uint8_t, LFSR::REG_SIZE> state{};
    for (int i = 0; i < LFSR::REG_SIZE; i++)
        state[i] = (stateStr.at(i) == '1') ? 1 : 0;

    return state;
}

void ITLab2::on_leRegState_textEdited(const QString& text)
{
    QString filtered;
    for (const QChar& c : text)
        if (c == '0' || c == '1')
            filtered.append(c);

    if (filtered.length() > LFSR::REG_SIZE)
        filtered = filtered.left(LFSR::REG_SIZE);

    if (filtered != text)
    {
        int cursor = ui.leRegState->cursorPosition();
        ui.leRegState->setText(filtered);
        ui.leRegState->setCursorPosition(qMin(cursor, filtered.length()));
    }

    ui.lblRegLen->setText(QString("%1 / %2 бит").arg(filtered.length()).arg(LFSR::REG_SIZE));
}

void ITLab2::on_btnLoad_clicked()
{
    /* Show the open file dialog */
    filePath = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Все файлы (*.*)");
    if (filePath.isEmpty()) return;

    /* Check if file opened */
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл:\n" + filePath);
        filePath.clear();
        return;
    }

    /* Read and close the file */
    fileData = file.readAll();
    file.close();

    resultData.clear();

    /* Update the UI */
    QFileInfo fileInfo(filePath);
    ui.lblFileName->setText(fileInfo.fileName() + QString("  (%1 байт)").arg(fileData.size()));
    ui.teInput->setPlainText(BytesToBitString(fileData));

    ui.teKey->clear();
    ui.teOutput->clear();
    ui.progressBar->setValue(0);
    ui.progressBar->setVisible(false);
    ui.statusLabel->setText("Файл загружен: " + fileInfo.fileName());
}

void ITLab2::on_btnProcess_clicked()
{
    if (fileData.isEmpty())
    {
        ui.statusLabel->setText("Ошибка: сначала загрузите файл.");
        return;
    }

    QString regStr = ui.leRegState->text();

    /* Validate register state */
    if (regStr.length() != LFSR::REG_SIZE)
    {
        ui.statusLabel->setText(
            QString("Ошибка: начальное состояние регистра должно содержать ровно %1 бит (сейчас: %2).").arg(LFSR::REG_SIZE).arg(regStr.length()));

        return;
    }

    if (regStr.count('0') == LFSR::REG_SIZE)
    {
        ui.statusLabel->setText("Ошибка: начальное состояние не может состоять только из нулей.");
        return;
    }

    /* Set up LFSR */
    std::array<uint8_t, LFSR::REG_SIZE> initState = ParseRegisterState(regStr);
    LFSR lfsr(initState);

    const qsizetype fileSize = fileData.size();

    /* Prepare progress bar */
    ui.progressBar->setValue(0);
    ui.progressBar->setVisible(true);
    ui.statusLabel->setText("Обработка...");
    QApplication::processEvents();

    /* Generate keystream and XOR with input data. */
    QByteArray keystream;
    QByteArray result;
    keystream.reserve(fileSize);
    result.reserve(fileSize);

    /* Update progress every ~5% */
    const qsizetype step = qMax(qsizetype(1), fileSize / 20);
    int lastPercent = 0;

    for (qsizetype i = 0; i < fileSize; ++i)
    {
        uint8_t keyByte = lfsr.nextByte();
        keystream.append(static_cast<char>(keyByte));
        result.append(static_cast<char>(static_cast<uint8_t>(fileData[i]) ^ keyByte));

        if ((i + 1) % step == 0 || i == fileSize - 1)
        {
            int percent = static_cast<int>((i + 1) * 100 / fileSize);
            if (percent != lastPercent)
            {
                lastPercent = percent;
                ui.progressBar->setValue(percent);
                QApplication::processEvents();
            }
        }
    }

    resultData = result;

    /* Display key */
    ui.teKey->setPlainText(BytesToBitString(keystream));

    /* Display output */
    ui.teOutput->setPlainText(BytesToBitString(result));

    ui.progressBar->setValue(100);

    QFileInfo fileInfo(filePath);
    bool wasEncrypted = fileInfo.suffix().toLower() == "crypt";
    ui.statusLabel->setText(wasEncrypted
        ? "Готово. Файл расшифрован. Нажмите «Сохранить результат»."
        : "Готово. Файл зашифрован. Нажмите «Сохранить результат».");
}

void ITLab2::on_btnSave_clicked()
{
    if (resultData.isEmpty())
    {
        ui.statusLabel->setText("Ошибка: нет данных для сохранения. Сначала обработайте файл.");
        return;
    }

    QFileInfo fileInfo(filePath);
    bool wasEncrypted = fileInfo.suffix().toLower() == "crypt";

    /* Suggest output filename */
    QString suggestedName;
    if (wasEncrypted)
    {
        /* Remove .crypt extension */
        suggestedName = fileInfo.path() + "/" + fileInfo.completeBaseName();
    }
    else
    {
        /* Add .crypt extension */
        suggestedName = filePath + ".crypt";
    }

    QString saveFileName = QFileDialog::getSaveFileName(this, "Сохранить результат", suggestedName, "Все файлы (*.*)");
    if (saveFileName.isEmpty()) return;

    QFile outFile(saveFileName);
    if (!outFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл:\n" + saveFileName);
        return;
    }

    outFile.write(resultData);
    outFile.close();

    QFileInfo outFi(saveFileName);
    ui.statusLabel->setText("Файл сохранён: " + outFi.fileName());
}
