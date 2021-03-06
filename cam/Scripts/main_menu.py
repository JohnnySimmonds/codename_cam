from component import *
from controller import *
from physics import *
from ui import *
from util import *
from entity import *
import sys
import os

sys.path.insert(0, os.getcwd() + "\Scripts")

from chaser import *
from runner import *
import player
from runner_ai import *
from camera_control import *
from chaser_ai import *
from vehicle_script import *
from ChaserManager import *
# from end_screen import *
import end_screen

def set_position(marker, item):
    marker.position = item.position + Vec2(-0.2, 0.0)


class MainMenu:
    def __init__(self, renderer):
        self.renderer = renderer
        self.first_press = True

    def create_powerups(self):
        e = self.entity
        while e.get_parent():
            e = e.get_parent()

        oilpositions = [
            Vec3(0.0, 0.1, 60),
            Vec3(40.0, 0.1, -60),
            Vec3(100.0, 0.1, 90),
            Vec3(-40.0, 0.1, -120),
            Vec3(0.0, 0.1, -200),
            Vec3(-40.0, 0.1, 200),
            Vec3(-100.0, 0.1, -90),
            Vec3(-150.0, 0.1, -90),
            Vec3(40.0, 0.1, 30)
        ]

        spikepositions = [
            Vec3(-20.0, 0.1, 100),
            Vec3(120.0, 0.1, -60),
            Vec3(80.0, 0.1, 70),
            Vec3(-100, 0.1, -40),
            Vec3(-150, 0.1, 60),
            Vec3(20.0, 0.1, -100),
            Vec3(-120.0, 0.1, 120),
            Vec3(-80.0, 0.1, -70),
            Vec3(100, 0.1, 40),
            Vec3(150, 0.1, -60)
        ]

        for p in oilpositions:
            o = Entity.create(e).lock()
            o.transform().position = p
            oilslick = Powerup(ChangeSurface(0.0), ChangeSurface(1.0), 3.0, "oil_slick_texture.jpg", "oil_slick_mesh.fbx")
            o.add_component(PowerupPickup("oil_pickup_texture.jpg", "pickup_mesh.fbx", "oil_slick.png", oilslick), self.physics)

        for s in spikepositions:
            spikes = Entity.create(e).lock()
            spikes.global_position = s
            powerup = Powerup(ChangeSpeed(3.0 / 4.0), ChangeSpeed(4.0 / 3.0), 3.0, "oil_slick_texture.jpg", "spike_mesh.fbx")
            spikes.add_component(PowerupPickup("spike_pickup_texture.png", "pickup_mesh.fbx", "spike-trap.png", powerup), self.physics)

    def start_game(self):
        e = self.entity

        while e.get_parent():
            e = e.get_parent()

        level = Entity.create(e).lock()
        mesh = Mesh(ModelShader("map_texture.jpg"), "map_mesh.fbx", Vec3(0.2, 0.4, 0.2), Vec3(3.0, 3.0, 3.0),
                    4)  # TODO hacky hacky hardcoded opengl constant, 4 is GL_TRIANGLES
        body = RigidBody(self.physics, "map_mesh.fbx", 3.0, False)
        level.add_component(mesh)
        level.add_component(body)

        ## uncomment this block to see the verts of the nav mesh
        # navMesh = Entity.create(e).lock()
        # aMesh = Mesh(ModelShader("nav_mesh.jpg"), "nav_mesh.fbx", Vec3(0.2, 0.4, 0.2), Vec3(3.0, 3.0, 3.0),
        #             0)
        # navMesh.add_component(aMesh)

        manager_entity = Entity.create(e).lock()
        manager = ChaserManager()
        manager_entity.add_component(manager, self.physics)

        end_entity = Entity.create(e).lock()
        endScreen = end_screen.EndScreen(manager, self.renderer)
        end_entity.add_component(endScreen, self.physics)

        chaser1 = Entity.create(e).lock()
        chaser1.global_position += Vec3(30.0, 2.0, 0.0)
        chaser1.add_component(ChaserAi(manager), self.physics)

        chaser2 = Entity.create(e).lock()
        chaser2.global_position += Vec3(0.0, 2.0, -30.0)
        chaser2.add_component(ChaserAi(manager), self.physics)

        player1 = Entity.create(e).lock()
        player1.global_position += Vec3(318.0, 2.0, 6.0)
        player1.add_component(player.Player(manager), self.physics)

        runner1 = Entity.create(e).lock()
        runner1.global_position = Vec3(342.0, 2.0, 309.0)
        runner1.add_component(RunnerAi(manager), self.physics)

        runner2 = Entity.create(e).lock()
        runner2.global_position = Vec3(336.0, 2.0, -337.0)
        runner2.add_component(RunnerAi(manager), self.physics)

        runner3 = Entity.create(e).lock()
        runner3.global_position = Vec3(-127.0, 2.0, -300.0)
        runner3.add_component(RunnerAi(manager), self.physics)

        runner4 = Entity.create(e).lock()
        runner4.global_position = Vec3(-306.0, 2.0, -333.0)
        runner4.add_component(RunnerAi(manager), self.physics)

        runner5 = Entity.create(e).lock()
        runner5.global_position = Vec3(-342.0, 2.0, 45.0)
        runner5.add_component(RunnerAi(manager), self.physics)

        runner6 = Entity.create(e).lock()
        runner6.global_position = Vec3(-345.0, 2.0, 316.0)
        runner6.add_component(RunnerAi(manager), self.physics)

        runner7 = Entity.create(e).lock()
        runner7.global_position = Vec3(-1.0, 2.0, 327.0)
        runner7.add_component(RunnerAi(manager), self.physics)


        # o = Entity.create(e).lock()
        # o.transform().position = Vec3(0.0, 0.0, 60)
        # oilslick = Powerup(ChangeSurface(0.0), ChangeSurface(1.0), 3.0, "oil_slick_texture.jpg", "oil_slick_mesh.fbx")
        # o.add_component(PowerupPickup("oil_pickup_texture.jpg", "pickup_mesh.fbx", "oil_slick.png", oilslick), self.physics)
        # spikes = Entity.create(e).lock()
        # spikes.global_position = Vec3(0.0, 0.0, 60)
        # powerup = Powerup(ChangeSpeed(3.0 / 4.0), ChangeSpeed(4.0 / 3.0), 3.0, "oil_slick_texture.jpg", "spike_mesh.fbx")
        # spikes.add_component(PowerupPickup("spike_pickup_texture.png", "pickup_mesh.fbx", "spike-trap.png", powerup), self.physics)

        e.add_component(ScriptComponent("start_game", self.physics))

        self.create_powerups()
        self.entity.destroy()

    def start(self):
        self.control = Controller(0)
        self.background = Image("background.png", Vec2(-1.0, -1.0), Vec2(2.0, 2.0), 3)
        self.startgame = Text("Start Game", "GROBOLD", Vec2(-0.3, 0.0), 12)
        self.quitgame = Text("Quit Game", "GROBOLD", Vec2(-0.3, -0.2), 12)
        self.marker = Image("menu-marker.png", Vec2(-1.0, -1.0), Vec2(0.2, 0.2), 1)
        self.selected = self.startgame

        set_position(self.marker, self.startgame)

        self.entity.add_component(self.background)
        self.entity.add_component(self.startgame)
        self.entity.add_component(self.quitgame)
        self.entity.add_component(self.marker)

    def update(self, dt):
        self.control.update()
        if self.control.updown > 0:  # down - probably shouldn't hardcode these...
            if self.selected != self.quitgame:
                self.selected = self.quitgame
                set_position(self.marker, self.quitgame)
        elif self.control.updown < 0:
            if self.selected != self.startgame:
                self.selected = self.startgame
                set_position(self.marker, self.startgame)

        if not self.control.select:
            self.first_press = False

        if self.control.select and not self.first_press:
            if self.selected == self.startgame:
                self.start_game()
            else:
                self.renderer.close_window()
