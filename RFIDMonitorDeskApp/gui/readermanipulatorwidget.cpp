/****************************************************************************
**
** www.celtab.org.br
**
** Copyright (C) 2013
**                     Gustavo Valiati <gustavovaliati@gmail.com>
**                     Luis Valdes <luisvaldes88@gmail.com>
**                     Thiago R. M. Bitencourt <thiago.mbitencourt@gmail.com>
**
** This file is part of the RFIDMonitor project
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; version 2
** of the License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
****************************************************************************/

#include <QFileDialog>
#include <QDir>

#include "readermanipulatorwidget.h"
#include "ui_readermanipulatorwidget.h"
#include "systemmessageswidget.h"

ReaderManipulatorWidget::ReaderManipulatorWidget(const Settings::ConnectionType type, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReaderManipulatorWidget),
    m_connectionType(type)
{
    ui->setupUi(this);

    ui->btStartPauseReading->setIcon(QIcon(":/icons/icon-ok"));
    ui->btClearOutput->setIcon(QIcon(":/icons/icon-clear"));
    ui->btSendCommand->setIcon(QIcon(":/icons/icon-send"));
    ui->btLogTo->setIcon(QIcon(":/icons/icon-search"));

    // Log file.
    m_logFile = new QFile(this);
    m_useLogFile = false;

    // Define the default window state.
    ui->btLogTo->setEnabled(true);
    ui->cbLogType->setEnabled(true);
    ui->btSendCommand->setEnabled(false);
    ui->leCommand->setEnabled(false);
    ui->cbLogType->addItem(tr("Append"), QIODevice::Append);
    ui->cbLogType->addItem(tr("Overwrite"), QIODevice::WriteOnly);
    ui->cbInputType->addItem("ASCII", SerialCommunication::KASCII);
    ui->cbInputType->addItem(tr("Number"), SerialCommunication::KNumber);

    connect(ui->btSendCommand, SIGNAL(clicked()), this, SLOT(btSendCommandClicked()));
    connect(ui->leCommand, SIGNAL(returnPressed()), this, SLOT(leCommandReturnPressed()));
    connect(ui->btStartPauseReading, SIGNAL(clicked(bool)), this, SLOT(btStartPauseReadingClicked(bool)));
    connect(ui->btLogTo, SIGNAL(clicked()), this, SLOT(btLogToClicked()));
    connect(ui->btClearOutput, SIGNAL(clicked()), this, SLOT(btClearOutputClicked()));

    if(type == Settings::KNetwork){
        connect(NetworkCommunication::instance(),SIGNAL(connectionFailed()),this, SLOT(connectionFinished()));
    }else{
        connect(SerialCommunication::instance(),SIGNAL(connectionFailed()),this, SLOT(connectionFinished()));
    }

    // instantiate the RI-CTL-MB2B-30 Manipulator and add it to the main tab.
    m_mb2b30 = new RICTLMB2B30Widget(m_connectionType, this);
    ui->tabWidget->addTab(m_mb2b30, "RI-CTL-MB2B-30");
}

ReaderManipulatorWidget::~ReaderManipulatorWidget()
{
    //When this Manipulator is ordered to close, close the log file first.
    if(m_logFile->isOpen())
        m_logFile->close();

    delete ui;
}

void ReaderManipulatorWidget::closeConnection()
{
    if(m_connectionType == Settings::KSerial){
        SerialCommunication::instance()->disconnectFromDevice();
    }
    else if(m_connectionType == Settings::KNetwork){
        NetworkCommunication::instance()->closeTCPConnection();
    }
}

void ReaderManipulatorWidget::sendCommand(const QString &command)
{
    if( ! command.isEmpty()){

        ui->leCommand->clear();

        if(m_connectionType == Settings::KSerial){
            if(SerialCommunication::instance()->sendCommand(
                        command,
                        (SerialCommunication::CommandType) ui->cbInputType->currentData().toInt())
              )
                    writeToOutput(tr("Command sent to device: %1").arg(command));

        }
        else if(m_connectionType == Settings::KNetwork){
            NetworkCommunication::instance()->sendNewCommandToReader(command);
        }
    }
}

void ReaderManipulatorWidget::writeToOutput(const QString &text)
{
    ui->teOutput->append(text);

    if(m_useLogFile && m_logFile->isOpen()){
        QTextStream logStream(m_logFile);
        logStream << text << QString("\r");
        logStream.flush();
    }
}

