const getGame = async () => {
	return new Promise((resolve) => {
		const interval = setInterval(() => {
			const game = document.getElementsByClassName("board")[0].game;
			if (game) {
				clearInterval(interval);
				resolve(game);
			}
		}
		, 1000);
	});
}

console.log("Getting game");
const game = await getGame();
console.log("Got game");

const addMarking = (from, to) => {
	game.markings.addOne({
		"data": {
			"keyPressed": "none",
			"from": from,
			"opacity": 0.8,
			"to": to
		},
		"node": true,
		"persistent": false,
		"type": "arrow",
	});
};

game.on("Move", async (move) => {
	console.log(move.data.move);
	console.log(move.data.move.userGenerated);
	if (!move.data.move.userGenerated) {
		console.log("Getting move");
		const move = await getMove();
		addMarking(move.substr(0, 2), move.substr(2, 2));
	}
});

const getMove = async () => {
	await fetch("http://127.0.0.1:8000/new_game", { method: "POST" });
	let move;
	for (let i = 0; (move = game.getNodeByIds({move: i, line: 0})) !== null; i++) {
		await fetch(`http://127.0.0.1:8000/add_move/${move.from}${move.to}`, { method: "POST" });
	}
	return (await (await fetch("http://127.0.0.1:8000/get_move")).json())["move"];
}
