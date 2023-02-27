import sys, os, re
from PyQt5.QtWidgets import (QApplication, QWidget, QGridLayout, QLabel)
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt, QCoreApplication, QThread
from PyQt5 import QtCore
from channelEdit import ChannelEdit
from captureThread import CaptureThread
import sqlite3 as sql

class MainWindow(QWidget):
    def __init__(self, w, h):
        super().__init__()
        self.width = w
        self.height = h
        self.local_ip = ''
        self.target_id = 0
        self.address = []
        self.ip = []
        self.monitors = []
        self.initUI()
        self.showFullScreen()
        self.db = sql.connect('channel.db')
        self.cursor = self.db.cursor()
        try:
            self.cursor.execute("create table channel(id int primary key, name varchar(256))")
            self.cursor.execute("INSERT INTO channel VALUES(1, '')")
            self.cursor.execute("INSERT INTO channel VALUES(2, '')")
            self.cursor.execute("INSERT INTO channel VALUES(3, '')")
            self.cursor.execute("INSERT INTO channel VALUES(4, '')")
            self.cursor.execute("INSERT INTO channel VALUES(5, '')")
        except Exception as err:
            print('sqlite except: ', err)
            exc = self.cursor.execute("select id, name from channel")

            for i, val in enumerate(exc):
                print('id: ', val[0])
                print('name: ', val[1])
                str = val[1]
                if str.find('GigE') > 0:
                    str = 'GigE'
                ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', val[1])

                if len(ip) != 0:
                    if i == 4:
                        self.setLocalIP(ip[0])
                        break
                    else:
                        self.ip.append(ip[0])
                else:
                    if i == 4:
                        break
                    self.ip.append(ip)
                self.address.append(str)

        print('address: ', self.address)
        print('ip: ', self.ip)
        self.createThread()

    def setLocalIP(self, ip):
        self.local_ip = ip
        cmd = 'ifconfig eth0 ' + self.local_ip + ' netmask 255.255.255.0'
        os.system(cmd)

    def initUI(self):
        self.resize(self.width, self.height)
        layout = QGridLayout()
        self.setLayout(layout)

        self.image1 = QLabel('1', self)
        self.image2 = QLabel('2', self)
        self.image3 = QLabel('3', self)
        self.image4 = QLabel('4', self)
        self.main_image = QLabel('', self)
        self.main_image.resize(self.width, self.height)
        self.main_image.move(0, 0)
        self.main_image.hide()
        self.images = [ self.main_image, self.image1, self.image2, self.image3, self.image4 ]

        for image in self.images:
            image.setStyleSheet("color : white;background-color:rgba(0, 204, 102, 255)")
            image.setFont(QFont('Arial', int(self.height/10)))
            image.setAlignment(QtCore.Qt.AlignCenter)

        layout.addWidget(self.images[1], 0, 0)
        layout.addWidget(self.images[2], 0, 1)
        layout.addWidget(self.images[3], 1, 0)
        layout.addWidget(self.images[4], 1, 1)

        layout.setSpacing(2)
        layout.setContentsMargins(0, 0, 0, 0)

    def createThread(self):
        for id, ip in enumerate(self.ip):
            monitor = CaptureThread(id+1, self.address[id], ip)
            monitor.image.connect(self.setImage)
            monitor.noframe.connect(self.noFrame)
            self.monitors.append(monitor)
            print("monitor: ", self.address[id])
            if len(ip) != 0:
                monitor.start()

    def noFrame(self, id):
        self.images[id].clear()
        self.images[id].setText(id.__str__())
        self.images[id].setStyleSheet("color : white;background-color:rgba(0, 204, 102, 255)")

    def setImage(self, qImg, id):
        display = None
        w = int(self.width/2)
        h = int(self.height/2)
        if self.target_id > 0:
            if self.target_id != id:
                return

            w = self.width
            h = self.height
            display = self.main_image
        else:
            display = self.images[id]

        display.setStyleSheet("background-color:rgba(0, 0, 0, 255)")
        display.setPixmap(qImg.scaled(w, h, Qt.KeepAspectRatio, Qt.SmoothTransformation))

    def changeAddress(self, address, local_ip):
        cmd = "UPDATE channel set name = ? where ID = ?"
        if local_ip != self.local_ip:
            ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', local_ip)
            if len(ip) != 0:
                self.setLocalIP(ip[0])
                data=(local_ip, 5)
                self.cursor.execute(cmd, data)
                self.db.commit()

        if len(address):
            ready_monitors = []
            for i, value in enumerate(address):
                data=(value, id+1)
                self.cursor.execute(cmd, data)
                self.db.commit()
                ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', value)
                if len(ip) != 0:
                    if self.address[i] != value:
                        self.ip[i] = ip[0]
                        self.monitors[i].stop()
                        self.monitors[i].setIPAddress(value, ip[0])
                        ready_monitors.append(self.monitors[i])

                else:
                    self.ip[id] = ''
                    self.monitors[i].stop()
                    print('stop monitor ', i+1)
            QThread.msleep(100)
            for monitor in ready_monitors:
                monitor.start()

            self.address = address
            print(self.address)
            print(self.ip)

    def keyPressEvent(self, e):
        if e.key() == Qt.Key_1:
            self.images[0].show()
            self.target_id = 1
        elif e.key() == Qt.Key_3:
            self.images[0].show()
            self.target_id = 3
        elif e.key() == Qt.Key_4:
            self.images[0].show()
            self.target_id = 4
        elif e.key() == Qt.Key_O:
            self.images[0].hide()
            self.target_id = 0
        elif e.key() == Qt.Key_Space:
            self.db.close()
            for monitor in self.monitors:
                monitor.stop()
                monitor.wait()
            QCoreApplication.quit()
        elif e.key() == Qt.Key_Q:
            print('address: ', self.address)
            print('local ip: ', self.local_ip)
            channel_edit = ChannelEdit(self.address, self.local_ip)
            channel_edit.exec()
            address, local_ip = channel_edit.get_results()
            self.changeAddress(address, local_ip)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    screen_size = app.desktop().screenGeometry()
    w = MainWindow(screen_size.width(), screen_size.height())
    w.show()
    sys.exit(app.exec_())