void ReaderManipulatorWidget::lockForms()
{
    ui->leCommand->setEnabled(false);
    ui->btSendCommand->setEnabled(false);
    ui->cbInputType->setEnabled(false);
    ui->btStartPauseReading->setEnabled(false);
    ui->btLogTo->setEnabled(false);
    ui->cbLogType->setEnabled(false);
    ui->btClearOutput->setEnabled(false);
}

void ReaderManipulatorWidget::newAnswerFromSerialComm(const QString answer)
{
    writeToOutput(answer);
}

void ReaderManipulatorWidget::newAnswerFromNetworkComm(const QString answer)
{
    writeToOutput(answer);
}

void ReaderManipulatorWidget::btSendCommandClicked()
{
    sendCommand(ui->leCommand->text());
}

void ReaderManipulatorWidget::leCommandReturnPressed()
{
    sendCommand(ui->leCommand->text());
}

void ReaderManipulatorWidget::btClearOutputClicked()
{
    ui->teOutput->clear();
}

void ReaderManipulatorWidget::btLogToClicked()
{
    QString fileName(QFileDialog::getOpenFileName(this, tr("Select log file"), QDir::homePath()));
    if(!fileName.isEmpty()){
        // If a file was selected.


        if(m_logFile->isOpen())
            m_logFile->close();

        // Test if the log file can be used.
        m_logFile->setFileName(fileName);
        if(m_logFile->open(QIODevice::Append)){

            // The log file can be used.

            m_logFile->close();
            m_useLogFile = true;
            SystemMessagesWidget::instance()->writeMessage(tr("The selected log file is good."));
        }else{

            // The log file cannot be used.

            m_useLogFile = false;
            SystemMessagesWidget::instance()->writeMessage(tr("Cannot use the selected log file. It is not writable"));
        }
        ui->leLogFile->setText(fileName);

    }else{

        // No file was selected.

        ui->leLogFile->setText("");
        m_useLogFile = false;
    }
}

void ReaderManipulatorWidget::btStartPauseReadingClicked(const bool checked)
{
    if(checked){
        // Start reading selected.



        ui->btStartPauseReading->setIcon(QIcon(":/icons/icon-cancel"));

        // Try to open the log file to use if must use it.
        if(m_useLogFile){
            if( ! m_logFile->open((QIODevice::OpenModeFlag)ui->cbLogType->currentData().toInt())){
                m_useLogFile = false;
                SystemMessagesWidget::instance()->writeMessage(tr("Problem with the log file. It is not writable anymore, "
                                                                  "and is not going to be used."));
            }
        }

        ui->btStartPauseReading->setText(tr("Pause"));
        ui->btLogTo->setEnabled(false);
        ui->cbLogType->setEnabled(false);
        ui->btSendCommand->setEnabled(true);
        ui->leCommand->setEnabled(true);


        // Connect the signal of new messagens from connections, to display them in the QTextEdit and in the log file.
        if(m_connectionType == Settings::KSerial){
            connect(SerialCommunication::instance(), SIGNAL(newAnswer(QString)), this, SLOT(newAnswerFromSerialComm(QString)));
        }
        else if(m_connectionType == Settings::KNetwork){
            NetworkCommunication::instance()->sendFullRead(true);
            connect(NetworkCommunication::instance(), SIGNAL(newReaderAnswer(QString)), this, SLOT(newAnswerFromNetworkComm(QString)));
        }

    }else{
        // Pause reading selected.


        ui->btStartPauseReading->setIcon(QIcon(":/icons/icon-ok"));

        if(m_logFile->isOpen())
            m_logFile->close();

        ui->btStartPauseReading->setText(tr("Start"));
        ui->btLogTo->setEnabled(true);
        ui->cbLogType->setEnabled(true);
        ui->btSendCommand->setEnabled(false);
        ui->leCommand->setEnabled(false);

        // Disconnect the signals to stop the output of answers.
        if(m_connectionType == Settings::KSerial){
            disconnect(SerialCommunication::instance(), SIGNAL(newAnswer(QString)), this, SLOT(newAnswerFromSerialComm(QString)));
        }
        else if(m_connectionType == Settings::KNetwork){
            NetworkCommunication::instance()->sendFullRead(false);
            disconnect(NetworkCommunication::instance(), SIGNAL(newReaderAnswer(QString)), this, SLOT(newAnswerFromNetworkComm(QString)));
        }
    }

}

void ReaderManipulatorWidget::connectionFinished()
{
    lockForms();
}
