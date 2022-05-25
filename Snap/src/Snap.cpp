#include "Snap.h"

#define MAP_WIDTH 11.0f
#define MAP_HEIGHT 11.0f

#define TILE_SIZE 0.9f

using namespace Nebula;

static mat4 BackgroundMatrix = scale(vec3(32.0f, 18.0f, 1.0f)) * translate(vec3(0.0f, 0.0f, 0.0f));
static mat4 PressToPlayMatrix = scale(vec3(1.5f)) * translate(vec3(-5.0f, 0.0f, 0.01f));
static mat4 PressToSkipMatrix = translate(vec3(-4.5f, -7.0f, 0.01f));
static mat4 YouDiedMatrix = scale(vec3(2.0f)) * translate(vec3(-2.0f, 1.5f, 0.01f));

static uint32_t ConvertToIndex(vec2 position, uint32_t width) {
	return position.x + position.y * width;
}

static void SwapTiles(Tile& a, Tile& b) {
	vec2 posA = a.TilePosition;
	a.TilePosition = b.TilePosition;
	b.TilePosition = posA;

	a.Animation.TargetPosition = a.TilePosition;
	a.Animation.Speed = { 5.0f, 5.0f };
	a.Animation.hasPositionAnimation = true;

	b.Animation.TargetPosition = b.TilePosition;
	b.Animation.Speed = { 5.0f, 5.0f };
	b.Animation.hasPositionAnimation = true;
}

void Snap::Attach() {
	RenderCommand::SetClearColour({ 0.2f, 0.2f, 0.2f, 1.0f });
	
	Data.Textures.push_back({ 1.0f, 0.0f, 0.0f, 1.0f }); //===RED====
	Data.Textures.push_back({ 1.0f, 0.5f, 0.0f, 1.0f }); //==ORANGE==
	Data.Textures.push_back({ 1.0f, 1.0f, 0.0f, 1.0f }); //==YELLOW==
	Data.Textures.push_back({ 0.0f, 1.0f, 0.0f, 1.0f }); //==GREEN===
	Data.Textures.push_back({ 0.0f, 1.0f, 1.0f, 1.0f }); //LIGHT BLUE
	Data.Textures.push_back({ 0.0f, 0.0f, 1.0f, 1.0f }); //===BLUE===
	Data.Textures.push_back({ 0.5f, 0.0f, 1.0f, 1.0f }); //==PURPLE==
	Data.Textures.push_back({ 1.0f, 0.0f, 0.5f, 1.0f }); //===PINK===

	Data.RoundedCube = Texture2D::Create("Resources/textures/rounded.png");
	Data.Background = Texture2D::Create("Resources/textures/background.png");
	Data.Cloud = Texture2D::Create("Resources/textures/cloud.png");

	Data.HeartFull = Texture2D::Create("Resources/textures/heart_full.png");
	Data.HeartEmpty = Texture2D::Create("Resources/textures/heart_empty.png");
	
	Data.FoundNull = Texture2D::Create("Resources/textures/found_null.png");
	Data.FoundFalse = Texture2D::Create("Resources/textures/found_false.png");
	Data.FoundTrue = Texture2D::Create("Resources/textures/found_true.png");

	FontManager::Add(new Font("Roboto", "Resources/fonts/Roboto/Regular.ttf", 86.0f));

	Data.HeartFull->SetFilterNearest(true);
	Data.HeartEmpty->SetFilterNearest(true);
	Data.FoundNull->SetFilterNearest(true);
	Data.FoundFalse->SetFilterNearest(true);
	Data.FoundTrue->SetFilterNearest(true);
	
	OnWindowResize(WindowResizeEvent(1600, 900));
	Reset();

	for (uint32_t i = 0; i < 10; i++)
		Clouds.push_back({ i * 5.0f - camProjection.x, (float)Rand(0.0f, camProjection.y * 2.0f) - camProjection.y });

	GamePhase = Phase::Stopped;
}

void Snap::Detach() {
	FontManager::Clean();
}

