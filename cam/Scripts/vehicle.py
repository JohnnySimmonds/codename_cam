import camera
import physics
import vehicle
import types
import controller
import math
import runner
from start_game import *

v = None

def start_game(event):
    v.set_active(True)

def init(self):
    global v
    _controller = controller.Controller(0)
    c = vehicle.Configuration()

    dims = physics.PxVec3(3, 1, 5)

    c.position = physics.PxVec3(0, 2, 30)
    c.chassis_dimensions = dims
    c.steer_angle = math.pi * .05
    c.torque = 100000
    c.wheel_radius = 0.5
    c.wheel_width = 0.4
    c.wheel_mass = 10
    c.omega = 1000
    c.chassis_mass = 1000
    c.wheel_moi = 20
    c.chassis_moi = physics.PxVec3(c.chassis_mass, c.chassis_mass / 2, c.chassis_mass)
    c.chassis_offset = physics.PxVec3(0, -dims.y, 0)

    v = vehicle.Vehicle(self.physics(),_controller, c)
    v.set_active(False)
    cam = camera.Camera(v)
    r = runner.Runner()
    self.entity().add_component(v)
    self.entity().add_component(cam)
    self.entity().add_component(r)
    # self.entity().register_start_game_handler(start_game)
    self.entity().register_handler(StartGame, start_game)
