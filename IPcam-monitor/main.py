import sys
from PyQt5.QtWidgets import (QApplication, QWidget, QGridLayout, QPushButton, QLabel, QMainWindow)
from PyQt5.QtGui import QPixmap, QImage, QFont
from PyQt5.QtCore import QThread, pyqtSignal, Qt, QCoreApplication
from PyQt5 import QtCore
from ping3 import ping
import cv2

class MyThread(QThread):
    image = pyqtSignal(QPixmap, int)
    noframe = pyqtSignal(int)
    def __init__(self, rtsp, ip, id):
        super().__init__()
        self.ip = ip
        self.rtsp = rtsp        
        self.id = id
        self.cap = cv2.VideoCapture()
        if ping(ip):
            self.cap.open(rtsp)

    def run(self):
        text = self.id.__str__()
        while True:
            if not self.cap.isOpened():
                
                if ping(self.ip):
                    print('open 1 ', self.rtsp)
                    self.cap.open(self.rtsp)
                    print('open 2 ', self.rtsp)
                QThread.sleep(1)
                continue

            ret, frame = self.cap.read()
            if not ret:
                QThread.sleep(1)
                print('no frame')
                self.cap.release()
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


class MyWidget(QWidget):
    def __init__(self, w, h):
        super().__init__()
        self.width = w
        self.height = h
        self.target_num = 0
        self.initUI()
        self.showFullScreen()

    def initUI(self):
        self.resize(self.width, self.height)
        layout = QGridLayout()
        self.setLayout(layout)

        self.image1 = QLabel('1', self)
        self.image1.setStyleSheet("color : white;background-color:rgba(0, 204, 102, 255)")
        self.image1.setFont(QFont('Arial', int(self.height/10)))
        self.image1.setAlignment(QtCore.Qt.AlignCenter)
        layout.addWidget(self.image1, 0, 0)
        self.image2 = QLabel('2', self)
        self.image2.setStyleSheet("color : white;background-color:rgba(0, 204, 102, 255)")
        self.image2.setFont(QFont('Arial', int(self.height/10)))
        self.image2.setAlignment(QtCore.Qt.AlignCenter)

        layout.addWidget(self.image2, 0, 1)
        self.image3 = QLabel('3', self)
        self.image3.setStyleSheet("color : white;background-color:rgba(0, 204, 102, 255)")
        self.image3.setFont(QFont('Arial', int(self.height/10)))
        self.image3.setAlignment(QtCore.Qt.AlignCenter)
        layout.addWidget(self.image3, 1, 0)
        self.image4 = QLabel('4', self)
        self.image4.setStyleSheet("color : white;background-color:rgba(0, 204, 102, 255)")
        self.image4.setFont(QFont('Arial', int(self.height/10)))
        self.image4.setAlignment(QtCore.Qt.AlignCenter)
        layout.addWidget(self.image4, 1, 1)
        layout.setSpacing(2)
        layout.setContentsMargins(0, 0, 0, 0)

        self.main_image = QLabel('Main', self)
        self.main_image.resize(self.width, self.height)
        self.main_image.move(0, 0)
        self.main_image.hide()
        
        self.monitor1 = MyThread('rtsp://test1234:test1234@192.168.1.108:554/stream1', '192.168.1.108', 1)
        self.monitor1.image.connect(self.setImage)
        self.monitor1.noframe.connect(self.noFrame)
        self.monitor1.start()

        self.monitor2 = MyThread('rtsp://192.168.1.106:8554/test', '192.168.1.106', 2)
        self.monitor2.image.connect(self.setImage)
        self.monitor2.noframe.connect(self.noFrame)
        self.monitor2.start()

        self.monitor3 = MyThread('rtsp://192.168.1.106:8554/test', '192.168.1.106', 3)
        self.monitor3.image.connect(self.setImage)
        self.monitor3.noframe.connect(self.noFrame)
        self.monitor3.start()

        self.monitor4 = MyThread('rtsp://test1234:test1234@192.168.1.108:554/stream1', '192.168.1.108', 4)
        self.monitor4.image.connect(self.setImage)
        self.monitor4.noframe.connect(self.noFrame)
        self.monitor4.start()

    def noFrame(self, id):

        display = self.image1
        if id == 2:
            display = self.image2
        elif id == 3:
            display = self.image3
        elif id == 4:
            display = self.image4

        display.clear()
        display.setText(id.__str__())
        display.setStyleSheet("color : white;background-color:rgba(0, 204, 102, 255)")

        
    def setImage(self, qImg, num):
        display = None
        w = int(self.width/2)
        h = int(self.height/2)
        if self.target_num > 0:
            if self.target_num != num:
                return
            
            w = self.width
            h = self.height
            display = self.main_image
        else:
            if num == 1:
                display = self.image1
            elif num == 2:
                display = self.image2
            elif num == 3:
                display = self.image3
            elif num == 4:
                display = self.image4

        display.setPixmap(qImg.scaled(w, h, Qt.KeepAspectRatio, Qt.SmoothTransformation))

    def keyPressEvent(self, e):
        if e.key() == Qt.Key_1:
            self.main_image.show()
            self.isFocus = True
            self.target_num = 1
        elif e.key() == Qt.Key_3:
            self.main_image.show()
            self.isFocus = True
            self.target_num = 3
        elif e.key() == Qt.Key_4:
            self.main_image.show()
            self.isFocus = True
            self.target_num = 4
        elif e.key() == Qt.Key_L:
            self.main_image.hide()
            self.isFocus = False
            self.target_num = 0
        elif e.key() == Qt.Key_Space:
            QCoreApplication.quit()

        

if __name__ == '__main__':
    app = QApplication(sys.argv)
    screen_size = app.desktop().screenGeometry()
    w = MyWidget(screen_size.width(), screen_size.height())
    w.show()
    sys.exit(app.exec_())
