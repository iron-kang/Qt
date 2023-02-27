from PyQt5.QtWidgets import QDialog
from PyQt5 import uic
from PyQt5.QtCore import Qt
import os
# pylint: disable=C0103
path = os.path.dirname(__file__)
qtCreatorFile = "channel_edit.ui"
Ui_Dialog, _ = uic.loadUiType(os.path.join(path,qtCreatorFile))

class ChannelEdit(QDialog, Ui_Dialog):
    """
    edit camera address and local ip
    """
    def __init__(self, address, local_ip):
        super(ChannelEdit, self).__init__()
        self.setupUi(self)
        self.setWindowFlags(Qt.CustomizeWindowHint)
        self.btn_ok.clicked.connect(self.ok)
        self.btn_cancel.clicked.connect(self.cancel)
        self.ed_channels = [ self.ed_channel1, self.ed_channel2, 
                            self.ed_channel3, self.ed_channel4 ]
        self.address = []
        self.local_ip = ''
        self.ed_local_ip.setText(local_ip)
        for i, value in enumerate(address):
            self.ed_channels[i].setText(value)

    def ok_ops(self):
        """
        ok button behavior
        """
        self.address = [ self.ed_channel1.text(), self.ed_channel2.text(), 
                        self.ed_channel3.text(), self.ed_channel4.text() ]
        self.local_ip = self.ed_local_ip.text()
        self.close()

    def cancel_ops(self):
        """
        cancel_ops button behavior
        """
        self.close()

    def get_results(self):
        """
        get camera address and local ip
        """
        return self.address, self.local_ip
    