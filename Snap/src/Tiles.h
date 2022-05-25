#pragma once

template <typename T>
struct Resetable {
	T value;
	T original;

	Resetable() = default;
	Resetable(T v): value(v), original(v) { }

	operator bool() const { return value; }

	void reset() {
		value = original;
	}

	void set(T v)  {
		value = v;
		original = v
	}
};

struct AnimationData {
	bool hasPositionAnimation = false;
	bool hasSizeAnimation = false;
	Nebula::vec2 TargetSize = { 0.0f, 0.0f };
	Nebula::vec2 TargetPosition = { 0.0f, 0.0f };
	Nebula::vec2 Speed = { 0.0f, 0.0f };

	AnimationData() = default;
	AnimationData(const AnimationData&) = default;
};

struct Tile {
	uint32_t textureIndex = 0;
	bool isShown = false;
	bool isPopped = false;
	bool isTempShown = false;
	AnimationData Animation;

	Nebula::vec2 Position{ 0.0f, 0.0f };
	Nebula::vec2 TilePosition{ 0.0f, 0.0f };
	Nebula::vec2 Size{ 1.0f, 1.0f };
	float Rotation = 0.0f;

	Tile() = default;
	Tile(const Tile&) = default;
	Tile(Nebula::vec2 pos, Nebula::vec2 size = { 1.0f, 1.0f }, uint32_t index = 0, bool shown = false) :
		Position(pos), Size(size), textureIndex(index), isShown(shown), TilePosition(pos) { }

	Nebula::mat4 GetTransform() const {
		return Nebula::translate(Nebula::vec3(Position, 0.1f)) * Nebula::scale(Nebula::vec3(Size, 1.0f)) * Nebula::rotate(Rotation, { 0.0f, 0.0f, 1.0f });
	}

	operator Nebula::mat4() const { return GetTransform(); }
};

struct TileData {
	Nebula::Array<Nebula::vec4> Textures; 
	
	Nebula::Ref<Nebula::Texture2D> RoundedCube;
	Nebula::Ref<Nebula::Texture2D> Background;

	Nebula::Ref<Nebula::Texture2D> HeartFull;
	Nebula::Ref<Nebula::Texture2D> HeartEmpty;

	Nebula::vec2 HeartSize;
	Nebula::vec2 HeartStartPos;
	
	Nebula::Ref<Nebula::Texture2D> FoundNull;
	Nebula::Ref<Nebula::Texture2D> FoundFalse;
	Nebula::Ref<Nebula::Texture2D> FoundTrue;
	
	Nebula::vec2 FoundSize;
	Nebula::vec2 FoundStartPos;

	Nebula::Ref<Nebula::Texture2D> Cloud;
};
