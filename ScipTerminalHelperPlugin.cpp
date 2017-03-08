/*
	This file is part of the UrgBenri application.

	Copyright (c) 2016 Mehrez Kristou.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Please contact kristou@hokuyo-aut.jp for more details.

*/

#include "ScipTerminalHelperPlugin.h"
#include "ui_ScipTerminalHelperPlugin.h"
#include "Connection.h"
#include "ConnectionUtils.h"

#include <QMessageBox>
#include <QDebug>
#include <QScrollBar>
#include <QTime>
#include <QThread>
#include <QToolTip>

#define LINES_PER_COMMAND 25

ScipTerminalHelperPlugin::ScipTerminalHelperPlugin(QWidget* parent)
    : HelperPluginInterface(parent)
    , ui(new Ui::ScipTerminalHelperPlugin)
    , m_sensor(NULL)
    , m_historySize(24000)
    , m_receptionThread(&receptionThreadProcess, this)
    , m_txColor(QColor(Qt::blue))
    , m_rxColor(QColor(Qt::red))
{
    REGISTER_FUNCTION(setDeviceMethod);

    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setWindowTitle(pluginName());
    setWindowIcon(pluginIcon());


    ui->console->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ui->console->setFont(QFont("Monaco", 10));
    ui->console->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    ui->console->setReadOnly(true);
    ui->console->setWordWrapMode(QTextOption::NoWrap);
    ui->console->setMaximumBlockCount(m_historySize * LINES_PER_COMMAND);
    connect(ui->console, &QPlainTextEdit::selectionChanged,
            this, &ScipTerminalHelperPlugin::decodeSelection);

    connect(ui->commandLine, &CommandLine::returnPressed,
            this, [&](){
        QString ending;
        if(ui->crButton->isChecked()) ending += "\r";
        if(ui->lfButton->isChecked()) ending += "\n";
        sendText(ending);
    });

    connect(this, &ScipTerminalHelperPlugin::connectionDataReady,
            this, &ScipTerminalHelperPlugin::recieveConnectionData);

    ui->lfButton->setToolTip("⤶");
    ui->crButton->setToolTip("¶");
}


ScipTerminalHelperPlugin::~ScipTerminalHelperPlugin(void)
{
}

void ScipTerminalHelperPlugin::saveState(QSettings &settings)
{

}

void ScipTerminalHelperPlugin::restoreState(QSettings &settings)
{

}

void ScipTerminalHelperPlugin::loadTranslator(const QString &locale)
{
    qApp->removeTranslator(&m_translator);
    if (m_translator.load(QString("plugin.%1").arg(locale), ":/ScipTerminalHelperPlugin")) {
        qApp->installTranslator(&m_translator);
    }
}

void ScipTerminalHelperPlugin::setDevice(UrgDevice* urg_)
{
    m_sensor = urg_;
}

void ScipTerminalHelperPlugin::sendText(const QString &term)
{
    if (ui->commandLine->text().isEmpty()) {
        QMessageBox::warning(this, QApplication::applicationName(),
                             tr("The command line is empty!"));
    }
    else {
        if (m_sensor && m_sensor->isConnected()) {
            string command = (ui->commandLine->text() + term).toStdString();
            Connection* con = m_sensor->connection();
            if(con) con->send(command.c_str(), command.size());
            updateText(QTime::currentTime(), m_txColor, QString::fromStdString(command));
            ui->commandLine->setText("");
        }
    }
}

void ScipTerminalHelperPlugin::recieveConnectionData(const QTime &time, const QByteArray &data)
{
    updateText(time, m_rxColor, data);
}

void ScipTerminalHelperPlugin::decodeSelection()
{
    QString selectedText = ui->console->textCursor().selectedText();
    if(selectedText.length() > 1 && selectedText.length() < 6){
        long decoded = m_sensor->decode(selectedText.toStdString(), selectedText.length());
        QPoint cursorPos = ui->console->viewport()->mapToGlobal(ui->console->cursorRect().center());
        QToolTip::showText(cursorPos, QString("%1 = %2").arg(selectedText).arg(decoded), ui->console, QRect(), 10000);
    }
    else{
        QToolTip::hideText();
    }
}