void Snap::Reset() {
	Tiles.clear();
	for (float y = 1; y <= MAP_HEIGHT; y++) {
		for (float x = 1; x <= MAP_WIDTH; x++)
			Tiles.push_back(Tile({ x - (MAP_HEIGHT / 2.0f) - 0.5f, -y + (MAP_HEIGHT / 2.0f) + 0.5f }, vec2(TILE_SIZE), Rand(0, Data.Textures.size() - 1)));
	}

	ResetVariables();
	Lives.reset();
	shownCards = 0;
	Score = 0;
}

void Snap::ResetVariables() {
	GamePhase = Phase::Pair;
	rounds.clear();

	card_one_index = -1;
	tempShownCards = 0;

	chances.reset();
	turnsLeft.reset();
	
	isSelected = true;
}

void Snap::OnEvent(Event& e) {
	Dispatcher d(e);
	d.Dispatch<KeyPressedEvent>(BIND_EVENT(Snap::OnKeyPressed));
	d.Dispatch<MouseMovedEvent>(BIND_EVENT(Snap::OnMouseMoved));
	d.Dispatch<MouseButtonPressedEvent>(BIND_EVENT(Snap::OnMousePressed));
	d.Dispatch<WindowResizeEvent>(BIND_EVENT(Snap::OnWindowResize));
}

bool Snap::OnKeyPressed(KeyPressedEvent& e) {
	if (GamePhase == Phase::Stopped) {
		Reset();
		return true;
	}

	switch (e.GetKeyCode())
	{
	case NB_W:
		if (isSelected)
			selectedTile.y--;
		hoveredTile.y--;
		break;
	case NB_A:
		if (isSelected)
			selectedTile.x--;
		hoveredTile.x--;
		break;
	case NB_S:
		if (isSelected)
			selectedTile.y++;
		hoveredTile.y++;
		break;
	case NB_D:
		if (isSelected)
			selectedTile.x++;
		hoveredTile.x++;
		break;

	case NB_ENTER:
		if (!e.GetRepeatCount())
			UpdatePhase();

		break;
	case NB_SPACE:
		if (GamePhase == Phase::Move) {
			GamePhase = Phase::Match;
			Phase_Match();
		}
		break;
	}

	selectedTile.x = std::max(std::min(selectedTile.x, MAP_WIDTH - 1.0f), 0.0f);
	selectedTile.y = std::max(std::min(selectedTile.y, MAP_HEIGHT - 1.0f), 0.0f);
	hoveredTile.x = std::max(std::min(hoveredTile.x, MAP_WIDTH - 1.0f), 0.0f);
	hoveredTile.y = std::max(std::min(hoveredTile.y, MAP_HEIGHT - 1.0f), 0.0f);

	return true;
}

bool Snap::OnMouseMoved(MouseMovedEvent& e) {
	float mx = e.GetX(); float my = e.GetY();
	vec2 mousePos = { mx / windowToLocalScale.value.x - camProjection.x, -(my / windowToLocalScale.value.y - camProjection.y) };

	hoveredTile.x = round(mousePos.x + (MAP_WIDTH / 2.0f - 0.5f));
	hoveredTile.y = round(-mousePos.y + (MAP_HEIGHT / 2.0f - 0.5f));
	hoveredTile.x = std::max(std::min(hoveredTile.x, MAP_WIDTH - 1.0f), 0.0f);
	hoveredTile.y = std::max(std::min(hoveredTile.y, MAP_HEIGHT - 1.0f), 0.0f);
	
	if (isSelected)
		selectedTile = hoveredTile;
	
	return true;
}


bool Snap::OnMousePressed(MouseButtonPressedEvent& e) {
	if (GamePhase == Phase::Stopped) {
		Reset();
		return true;
	}

	if (e.GetMouseButton() == NB_MOUSE_0)
		UpdatePhase();


	return true;
}

void Snap::UpdatePhase() {
	switch (GamePhase)
	{
	case Snap::Pair:
		Phase_Pair();
		break;
	case Snap::Move:
		Phase_Move();
		break;
	case Snap::Match:
		break;
	case Snap::Stopped:
		break;
	}
}

