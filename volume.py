#!/usr/bin/python
# -*- coding: utf-8 -*-

#Bluetooth volume control GUI
#Copyright (C) 2013 Andrzej Surowiec <emeryth@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import sys
import serial
import time
from PyQt4 import QtGui, QtCore

class VolumeControl(QtGui.QWidget):
    
    def __init__(self):
        super(VolumeControl, self).__init__()   
        self.volume=0
        self.ser=serial.Serial("/dev/rfcomm0",9600,timeout=0.1)
        self.ser.flushInput()
        self.initUI()
        self.getVolume()
    
    def getVolume(self):
        #print "getting volume"
        self.ser.write("\n")
        time.sleep(0.1)
        self.ser.flushInput()
        self.ser.write("get\n")
        time.sleep(0.1)
        self.ser.readline()
        line= self.ser.readline()
        self.volume=int(line[2:])
        self.label.setText("Volume: %d"%self.volume)
        self.volumeSlider.setValue(self.volume)

        
    def setVolume(self,volume):
        #print "setting volume to %d" %volume
        self.volume=volume
        self.label.setText("Volume: %d"%self.volume)
        self.ser.write("v%03d\n"%self.volume)
        time.sleep(0.1)
        self.ser.flushInput()
  
    def turnOn(self):
        self.ser.write("on\n")
        time.sleep(0.1)
        self.ser.flushInput()

    def turnOff(self):
        self.ser.write("off\n")
        time.sleep(0.1)
        self.ser.flushInput()

    def initUI(self):      
        self.volumeSlider = QtGui.QSlider(QtCore.Qt.Horizontal, self)
        self.volumeSlider.setFocusPolicy(QtCore.Qt.NoFocus)
        self.volumeSlider.setMaximum(255)
        self.volumeSlider.setGeometry(10, 80, 700, 60)
        self.volumeSlider.valueChanged[int].connect(self.changeValue)
        
        onButton = QtGui.QPushButton('ON', self)
        onButton.move(10,150)
        onButton.clicked.connect(self.turnOn)
        
        offButton = QtGui.QPushButton('OFF', self)
        offButton.move(160,150)
        offButton.clicked.connect(self.turnOff)

        self.label = QtGui.QLabel(self)
        self.label.setText("Volume: %d"%self.volume)
        self.label.setGeometry(10, 50, 300, 30)

        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.getVolume)
        self.timer.start(1000)
        
        self.setGeometry(0, 0, 800, 400)
        self.setWindowTitle('Volume Control')
        self.show()
        
    def changeValue(self, value):
        self.setVolume(value)
        self.getVolume()
        
        
def main():  
    app = QtGui.QApplication(sys.argv)
    ex = VolumeControl()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main() 
