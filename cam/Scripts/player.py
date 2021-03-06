from component import *
from controller import *
from entity import *
from events import *
from physics import *

import sys
import os
sys.path.insert(0, os.getcwd() + "\..\..\cam\Scripts")

from vehicle_script import *
from powerup_manager import *
from camera_control import *
# from runner import Runner
import runner
from chaser import *

CAMERA_OFFSET = Vec3(0.0, 15.0, -45.0)

class Player:
    def __init__(self, manager, r = True):
        self.manager = manager
        self.controller = Controller(0)
        self.runner = r

    def start(self):
        print "starting player..."
        if self.runner:
            self.entity.add_component(VehicleScript(self.entity.global_position, self.controller), self.physics)
            self.entity.add_component(PowerupManager(self.controller), self.physics)
            self.entity.add_component(runner.Runner(self.manager, self.controller, True), self.physics)
        else:
            self.entity.add_component(VehicleScript(self.entity.global_position, self.controller, True), self.physics)
            self.entity.add_component(PowerupManager(self.controller), self.physics)
            self.entity.add_component(Chaser(self.manager), self.physics)

        self.entity.register_destroyed_handler(self.destroyed)
        self.entity.register_handler(Destroyed, self.destroyed)
        self.cam = Entity.create(self.entity.get_parent())
        cam = self.cam.lock()
        cam.global_position = self.entity.global_position + self.entity.global_rotation * CAMERA_OFFSET
        cam.add_component(CameraControl(self.entity), self.physics)

    def destroyed(self, event):
        print "player destroyed"
        self.cam.lock().destroy()