bool Snap::OnWindowResize(WindowResizeEvent& e) {
	windowToLocalScale.reset();
	if (e.GetWidth() < MAP_WIDTH * windowToLocalScale.value.x)
		windowToLocalScale.value = vec2(e.GetWidth() / MAP_WIDTH);
	
	if (e.GetHeight() < MAP_HEIGHT * windowToLocalScale.value.y)
		windowToLocalScale.value = vec2(e.GetHeight() / MAP_HEIGHT);

	for (vec2& c : Clouds)
		c.x /= camProjection.x;

	float hWidth = e.GetWidth() / windowToLocalScale.value.x / 2.0f;
	float hHeight = e.GetHeight() / windowToLocalScale.value.y / 2.0f;
	SceneCam.SetProjection(-hWidth, hWidth, -hHeight, hHeight);
	camProjection = { hWidth, hHeight };
	BackgroundMatrix = scale(vec3(camProjection * 2.0f, 1.0f));
	
	vec2 position = { e.GetWidth() / windowToLocalScale.value.x - camProjection.x, camProjection.y };
	Data.HeartSize = { 1.5f, 1.5f };
	Data.HeartStartPos = position - vec2((Lives.original - 0.5f) * Data.HeartSize.x, Data.HeartSize.y / 2.0f);
	
	position.y = -(1.5f - camProjection.y);
	Data.FoundSize = { 1.5f, 1.5f };
	Data.FoundStartPos = position - vec2((chances.original - 0.5f) * Data.FoundSize.x, Data.FoundSize.y / 2.0f);

	for (vec2& c : Clouds)
		c.x *= camProjection.x;
	return true;
}

void Snap::Phase_Pair() {
	uint32_t i = ConvertToIndex(selectedTile, MAP_WIDTH);
	if (Tiles[i].isShown || tempShownCards || (card_one_index != -1 && Tiles[card_one_index].Animation.hasSizeAnimation))
		return;

	Tiles[i].Animation.hasSizeAnimation = true;
	Tiles[i].Animation.Speed = { 2.5f, 2.5f };
	Tiles[i].Animation.TargetSize = { 0.0f, TILE_SIZE };
	shownCards++;

	if (card_one_index == -1) {
		card_one_index = i;
		return;
	}

	chances.value--;

	if (Tiles[card_one_index].textureIndex == Tiles[i].textureIndex) {
		Score += 2;
		card_one_index = -1;
		rounds.push_back(true);
		NB_TRACE("Score: {0}", Score);
	} else {
		Tiles[card_one_index].Animation.hasSizeAnimation = true;
		Tiles[card_one_index].Animation.Speed = { 2.5f, 2.5f };
		Tiles[card_one_index].Animation.TargetSize = { 0.0f, TILE_SIZE };
		Tiles[i].isShown = true;

		shownCards -= 2;
		card_one_index = -1;
		rounds.push_back(false);
	}
	
	if (chances.value > 0.0f && chances.value < chances.original)
		return;
	
	bool found = false; bool madeError = false;
	for (bool b : rounds) {
		if (b)
			found = true;
		else
			madeError = true;
	}
	
	if (!madeError)
		Lives.value++;

	if (shownCards)
		GamePhase = Phase::Move;

	if (found)
		return;
	
	Lives.value--;
	if (!Lives)
		return;

	timer.reset();
	chances.reset();
	
	for (Tile& tile : Tiles) {
		if (tile.isShown || Rand(0, 5)) continue;

		tempShownCards++;
		tile.isTempShown = true;
		tile.Animation.hasSizeAnimation = true;
		tile.Animation.Speed = { 2.5f, 2.5f };
		tile.Animation.TargetSize = { 0.0f, TILE_SIZE };
	}

	rounds.clear();
}


