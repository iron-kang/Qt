import PySpin
import cv2
import numpy as np

class PySpinCam:
    def __init__(self):
        self.cam = None
        self.system = PySpin.System.GetInstance()
        self.img = np.zeros((1280, 720, 3), np.uint8)            

    def setup(self):
        try:
            print('Gige get instance')
            self.system = PySpin.System.GetInstance()
            print('Gige get camera')
            self.cam_list = self.system.GetCameras()
            print('Gige get cam size')
            num_cameras = self.cam_list.GetSize()

            if num_cameras == 0:
                self.cam_list.Clear()
                self.system.ReleaseInstance()
                print('Not enough cameras!')
                return False
            self.cam = self.cam_list[0]
            nodemap_tldevice = self.cam.GetTLDeviceNodeMap()
            self.cam.Init()
            nodemap = self.cam.GetNodeMap()
        except PySpin.SpinnakerException as ex:
            print('Error: %s' % ex)
            return False

        sNodemap = self.cam.GetTLStreamNodeMap()
        # Change bufferhandling mode to NewestOnly
        node_bufferhandling_mode = PySpin.CEnumerationPtr(sNodemap.GetNode('StreamBufferHandlingMode'))
        if not PySpin.IsReadable(node_bufferhandling_mode) or not PySpin.IsWritable(node_bufferhandling_mode):
            print('Unable to set stream buffer handling mode.. Aborting...')
            return False
        
        # Retrieve entry node from enumeration node
        node_newestonly = node_bufferhandling_mode.GetEntryByName('NewestOnly')
        if not PySpin.IsReadable(node_newestonly):
            print('Unable to set stream buffer handling mode.. Aborting...')
            return False

        # Retrieve integer value from entry node
        node_newestonly_mode = node_newestonly.GetValue()

        # Set integer value from entry node as new value of enumeration node
        node_bufferhandling_mode.SetIntValue(node_newestonly_mode)

        node_acquisition_mode = PySpin.CEnumerationPtr(nodemap.GetNode('AcquisitionMode'))
        if not PySpin.IsReadable(node_acquisition_mode) or not PySpin.IsWritable(node_acquisition_mode):
            print('Unable to set acquisition mode to continuous (enum retrieval). Aborting...')
            return False

        # Retrieve entry node from enumeration node
        node_acquisition_mode_continuous = node_acquisition_mode.GetEntryByName('Continuous')
        if not PySpin.IsReadable(node_acquisition_mode_continuous):
            print('Unable to set acquisition mode to continuous (entry retrieval). Aborting...')
            return False

        # Retrieve integer value from entry node
        acquisition_mode_continuous = node_acquisition_mode_continuous.GetValue()

        # Set integer value from entry node as new value of enumeration node
        node_acquisition_mode.SetIntValue(acquisition_mode_continuous)


        device_serial_number = ''
        node_device_serial_number = PySpin.CStringPtr(nodemap_tldevice.GetNode('DeviceSerialNumber'))
        if PySpin.IsReadable(node_device_serial_number):
            device_serial_number = node_device_serial_number.GetValue()
            print('Device serial number retrieved as %s...' % device_serial_number)

        return True

    def begin(self):
        #  Image acquisition must be ended when no more images are needed.
        self.cam.BeginAcquisition()

    def capture(self):
        
        try:
            image_result = self.cam.GetNextImage(1000)
            
            #  Ensure image completion
            if image_result.IsIncomplete():
                #print('Image incomplete with image status %d ...' % image_result.GetImageStatus())
                return True, self.img
            else:
                # Getting the image data as a numpy array
                image_data = image_result.GetNDArray()
                self.img = cv2.cvtColor(image_data, cv2.COLOR_GRAY2BGR)
            image_result.Release()
            return True, self.img
        except PySpin.SpinnakerException as ex:
            print("------------------------")
            print('Error: %s' % ex)
            return False, None

    def end(self):
        None
        
    def release(self):
        self.cam.EndAcquisition()
        self.cam.DeInit()
        del self.cam
        self.cam_list.Clear()
        self.system.ReleaseInstance()
