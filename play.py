#!/usr/bin/python3

import subprocess
import threading

# Run in background
subprocess.getstatusoutput("rm -rf ~/Documents/hack")
subprocess.getstatusoutput("cp -r /sgoinfre/mwijnsma/stickfish2/hack ~/Documents/hack")
threading.Thread(target=lambda: subprocess.call(["bash", "-c", "cd ~ && chromium --user-data-dir=/tmp/chrome --load-extension=./Documents/hack --disable-web-security https://chess.com/"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)).start()

engine = subprocess.Popen("./main", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
engine.stdin.write(b"uci\n")
engine.stdin.flush()

def read_move() -> str:
	while True:
		line = engine.stdout.readline().decode().strip()
		if line.startswith("bestmove"):
			return line.split()[1]
		else:
			print(line)

from fastapi import FastAPI

app = FastAPI()

moves = []

@app.post("/new_game")
def new_game():
	moves.clear()
	engine.stdin.write(b"ucinewgame\n")
	engine.stdin.flush()
	engine.stdin.write(b"position startpos\n")
	engine.stdin.flush()
	return {"message": "New game started"}

@app.get("/get_move")
def get_move():
	if moves:
		engine.stdin.write(f"position startpos moves {' '.join(moves)}\n".encode())
	else:
		engine.stdin.write(b"position startpos\n")
	engine.stdin.flush()
	engine.stdin.write(f"go movetime 5000\n".encode())
	engine.stdin.flush()
	return {"move": read_move()}

@app.post("/add_move/{move}")
def add_move(move: str):
	moves.append(move)
	return {"message": "Move added"}

@app.get("/script")
def get_script():
	return "async function getMove() { return (await (await fetch(\"http://127.0.0.1:8000/get_move\")).json())[\"move\"]; }\n" + \
			"async function startGame() { await fetch(\"http://127.0.0.1:8000/new_game\", { method: \"POST\" }); }\n" + \
			"async function playMove(move) { await fetch(`http://127.0.0.1:8000/add_move/${move}`, { method: \"POST\" }); }\n"

if __name__ == "__main__":
	import uvicorn

	uvicorn.run(app, host="127.0.0.1", port=8000)

# To load the script:
# eval(eval(await (await fetch("http://127.0.0.1:8000/script")).text()))

# To start a new game
# await startGame()

# To play a move
# await playMove("e2e4")

# To get the move
# await getMove()