void Snap::Phase_Move() {
	isSelected = !isSelected;
	if (!isSelected)
		return;

	if (hoveredTile == selectedTile) {
		isSelected = true;
		return;
	}

	uint32_t i = ConvertToIndex(selectedTile, MAP_WIDTH);
	
	Array<vec2> Positions = {
		{ Tiles[i].TilePosition.x, Tiles[i].TilePosition.y + 1 },
		{ Tiles[i].TilePosition.x, Tiles[i].TilePosition.y - 1 },
		{ Tiles[i].TilePosition.x + 1, Tiles[i].TilePosition.y },
		{ Tiles[i].TilePosition.x - 1, Tiles[i].TilePosition.y }
	};
	
	bool found = false;
	for (vec2& pos : Positions) {
		pos.x = round( pos.x + (MAP_WIDTH / 2.0f - 0.5f));
		pos.y = round(-pos.y + (MAP_HEIGHT / 2.0f - 0.5f));
		if (pos == hoveredTile) {
			found = true;
			break;
		}
	}

	if (!found) {
		isSelected = false;
		return;
	}

	uint32_t x = ConvertToIndex(hoveredTile, MAP_WIDTH);
	SwapTiles(Tiles[i], Tiles[x]);
	Tiles.swap(i, x);
	turnsLeft.value--;

	selectedTile = hoveredTile;

	if (!turnsLeft) {
		GamePhase = Phase::Match;
		timer.reset();
		Phase_Match();
	}
}

void Snap::Phase_Match() {
	for (Tile& tile : Tiles) {
		if (!tile.isShown) continue;

		Array<vec2> Positions = {
			{ tile.TilePosition.x, tile.TilePosition.y + 1 },
			{ tile.TilePosition.x, tile.TilePosition.y - 1 },
			{ tile.TilePosition.x + 1, tile.TilePosition.y },
			{ tile.TilePosition.x - 1, tile.TilePosition.y }
		};

		for (vec2& pos : Positions) {
			pos.x = round( pos.x + (MAP_WIDTH / 2.0f - 0.5f));
			pos.y = round(-pos.y + (MAP_HEIGHT / 2.0f - 0.5f));
			
			if (pos.x < 0.0f || pos.x > MAP_WIDTH - 1.0f) continue;
			if (pos.y < 0.0f || pos.y > MAP_HEIGHT - 1.0f) continue;

			uint32_t i = ConvertToIndex(pos, MAP_WIDTH);
			if (!Tiles[i].isShown) continue;

			if (Tiles[i].textureIndex == tile.textureIndex) {
				tile.isPopped = true;
				tile.isTempShown = true;
				tile.Animation.hasSizeAnimation = true;
				tile.Animation.Speed = vec2(4.0f);
				tile.Animation.TargetSize = vec2(0.0f);

				shownCards--;
				Score += 5; 
				NB_INFO("Score: {0}", Score);
				break;
			}
		}
	}
}

