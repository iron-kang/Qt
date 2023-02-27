from PyQt5.QtCore import QThread, pyqtSignal
from PyQt5.QtGui import QPixmap, QImage
from ping3 import ping
from PySpinCam import PySpinCam
import cv2

class CaptureThread(QThread):
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
            self.GigECam.release()
        else:
            self.cap.release()
        
        self.isOpened = False
        self.noframe.emit(self.id)
        print('exit: ', self.id)