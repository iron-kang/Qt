import sys, os
from PyQt5.QtWidgets import (QApplication, QWidget, QGridLayout, QPushButton, QLabel, QMainWindow, QDialog, QMessageBox)
from PyQt5.QtGui import QPixmap, QImage, QFont
from PyQt5.QtCore import QThread, pyqtSignal, Qt, QCoreApplication
from PyQt5 import QtCore, uic
from ping3 import ping
#from PySpinCam import PySpinCam
import cv2, re
import sqlite3 as sql

path = os.path.dirname(__file__)
qtCreatorFile = "channel_edit.ui"
Ui_Dialog, _ = uic.loadUiType(os.path.join(path,qtCreatorFile))

class ChannelEdit(QDialog, Ui_Dialog):
    def __init__(self, list, local_ip):
        super(ChannelEdit, self).__init__()
        self.setupUi(self)
        self.setWindowFlags(Qt.CustomizeWindowHint)
        self.btn_ok.clicked.connect(self.ok)
        self.btn_cancel.clicked.connect(self.cancel)
        self.ed_channels = [ self.ed_channel1, self.ed_channel2, self.ed_channel3, self.ed_channel4 ]
        self.address = []
        self.local_IP = ''
        self.ed_local_ip.setText(local_ip)
        for id, value in enumerate(list):
            self.ed_channels[id].setText(value) 

    def ok(self):
        self.address = [ self.ed_channel1.text(), self.ed_channel2.text(), self.ed_channel3.text(), self.ed_channel4.text() ]
        self.local_IP = self.ed_local_ip.text()
        self.close()

    def cancel(self):
        self.close()

    def getResults(self):
        return self.address, self.local_IP
        

class MyThread(QThread):
    image = pyqtSignal(QPixmap, int)
    noframe = pyqtSignal(int)
    def __init__(self, id, address, ip):
        super().__init__()
        self.id = id
        self.isOpened = False
        self.address = address
        self.ip = ip        
        self.isGigE = False
        self.run = True
        print(address, ip)
        self.setIPAddress(address, ip)
        if address.find('GigE') != -1:
            self.isGigE = True
        else:
            self.cap = cv2.VideoCapture()
        # if len(ip) != 0 and ping(ip):
        #     self.open()

    def setIPAddress(self, address, ip):
        self.address = address
        self.ip = ip
        if address.find('GigE') != -1:
            self.isGigE = True
        else:
            self.isGigE = False

    def open(self):
        if self.isGigE:
            print('PySpinCam init')
            self.GigECam = PySpinCam()
            print('gige setup')
            if self.GigECam.setup() == False:
                return
            print('gige begin')
            self.GigECam.begin()
            self.isOpened = True
            # self.setPriority(6)
            print('PySpinCam init finish')
        else:
            print('rtsp init: ', self.address)
            self.cap.open(self.address)
            print('rtsp open...')
            self.isOpened = self.cap.isOpened()
            print('rtsp init finish')


    def stop(self):
        self.run = False 

    def run(self):
        text = self.id.__str__()
        self.run = True
        
        print('run: ', self.address, self.id)
        while self.run:
            if not self.isOpened:
                try: 
                    if ping(self.ip):
                        self.open()
                        print('open device....')
                except:
                    QThread.msleep(200)
                QThread.msleep(500)
                continue

            if self.isGigE:
                ret, frame = self.GigECam.capture()
            else:
                ret, frame = self.cap.read()
            
            if not ret:
                if self.isGigE:
                    self.GigECam.end()                          
                else:
                    self.cap.release()
                self.isOpened = False  
                self.noframe.emit(self.id)
                continue

            if not self.isGigE:
                frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

            height, width, channel = frame.shape    
            if self.isGigE:
                cv2.putText(frame, text, (width-50, height-int(50/2)), cv2.FONT_HERSHEY_SIMPLEX, 2, (255, 255, 255), 3, cv2.LINE_AA)
                cv2.line(frame, (int(width/2)-25, int(height/2)), (int(width/2)+25, int(height/2)), (255, 0, 0), 5)
                cv2.line(frame, (int(width/2), int(height/2)-25), (int(width/2), int(height/2)+25), (255, 0, 0), 5)

            pixmap = QPixmap.fromImage(QImage(frame.data, width, height, 3 * width, QImage.Format_RGB888))
            self.image.emit(pixmap, self.id)
            QThread.msleep(30)
        
        if self.isGigE:
            self.GigECam.end()
        else:
            self.cap.release()
        
        self.isOpened = False
        self.noframe.emit(self.id)
        print('exit: ', self.id)


class MainWindow(QWidget):
    def __init__(self, w, h):
        super().__init__()
        self.width = w
        self.height = h
        self.local_IP = ''
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
            # c.execute("UPDATE channel set name = '192.168.1.1' where ID=5")
            # conn.commit()
            exc = self.cursor.execute("select id, name from channel")

            for id, val in enumerate(exc):
                print('id: ', val[0])
                print('name: ', val[1])
                str = val[1]
                if str.find('GigE') > 0:
                    str = 'GigE'
                ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', val[1])
                
                if len(ip) != 0:
                    if id == 4:
                        self.setLocalIP(ip[0])
                        break
                    else:
                        self.ip.append(ip[0])
                else:
                    if id == 4:
                        break
                    self.ip.append(ip)
                self.address.append(str)

        print('address: ', self.address)
        print('ip: ', self.ip)
        self.createThread()

    def setLocalIP(self, ip):
        self.local_IP = ip
        cmd = 'ifconfig eth0 ' + self.local_IP + ' netmask 255.255.255.0'
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
            monitor = MyThread(id+1, self.address[id], ip)
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
        if local_ip != self.local_IP:
            ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', local_ip)
            if len(ip) != 0:
                self.setLocalIP(ip[0])
                data=(local_ip, 5)
                self.cursor.execute(cmd, data)
                self.db.commit()

        if len(address):
            ready_monitors = []
            for id, value in enumerate(address):
                data=(value, id+1)
                self.cursor.execute(cmd, data)
                self.db.commit()
                ip = re.findall(r'[0-9]+(?:\.[0-9]+){3}', value)
                if len(ip) != 0:
                    if self.address[id] != value:
                        self.ip[id] = ip[0]
                        self.monitors[id].stop()
                        self.monitors[id].setIPAddress(value, ip[0])
                        ready_monitors.append(self.monitors[id])
                        
                else:
                    self.ip[id] = ''
                    self.monitors[id].stop()
                    print('stop monitor ', id+1)
            QThread.msleep(100)
            for monitor in ready_monitors:
                # monitor.open()
                monitor.start()
                
            self.address = address
            print(self.address)
            print(self.ip)

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
        elif e.key() == Qt.Key_O:
            self.images[0].hide()
            self.isFocus = False
            self.target_id = 0
        elif e.key() == Qt.Key_Space:
            self.db.close()
            for monitor in self.monitors:
                monitor.stop()
                monitor.wait()
            QCoreApplication.quit()
        elif e.key() == Qt.Key_Q:
            print('address: ', self.address)
            print('local ip: ', self.local_IP)
            channel_edit = ChannelEdit(self.address, self.local_IP)
            channel_edit.exec()
            address, local_ip = channel_edit.getResults()
            self.changeAddress(address, local_ip)
        

if __name__ == '__main__':
    app = QApplication(sys.argv)
    screen_size = app.desktop().screenGeometry()
    w = MainWindow(screen_size.width(), screen_size.height())
    w.show()
    sys.exit(app.exec_())