void Snap::Update(Timestep ts) {
	if (Lives <= 0)
		GamePhase = Phase::Stopped;

	if (ts < 2.0f) {
		for (vec2& c : Clouds) {
			c.x -= ts;
			if (c.x + 2.5f < -camProjection.x) {
				c.x = camProjection.x + 2.5f;
				c.y = Rand(0.0f, camProjection.y * 2.0f) - camProjection.y;
			}
		}
	}

	bool hasTilesMoved = false;
	for (Tile& tile : Tiles) {
		if (tile.Animation.hasPositionAnimation) {
			vec2 direction = tile.Animation.TargetPosition - tile.Position;
			vec2 d = direction;
			if (d.x < 0.0f)
				d.x = -d.x;
			if (d.y < 0.0f)
				d.y = -d.y;

			if (d.x + d.y < 0.05f) {
				tile.Position = tile.Animation.TargetPosition;
				tile.Animation.hasPositionAnimation = false;
				continue;
			}

			hasTilesMoved = true;
			tile.Position += tile.Animation.Speed * normalize(direction) * (float)ts;
		}

		if (tile.Animation.hasSizeAnimation) {
			vec2 direction = tile.Animation.TargetSize - tile.Size;
			vec2 d = direction;
			if (d.x < 0.0f)
				d.x = -d.x;
			if (d.y < 0.0f)
				d.y = -d.y;

			if (d.x + d.y < 0.05f) {
				tile.Size = tile.Animation.TargetSize;
				tile.Animation.hasSizeAnimation = false;
				continue;
			}

			hasTilesMoved = true;
			tile.Size += tile.Animation.Speed * normalize(direction) * (float)ts;
		}
	}

	for (Tile& tile : Tiles) {
		if (tile.isPopped && tile.Size == vec2(0.0f)) {
			tile.isShown = false;
			tile.isTempShown = false;
			tile.Animation.hasSizeAnimation = false;
			tile.Size = vec2(TILE_SIZE);
		} else if (tile.Size.x == 0.0f || tile.Size.y == 0.0f) {
			tile.isShown = !tile.isShown;
			tile.Animation.hasSizeAnimation = true;
			tile.Animation.Speed = vec2(2.5f);
			tile.Animation.TargetSize = vec2(TILE_SIZE);

			if (tile.isTempShown && !tile.isShown) {
				tempShownCards--;
				tile.isTempShown = false;
			}
		} else if (tile.Size.x == TILE_SIZE && tile.Size.y == TILE_SIZE && tile.isTempShown && timer.elapsed() >= 1.5f) {
			tile.Animation.hasSizeAnimation = true;
			tile.Animation.Speed = { 2.5f, 2.5f };
			tile.Animation.TargetSize = { 0.0f, TILE_SIZE };
		}

		if (tile.isPopped && tile.isTempShown && tile.Size == vec2(0.0f))
			tile.isTempShown = false;
	}

	if (GamePhase == Phase::Match) {
		if (!(timer.elapsed() >= 0.5f))
			return;

		for (Tile& tile : Tiles) {
			if (tile.isPopped) continue;

			vec2 belowPos = { tile.TilePosition.x, tile.TilePosition.y - 1 };
			belowPos.x = round( belowPos.x + (MAP_WIDTH / 2.0f - 0.5f));
			belowPos.y = round(-belowPos.y + (MAP_HEIGHT / 2.0f - 0.5f));
			
			if (belowPos.x < 0.0f || belowPos.x > MAP_WIDTH - 1.0f) continue;
			if (belowPos.y < 0.0f || belowPos.y > MAP_HEIGHT - 1.0f) continue;
			
			uint32_t belowIndex = ConvertToIndex(belowPos, MAP_WIDTH);

			if (!Tiles[belowIndex].isPopped) continue;
			
			vec2 tilePos = tile.TilePosition;
			tilePos.x = round( tilePos.x + (MAP_WIDTH / 2.0f - 0.5f));
			tilePos.y = round(-tilePos.y + (MAP_HEIGHT / 2.0f - 0.5f));
			uint32_t tileIndex = ConvertToIndex(tilePos, MAP_WIDTH);

			hasTilesMoved = true;
			
			SwapTiles(Tiles[belowIndex], Tiles[tileIndex]);
			Tiles.swap(belowIndex, tileIndex);
		}

		if (!hasTilesMoved) {
			for (uint32_t i = 0; i < Tiles.size(); i++) {
				Tile& tile = Tiles[i];
		
				for (float i = 0; i < MAP_WIDTH; i++) {
					if (Tiles[i].isPopped) {
						Tiles[i] = Tile(Tiles[i].TilePosition, Tiles[i].Size, Rand(0, Data.Textures.size() - 1));
						hasTilesMoved = true;
					}
				}
			}
		}

		if (!hasTilesMoved)
			ResetVariables();
	}
}

