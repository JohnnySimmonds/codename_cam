from component import *
from controller import *
from physics import *
from ui import *
from entity import *

import sys
import os
sys.path.insert(0, os.getcwd() + "\..\..\cam\Scripts")
from runner_ai import *

def set_position(marker, item):
    marker.position = item.position + Vec2(-0.1, 0.05)

class MainMenu:
    def start_game(self):
        e = self.entity

        while e.get_parent():
            e = e.get_parent()

        level = Entity.create(e).lock()
        mesh = Mesh(ModelShader("map_texture.jpg"), "map_mesh.fbx", Vec3(0.2, 0.4, 0.2), Vec3(2.0, 2.0, 2.0), 4) # TODO hacky hacky hardcoded opengl constant, 4 is GL_TRIANGLES
        body = RigidBody(self.physics, "map_mesh.fbx", 2.0, False)
        level.add_component(mesh)
        level.add_component(body)

        ai = Entity.create(e).lock()
        mesh = Mesh(ModelShader("chaser_texture.jpg"), "chaser_mesh.fbx", Vec3(1.0, 0.84, 0.0), Vec3(1, 1, 1), 4)
        vehicle = ScriptComponent("chaser_ai", self.physics)
        chaser = ScriptComponent("chaser", self.physics)
        ai.add_component(mesh)
        ai.add_component(vehicle)
        ai.add_component(chaser)

        player = Entity.create(e).lock()
        vehicle = ScriptComponent("vehicle_script", self.physics)

        player.add_component(vehicle)
        player.add_component(ScriptComponent("powerup_manager", self.physics))

        runner = Entity.create(e).lock()
        mesh = Mesh(ModelShader("runner_texture.jpg"), "runner_mesh.fbx", Vec3(1.0, 0.84, 0.0), Vec3(1, 1, 1), 4)
        #ai = ScriptComponent("runner_ai", self.physics)
        ai = RunnerAi(Vec3(0.0, 0.0, -30.0))
        runner.add_component(mesh)
        runner.add_component(ai, self.physics)

        o = Entity.create(e).lock()
        o.transform().position = Vec3(0.0, 0.0, 60)
        o.add_component(ScriptComponent("oil_slick_powerup", self.physics))
        
        runner = Entity.create(e).lock()
        runner.add_component(Mesh(ModelShader("runner_texture.jpg"), "runner_mesh.fbx", Vec3(1.0, 0.84, 0.0), Vec3(1, 1, 1), 4))
        # runner.add_component(ScriptComponent("runner_ai", self.physics))
        runner.add_component(RunnerAi(Vec3(0.0, 0.0, 60.0)), self.physics)

        e.add_component(ScriptComponent("start_game", self.physics))

        self.entity.destroy()

    def start(self):
        self.control = Controller(0)
        self.background = Image("background.png", Vec2(-1.0, -1.0), Vec2(2.0, 2.0), 3)
        self.startgame = Image("start-game.png", Vec2(-0.3, 0.0), Vec2(0.6, 0.2), 2)
        self.quitgame = Image("quit-game.png", Vec2(-0.3, -0.2), Vec2(0.6, 0.2), 2)
        self.marker = Image("menu-marker.png", Vec2(-1.0, -1.0), Vec2(0.2, 0.2), 1)
        self.selected = self.startgame

        set_position(self.marker, self.startgame)

        self.entity.add_component(self.background)
        self.entity.add_component(self.startgame)
        self.entity.add_component(self.quitgame)
        self.entity.add_component(self.marker)

    def update(self, dt):
        self.control.update()
        if self.control.direction == 7: # down - probably shouldn't hardcode these...
            if self.selected != self.quitgame:
                self.selected = self.quitgame
                set_position(self.marker, self.quitgame)
        elif self.control.direction == 6:
            if self.selected != self.startgame:
                self.selected = self.startgame
                set_position(self.marker, self.startgame)

        if self.control.select:
            if self.selected == self.startgame:
                self.start_game()
