from PySide6.QtWidgets import QDialog
from PySide6.QtCore import Qt
from ui_channel_edit import Ui_ChannelEidt
import os

class ChannelEdit(QDialog):
    """
    edit camera address and local ip
    """
    def __init__(self, address, local_ip, parent=None):
        super(ChannelEdit, self).__init__(parent=parent)
        self.ui = Ui_ChannelEidt()
        self.ui.setupUi(self)
        self.setWindowFlags(Qt.CustomizeWindowHint)
        self.ui.btn_ok.clicked.connect(self.ok_ops)
        self.ui.btn_cancel.clicked.connect(self.cancel_ops)
        self.ui.ed_channels = [ self.ui.ed_channel1, self.ui.ed_channel2, 
                            self.ui.ed_channel3, self.ui.ed_channel4 ]
        self.address = []
        self.local_ip = ''
        self.ui.ed_local_ip.setText(local_ip)
        for i, value in enumerate(address):
            self.ui.ed_channels[i].setText(value)

    def ok_ops(self):
        """
        ok button behavior
        """
        self.address = [ self.ui.ed_channel1.text(), self.ui.ed_channel2.text(), 
                        self.ui.ed_channel3.text(), self.ui.ed_channel4.text() ]
        self.local_ip = self.ui.ed_local_ip.text()
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
    