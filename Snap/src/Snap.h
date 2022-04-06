#pragma once

#include <Nebula.h>
#include "Tiles.h"

class Snap : public Nebula::Layer {
public:
	Snap() : Nebula::Layer("Snap"), SceneCam(-16.0f, 16.0f, -9.0f, 9.0f) { }
	~Snap() { }

	void Attach() override;
	void Detach() override { }

	void OnEvent(Nebula::Event& e) override;

	void Update(Nebula::Timestep ts) override;
	void Render() override;
private:
	bool OnKeyPressed(Nebula::KeyPressedEvent& e);
	bool OnMouseMoved(Nebula::MouseMovedEvent& e);
	bool OnMousePressed(Nebula::MouseButtonPressedEvent& e);
	bool OnWindowResize(Nebula::WindowResizeEvent& e);

	void UpdatePhase();

	void Phase_Pair();
	void Phase_Move();
	void Phase_Match();

	void Reset();
	void ResetVariables();
private:
	Nebula::OrthographicCamera SceneCam;

	TileData Data;
	Nebula::Array<Tile> Tiles;
	Nebula::vec2 selectedTile;
	Nebula::vec2 hoveredTile;

	float Score = 0.0f;
	Resetable<uint32_t> Lives = 3.0f;

	Nebula::vec2 camProjection = { 16.0f, 9.0f };
	Resetable<Nebula::vec2> windowToLocalScale = Nebula::vec2(60.0f);
	Nebula::Timer timer;

	enum Phase {
		Pair, Move, Match, Stopped
	};

	Phase GamePhase;
	
	//Pair Phase
	uint32_t card_one_index = -1;
	float shownCards = 0;
	float tempShownCards = 0;
	Resetable<uint32_t> chances = 3;
	Nebula::Array<bool> rounds;

	//Move Phase
	Resetable<uint32_t> turnsLeft = 3;
	bool isSelected;
};