void ScipTerminalHelperPlugin::changeEvent(QEvent *event)
{
    HelperPluginInterface::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange: {
        if(ui) {
            ui->retranslateUi(this);
            ui->commandLine->setPlaceholderText(tr("Enter a command here!"));
        }
        setWindowTitle(pluginName());
    }break;
    default: {
    }
    }
}

void ScipTerminalHelperPlugin::showEvent(QShowEvent *event)
{
    HelperPluginInterface::showEvent(event);
    if(!m_receptionThread.isRunning()){
        m_receptionThread.run();
    }
    if(m_sensor){
        Connection* con = m_sensor->connection();
        if(con){
            QString title = tr("SCIP Terminal");
            setWindowTitle(QString("%1 (%2)").arg(title).arg(con->getDevice()));
        }
    }
}

void ScipTerminalHelperPlugin::hideEvent(QHideEvent *event)
{
    HelperPluginInterface::hideEvent(event);

    if(m_receptionThread.isRunning()){
        m_receptionThread.stop();
    }

    Connection* con = m_sensor->connection();
    con->send("QT\n", 3);
    ui->commandLine->clear();
    ui->console->clear();
}
int ScipTerminalHelperPlugin::historySize() const
{
    return m_historySize;
}

void ScipTerminalHelperPlugin::setHistorySize(int historySize)
{
    m_historySize = historySize;
    ui->console->setMaximumBlockCount(m_historySize * LINES_PER_COMMAND);
}

int ScipTerminalHelperPlugin::receptionThreadProcess(void *args)
{
    ScipTerminalHelperPlugin* obj = static_cast<ScipTerminalHelperPlugin*>(args);
    enum {BUFFERSIZE = 65535};
    char buffer[BUFFERSIZE];
    Connection* con = obj->m_sensor->connection();
    int count = 0;
    QTime timer;
    timer.start();
    bool lastDetection = false;
    bool currentDetection = false;
    bool receptionStarted = false;
    while(!obj->m_receptionThread.exitThread){
        int recieved = con->receive(&buffer[count], 1, 1);
        if(recieved > 0){
            if(!receptionStarted) timer.restart();
            receptionStarted = true;
            currentDetection = (buffer[count] == '\n');
            count++;
            if((timer.elapsed() > 25) || (currentDetection && lastDetection) || (count >= BUFFERSIZE -1)){
                buffer[count] = '\0';
                emit obj->connectionDataReady(QTime::currentTime(), QByteArray(buffer, count));
                count = 0;
                receptionStarted = false;
            }
            lastDetection = currentDetection;
        }
    }

    return 0;
}

QVariant ScipTerminalHelperPlugin::setDeviceMethod(const QVariantList &vars)
{
    if(vars.size() == 1){
        setDevice((UrgDevice*) vars[0].value<void *>());
    }
    return QVariant();
}

void ScipTerminalHelperPlugin::onLoad(PluginManagerInterface *manager)
{

}

void ScipTerminalHelperPlugin::onUnload()
{

}

void ScipTerminalHelperPlugin::updateText(const QTime &time, const QColor col, const QString &s)
{
    bool activateScroll = ui->console->verticalScrollBar()->value() == ui->console->verticalScrollBar()->maximum();
    QTextCharFormat tf;
    tf = ui->console->currentCharFormat();
    tf.setForeground(QBrush(col));
    ui->console->setCurrentCharFormat(tf);
    QString localString = s;
    localString.replace("\n", "⤶");
    localString.replace("\r", "¶");
    ui->console->appendPlainText(time.toString("HH:mm:ss.zzz") + ": " + localString);
    if(activateScroll){
        ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
    }
}

