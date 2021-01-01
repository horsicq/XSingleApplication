// copyright (c) 2020 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "xsingleapplication.h"

XSingleApplication::XSingleApplication(int &argc, char *argv[], bool bIsSingle) : QApplication(argc,argv)
{
    g_pSharedMemory=nullptr;
    g_bIsPrimary=false;
    g_pLocalServer=0;
    g_pLocalSocket=0;

    QString sApplicationID=sGetApplicationID();

#ifndef Q_OS_WINDOWS
    // Cleanup for unix after crash
    // https://doc.qt.io/qt-5/qsharedmemory.html
    g_pSharedMemory=new QSharedMemory(sApplicationID);
    g_pSharedMemory->attach();
    cleanUp();
#endif

    if(bIsSingle)
    {
        g_pSharedMemory=new QSharedMemory(sApplicationID);

        if(g_pSharedMemory->attach())
        {
            qDebug("Instance!!!");
            // TODO Serialize
            g_pLocalSocket=new QLocalSocket();
            g_pLocalSocket->connectToServer(sApplicationID);
            g_pLocalSocket->waitForConnected();
            g_pLocalSocket->write("TestConnection"); // TODO
            g_pLocalSocket->waitForBytesWritten();
            g_pLocalSocket->flush();

            qDebug("END");

            cleanUp();
        }
        else
        {
            g_pSharedMemory->create(0x1000);
            g_bIsPrimary=true;

            QLocalServer::removeServer(sApplicationID);
            g_pLocalServer=new QLocalServer();
            g_pLocalServer->setSocketOptions(QLocalServer::UserAccessOption);
            g_pLocalServer->listen(sApplicationID);
            connect(g_pLocalServer,SIGNAL(newConnection()),this,SLOT(serverConnection()));
        }
    }
    else
    {
        g_bIsPrimary=true;
    }
}

XSingleApplication::~XSingleApplication()
{
    cleanUp();
}

bool XSingleApplication::isPrimary()
{
    return g_bIsPrimary;
}

QString XSingleApplication::getUser()
{
    QString sResult;

#ifdef Q_OS_WINDOWS
    sResult=qgetenv("USERNAME");
#else
    sResult=qgetenv("USER");
#endif

    return sResult;
}

QString XSingleApplication::sGetApplicationID()
{
    QString sString=QString("%1|%2|%3").arg(QCoreApplication::organizationName()).arg(QCoreApplication::applicationName()).arg(getUser());

    QCryptographicHash cryptoHash(QCryptographicHash::Md5);
    cryptoHash.addData(sString.toUtf8());

    return cryptoHash.result().toHex();
}

void XSingleApplication::cleanUp()
{
    if(g_pSharedMemory)
    {
        delete g_pSharedMemory;
        g_pSharedMemory=nullptr;
    }

    if(g_pLocalServer)
    {
        g_pLocalServer->close();
        delete g_pLocalServer;
    }

    if(g_pLocalSocket)
    {
        g_pLocalSocket->close();
        delete g_pLocalSocket;
    }
}

void XSingleApplication::serverConnection()
{
    qDebug("void XSingleApplication::serverConnection()");
}
