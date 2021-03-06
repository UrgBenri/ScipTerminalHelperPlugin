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

#include "CommandLine.h"

#include <QDebug>
#include <QAbstractItemView>

CommandLine::CommandLine(QWidget *parent)
    : QLineEdit(parent)
    , m_completer(new QCompleter(this))
    , m_model(new QStringListModel(this))
{
    m_completer->setModel(m_model);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    setCompleter(m_completer);
}

void CommandLine::keyPressEvent (QKeyEvent *event)
{
    if((event->key() == Qt::Key_Return) ||
            (event->key() == Qt::Key_Enter)){
        QStringList list = m_model->stringList();
        QString testData = text();
        if(!list.contains(testData)){
            list << text();
            m_model->setStringList(list);
        }
    }
    if((event->key() == Qt::Key_Up) || (event->key() == Qt::Key_Down)){
        emit textChanged(text());
    }

    if ((event->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)) &&
        event->key() == Qt::Key_Z) {
        event->ignore();
    }else {
        QLineEdit::keyPressEvent(event);
    }
}