void Snap::Render() {
	RenderCommand::Clear();

	Renderer2D::BeginScene(SceneCam, SceneCam.GetViewMatrix());
	if (GamePhase != Phase::Stopped) {
		for (Tile& tile : Tiles) {
			if (tile.isPopped && !tile.isTempShown) continue;
	
			vec4 colour(0.4f, 0.4f, 0.4f, 1.0f);
			if (tile.isShown)
				colour = Data.Textures[tile.textureIndex];
	
			Renderer2D::Draw(NB_QUAD, tile, colour);
		}
	
		if (GamePhase != Phase::Match) {
			Renderer2D::Draw(NB_QUAD, translate(vec3(Tiles[ConvertToIndex(selectedTile, MAP_WIDTH)].Position)) * scale(vec3(1.1f, 1.1f, 1.0f)),
				vec4(0.83f, 0.68f, 0.21f, 1.0f));
	
			Renderer2D::Draw(NB_QUAD, translate(vec3(Tiles[ConvertToIndex(hoveredTile, MAP_WIDTH)].Position)) * scale(vec3(1.1f, 1.1f, 1.0f)),
				vec4(0.83f, 0.68f, 0.21f, 0.5f));
		}
	}
	
	if (GamePhase == Phase::Move && !isSelected) {
		Array<vec2> Positions = {
			{ selectedTile.x, selectedTile.y + 1 },
			{ selectedTile.x, selectedTile.y - 1 },
			{ selectedTile.x + 1, selectedTile.y },
			{ selectedTile.x - 1, selectedTile.y }
		};
	
		for (vec2& pos : Positions) {
			if (pos.x < 0.0f || pos.x > MAP_WIDTH - 1.0f) continue;
			if (pos.y < 0.0f || pos.y > MAP_HEIGHT - 1.0f) continue;
	
			Renderer2D::Draw(NB_QUAD, translate(vec3(Tiles[ConvertToIndex(pos, MAP_WIDTH)].TilePosition)) * scale(vec3(1.1f, 1.1f, 1.0f)),
				vec4(1.0f, 1.0f, 1.0f, 0.75f));
		}
	}
	
	Renderer2D::Draw(NB_QUAD, BackgroundMatrix, vec4(1.0f), Data.Background);

	for (vec2& c : Clouds)
		Renderer2D::Draw(NB_QUAD, translate(vec3(c, 0.001f)) * scale(vec3(2.5f, 1.0f, 1.0f)), vec4(1.0f), Data.Cloud);

	Renderer2D::EndScene();

	Renderer2D::BeginScene(SceneCam, SceneCam.GetViewMatrix());

	for (uint32_t i = 0; i < Lives.original; i++) {
		Ref<Texture2D> texture = Data.HeartFull;
		if (i + 1 > Lives.value)
			texture = Data.HeartEmpty;
	
		Renderer2D::Draw(NB_QUAD, translate(vec3(Data.HeartStartPos.x + i * Data.HeartSize.x, Data.HeartStartPos.y, 1.0f)) * scale(vec3(Data.HeartSize, 1.0f)), vec4(1.0f), texture);
	}
	
	for (uint32_t i = 0; i < chances.original; i++) {
		Ref<Texture2D> texture = Data.FoundNull;
		if (i < rounds.size()) {
			if (rounds[i])
				texture = Data.FoundTrue;
			else
				texture = Data.FoundFalse;
		}
	
		Renderer2D::Draw(NB_QUAD, translate(vec3(Data.FoundStartPos.x + i * Data.FoundSize.x, Data.FoundStartPos.y, 1.0f)) * scale(vec3(Data.FoundSize, 1.0f)), vec4(1.0f), texture);
	}
	
	if (GamePhase == Phase::Stopped) {
		Renderer2D::DrawString("Press Any Button to Play", FontManager::Get("Roboto"), PressToPlayMatrix, { 0.0f, 0.0f, 0.0f, 1.0f });
		if (Lives.value <= 0.0f)
			Renderer2D::DrawString("You Died!", FontManager::Get("Roboto"), YouDiedMatrix, { 0.0f, 0.0f, 0.0f, 1.0f });
	} else {
		if (GamePhase == Phase::Move)
			Renderer2D::DrawString("Press Space to Skip", FontManager::Get("Roboto"), PressToSkipMatrix, { 0.0f, 0.0f, 0.0f, 1.0f });
	}

	Renderer2D::EndScene();
}