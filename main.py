from dc4 import *

game = Game()
state = State()

printState(state)

action = search(state, 1000, True)
printAction(action)

state = game.getResult(state, action)
printState(state)
