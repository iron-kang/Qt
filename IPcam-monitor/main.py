import sys, os
from PyQt5.QtWidgets import (QApplication, QWidget, QGridLayout, QPushButton, QLabel, QMainWindow, QDialog, QMessageBox)
from PyQt5.QtGui import QPixmap, QImage, QFont
from PyQt5.QtCore import QThread, pyqtSignal, Qt, QCoreApplication, QFile
from PyQt5 import QtCore, uic
from PyQt5.QtSql import QSqlDatabase, QSqlQuery, QSqlTableModel
from ping3 import ping
import cv2, re

path = os.path.dirname(__file__)
qtCreatorFile = "channel_edit.ui"
Ui_Dialog, _ = uic.loadUiType(os.path.join(path,qtCreatorFile))

class ChannelEdit(QDialog, Ui_Dialog):
    def __init__(self, list):
        super(ChannelEdit, self).__init__()
        self.setupUi(self)
        self.setWindowTitle('Cannel edit')
        self.setWindowFlags(Qt.CustomizeWindowHint)
        self.btn_ok.clicked.connect(self.ok)
        self.btn_cancel.clicked.connect(self.cancel)
        self.ed_channels = [ self.ed_channel1, self.ed_channel2, self.ed_channel3, self.ed_channel4 ]
        self.result = []
        for id, value in enumerate(list):
            self.ed_channels[id].setText(value) 

    def ok(self):
        self.result = [ self.ed_channel1.text(), self.ed_channel2.text(), self.ed_channel3.text(), self.ed_channel4.text() ]
        self.close()

    def cancel(self):
        self.close()

    def getResults(self):
        return self.result
        

class MyThread(QThread):
    image = pyqtSignal(QPixmap, int)
    noframe = pyqtSignal(int)
    def __init__(self, id):
        super().__init__()
        self.id = id

    def open(self, address, ip):
        self.address = address
        self.ip = ip        
        
        self.run = True
        if address == '1':
            print('open web cam................', address)
            self.cap = cv2.VideoCapture(int(address))
        elif address == '0':
            print('open CSI cam................')
            self.cap = cv2.VideoCapture('nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)3280, height=(int)2464, format=(string)NV12, framerate=(fraction)21/1 ! nvvidconv flip-method=0 ! video/x-raw, width=(int)820, height=(int)616, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsin', cv2.WINDOW_AUTOSIZE)
        else:
            self.cap = cv2.VideoCapture()
        if ping(ip):
            if id == '1' or id == '4':
                self.cap.open(address)

    def stop(self):
        self.run = False 
        #QThread.sleep(2)
        

    def run(self):
        text = self.id.__str__()
        while self.run:
            if not self.cap.isOpened():
                if ping(self.ip):
                    self.cap.open(self.address)
                QThread.sleep(1)
                continue

            ret, frame = self.cap.read()
            if not ret:
                QThread.sleep(1)
                print('no frame')
                self.noframe.emit(self.id)
                continue
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            height, width, channel = frame.shape
            cv2.putText(frame, text, (width-50, height-int(50/2)), cv2.FONT_HERSHEY_SIMPLEX, 2, (255, 255, 255), 3, cv2.LINE_AA)
            if self.id == 4:
                cv2.line(frame, (int(width/2)-25, int(height/2)), (int(width/2)+25, int(height/2)), (255, 0, 0), 5)
                cv2.line(frame, (int(width/2), int(height/2)-25), (int(width/2), int(height/2)+25), (255, 0, 0), 5)
            bytesPerLine = 3 * width
            qImg = QImage(frame.data, width, height, bytesPerLine, QImage.Format_RGB888)
            pixmap = QPixmap.fromImage(qImg)
            self.image.emit(pixmap, self.id)
        
        self.cap.release()
        self.noframe.emit(self.id)


class MyWidget(QWidget):
    def __init__(self, w, h):
        super().__init__()
        self.width = w
        self.height = h
        self.target_id = 0
        self.address = []
        self.ip = []
        self.monitors = []
        self.initUI()
        self.showFullScreen()
        self.db = QSqlDatabase.addDatabase('QSQLITE')
        
        self.db.setDatabaseName('channel.db')
        self.query = None
        if not self.db.open():
            print('db open fail')
            QMessageBox.critical(None, ("Warning"), ("Can't create database conection"), QMessageBox.Cancel)
        else:
            self.query = QSqlQuery()
            self.query.exec_("create table channel(id int primary key, name varchar(256))")
            self.query.exec_("INSERT INTO channel VALUES(1, '')")
            self.query.exec_("INSERT INTO channel VALUES(2, '')")
            self.query.exec_("INSERT INTO channel VALUES(3, '')")
            self.query.exec_("INSERT INTO channel VALUES(4, '')")
            self.query.exec_("select id, name from channel")
            while self.query.next():
                self.address.append(self.query.value(1))
                ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', self.query.value(1))
                if len(ip) != 0:
                    self.ip.append(ip[0])
                else:
                    self.ip.append(ip)
        self.createThread()

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
            monitor = MyThread(id+1)
            monitor.image.connect(self.setImage)
            monitor.noframe.connect(self.noFrame)
            self.monitors.append(monitor)
            if len(ip) != 0:
                monitor.open(self.address[id], ip)
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

    def changeAddress(self, channels):
        if len(channels):
            
            for id, value in enumerate(channels):
                self.query.prepare("UPDATE channel SET name=:name where id=:id")
                self.query.bindValue(":id", id+1)
                self.query.bindValue(":name", value)
                self.query.exec()
                ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', value)
                if len(ip) != 0:
                    
                    if self.address[id] != value:
                        self.ip[id] = ip[0]
                        self.monitors[id].stop()
                        self.monitors[id].open(value, ip[0])
                        self.monitors[id].start()
                        print('run: ', value, ip[0])
                else:
                    self.ip[id] = ''
                    self.monitors[id].stop()
                    print('stop monitor ', id+1)
                
            self.address = channels

    def keyPressEvent(self, e):
        if e.key() == Qt.Key_1:
            self.images[0].show()
            self.isFocus = True
            self.target_id = 1
        elif e.key() == Qt.Key_3:
            self.images[0].show()
            self.isFocus = True
            self.target_id = 3
        elif e.key() == Qt.Key_4:
            self.images[0].show()
            self.isFocus = True
            self.target_id = 4
        elif e.key() == Qt.Key_L:
            self.images[0].hide()
            self.isFocus = False
            self.target_id = 0
        elif e.key() == Qt.Key_Space:
            self.db.close()
            QCoreApplication.quit()
        elif e.key() == Qt.Key_Q:
            channel_edit = ChannelEdit(self.address)
            channel_edit.exec()
            ret = channel_edit.getResults()
            self.changeAddress(ret)
        

if __name__ == '__main__':
    app = QApplication(sys.argv)
    screen_size = app.desktop().screenGeometry()
    w = MyWidget(screen_size.width(), screen_size.height())
    w.show()
    sys.exit(app.exec_())
